//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/Converter.cc
//---------------------------------------------------------------------------//
#include "Converter.hh"

#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <G4LogicalVolumeStore.hh>
#include <G4PVDivision.hh>
#include <G4PVPlacement.hh>
#include <G4ReflectionFactory.hh>
#include <G4VPhysicalVolume.hh>
#include <VecGeom/management/ReflFactory.h>
#include <VecGeom/volumes/LogicalVolume.h>
#include <VecGeom/volumes/PlacedVolume.h>

#include "LogicalVolumeConverter.hh"
#include "Scaler.hh"
#include "SolidConverter.hh"
#include "Transformer.hh"

namespace g4vg
{
namespace
{
//---------------------------------------------------------------------------//
//! Get the underlying volume if one exists (const-correct)
G4LogicalVolume const* get_constituent_lv(G4LogicalVolume const& lv)
{
    return G4ReflectionFactory::Instance()->GetConstituentLV(
        const_cast<G4LogicalVolume*>(&lv));
}

//---------------------------------------------------------------------------//
/*!
 * Build a VecGeom transform from a Geant4 physical volume.
 */
vecgeom::Transformation3D
build_transform(Transformer const& convert, G4VPhysicalVolume const& g4pv)
{
    return convert(g4pv.GetTranslation(), g4pv.GetRotation());
}

//---------------------------------------------------------------------------//
//! Add all visited logical volumes to a set.
struct LVMapVisitor
{
    std::unordered_set<G4LogicalVolume const*>* all_lv;

    void operator()(G4LogicalVolume const* lv)
    {
        G4VG_EXPECT(lv);
        if (auto const* unrefl_lv = get_constituent_lv(*lv))
        {
            // Visit underlying instead of reflected
            return (*this)(unrefl_lv);
        }

        // Add this LV
        all_lv->insert(lv);

        // Visit daughters
        for (auto const i : range(lv->GetNoDaughters()))
        {
            G4VPhysicalVolume const* daughter{lv->GetDaughter(i)};
            G4VG_ASSERT(daughter);
            (*this)(daughter->GetLogicalVolume());
        }
    }
};

//---------------------------------------------------------------------------//
/*!
 * Place a daughter in a mother, accounting for reflection.
 */
class DaughterPlacer
{
  public:
    using VGLogicalVolume = vecgeom::LogicalVolume;
    using VGPlacedVolume = vecgeom::VPlacedVolume;
    using VecPv = std::vector<G4VPhysicalVolume const*>;

    template<class F>
    DaughterPlacer(F&& build_vgdaughter,
                   Transformer const& trans,
                   VecPv* placed_volumes,
                   G4LogicalVolume const* daughter_g4lv,
                   VGLogicalVolume* mother_lv)
        : convert_transform_{trans}
        , placed_pv_{placed_volumes}
        , mother_lv_{mother_lv}
    {
        G4VG_EXPECT(placed_pv_);
        G4VG_EXPECT(daughter_g4lv);
        G4VG_EXPECT(mother_lv_);

        // Test for reflection
        if (G4LogicalVolume const* unrefl_g4lv
            = get_constituent_lv(*daughter_g4lv))
        {
            // Replace with constituent volume, and flip the Z scale
            // See G4ReflectionFactory::CheckScale: the reflection value is
            // hard coded to {1, 1, -1}
            daughter_g4lv = unrefl_g4lv;
            flip_z_ = true;
        }

        daughter_lv_ = build_vgdaughter(daughter_g4lv);
        G4VG_ENSURE(daughter_lv_);
    }

    //! Using Geant4 daughter physical volume, place the VecGeom daughter
    void operator()(G4VPhysicalVolume const* g4pv) const
    {
        G4VG_EXPECT(g4pv);

        vecgeom::Vector3D<double> const reflvec{
            1, 1, static_cast<double>(flip_z_ ? -1 : 1)};

        // Use the VGDML reflection factory to place the daughter in the
        // mother (it must *always* be used, in case parent is reflected)
        vecgeom::ReflFactory::Instance().Place(
            build_transform(convert_transform_, *g4pv),
            reflvec,
            g4pv->GetName(),
            daughter_lv_,
            mother_lv_,
            g4pv->GetCopyNo());

        // Add the newly placed daughter to the map
        auto const& daughters = mother_lv_->GetDaughters();
        G4VG_ASSERT(daughters.size() > 0);
        VGPlacedVolume const* vgpv = daughters[daughters.size() - 1];
        G4VG_ASSERT(vgpv);
        auto id = vgpv->id();
        placed_pv_->resize(std::max<std::size_t>(placed_pv_->size(), id + 1),
                           nullptr);
        (*placed_pv_)[id] = g4pv;
    }

  private:
    Transformer const& convert_transform_;
    Converter::VecPv* placed_pv_{nullptr};
    VGLogicalVolume* mother_lv_{nullptr};
    VGLogicalVolume* daughter_lv_{nullptr};
    bool flip_z_{false};
};

}  // namespace

//---------------------------------------------------------------------------//
//! Construct with scale
Converter::Converter(Options options)
    : options_{options}
    , convert_scale_{std::make_unique<Scaler>(options.scale)}
    , convert_transform_{std::make_unique<Transformer>(*convert_scale_)}
    , convert_solid_{std::make_unique<SolidConverter>(
          *convert_scale_, *convert_transform_, options.compare_volumes)}
    , convert_lv_{std::make_unique<LogicalVolumeConverter>(*convert_solid_)}
{
}

//---------------------------------------------------------------------------//
//! Default destructor
Converter::~Converter() = default;

//---------------------------------------------------------------------------//
auto Converter::operator()(arg_type g4world) -> result_type
{
    G4VG_EXPECT(g4world);
    G4VG_EXPECT(!g4world->GetRotation());
    G4VG_EXPECT(g4world->GetTranslation() == G4ThreeVector(0, 0, 0));

    VECGEOM_LOG(status) << "Converting Geant4 geometry";

    // Recurse through physical volumes once to build underlying LV
    std::unordered_set<G4LogicalVolume const*> all_g4lv;
    all_g4lv.reserve(G4LogicalVolumeStore::GetInstance()->size());
    LVMapVisitor{&all_g4lv}(g4world->GetLogicalVolume());

    // Convert visited volumes in instance order to try to approximate layout
    // of Geant4
    for (auto* lv : *G4LogicalVolumeStore::GetInstance())
    {
        if (all_g4lv.count(lv))
        {
            (*convert_lv_)(*lv);
        }
    }

    // Place world volume
    VGLogicalVolume* world_lv
        = this->build_with_daughters(g4world->GetLogicalVolume());
    auto trans = build_transform(*convert_transform_, *g4world);

    result_type result;
    result.world = world_lv->Place(g4world->GetName().c_str(), &trans);
    result.logical_volumes = convert_lv_->make_volume_map();
    result.physical_volumes = std::move(placed_volumes_);

    G4VG_ENSURE(result.world);
    G4VG_ENSURE(!result.logical_volumes.empty());
    G4VG_ENSURE(!result.physical_volumes.empty());
    return result;
}

//---------------------------------------------------------------------------//
//! \cond
/*!
 * Convert a volume and its daughter volumes.
 */
auto Converter::build_with_daughters(G4LogicalVolume const* mother_g4lv)
    -> VGLogicalVolume*
{
    G4VG_EXPECT(mother_g4lv);

    if (G4VG_UNLIKELY(options_.verbose))
    {
        std::clog << std::string(depth_, ' ') << "Converting "
                  << mother_g4lv->GetName() << std::endl;
    }

    // Convert or get corresponding VecGeom volume
    VGLogicalVolume* mother_lv = (*convert_lv_)(*mother_g4lv);

    if (auto [iter, inserted] = built_daughters_.insert(mother_lv); !inserted)
    {
        // Daughters have already been built
        return mother_lv;
    }

    ++depth_;

    auto convert_daughter = [this](G4LogicalVolume const* g4lv) {
        return this->build_with_daughters(g4lv);
    };

    // Place daughter logical volumes in this mother
    for (auto i : range(mother_g4lv->GetNoDaughters()))
    {
        // Get daughter volume
        G4VPhysicalVolume* g4pv = mother_g4lv->GetDaughter(i);

        DaughterPlacer place_daughter(convert_daughter,
                                      *convert_transform_,
                                      &placed_volumes_,
                                      g4pv->GetLogicalVolume(),
                                      mother_lv);

        if (auto* placed = dynamic_cast<G4PVPlacement const*>(g4pv))
        {
            // Place daughter, accounting for reflection
            place_daughter(placed);
        }
        else if (G4VPVParameterisation* param = g4pv->GetParameterisation())
        {
            // Loop over number of replicas
            for (auto j : range(g4pv->GetMultiplicity()))
            {
                // Use the paramterization to *change* the physical volume's
                // position (yes, this is how Geant4 does it too)
                param->ComputeTransformation(j, g4pv);

                // Add a copy
                place_daughter(g4pv);
            }
        }
        else
        {
            TypeDemangler<G4VPhysicalVolume> demangle_pv_type;
            VECGEOM_LOG(error)
                << "Unsupported type '" << demangle_pv_type(*g4pv)
                << "' for physical volume '" << g4pv->GetName()
                << "' (corresponding LV: "
                << PrintableLV{g4pv->GetLogicalVolume()} << ")";
        }
    }

    --depth_;

    return mother_lv;
}
//! \endcond

//---------------------------------------------------------------------------//
}  // namespace g4vg
