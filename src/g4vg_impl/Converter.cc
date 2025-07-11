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
#include <G4ReflectionFactory.hh>
#include <G4ReplicaNavigation.hh>
#include <G4VPVParameterisation.hh>
#include <G4VPhysicalVolume.hh>
#include <VecGeom/management/ReflFactory.h>
#include <VecGeom/volumes/LogicalVolume.h>
#include <VecGeom/volumes/PlacedVolume.h>

#include "Logger.hh"
#include "LogicalVolumeConverter.hh"
#include "PrintableLV.hh"
#include "Scaler.hh"
#include "SolidConverter.hh"
#include "Transformer.hh"
#include "TypeDemangler.hh"

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
    bool reflection_factory{true};
    std::unordered_set<G4LogicalVolume const*>* all_lv;

    void operator()(G4LogicalVolume const* lv)
    {
        G4VG_EXPECT(lv);
        if (reflection_factory)
        {
            if (auto const* unrefl_lv = get_constituent_lv(*lv))
            {
                // Visit underlying instead of reflected
                return (*this)(unrefl_lv);
            }
        }

        // Add this LV
        all_lv->insert(lv);

        // Visit daughters
        using size_type = decltype(lv->GetNoDaughters());
        for (size_type i = 0, imax = lv->GetNoDaughters(); i != imax; ++i)
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
                   bool reflection_factory,
                   Transformer const& trans,
                   VecPv* placed_volumes,
                   G4LogicalVolume const* daughter_g4lv,
                   VGLogicalVolume* mother_lv)
        : reflection_factory_{reflection_factory}
        , convert_transform_{trans}
        , placed_pv_{placed_volumes}
        , mother_lv_{mother_lv}
    {
        G4VG_EXPECT(placed_pv_);
        G4VG_EXPECT(daughter_g4lv);
        G4VG_EXPECT(mother_lv_);

        // Test for reflection
        if (reflection_factory_)
        {
            if (G4LogicalVolume const* unrefl_g4lv
                = get_constituent_lv(*daughter_g4lv))
            {
                // Replace with constituent volume, and flip the Z scale
                // See G4ReflectionFactory::CheckScale: the reflection value is
                // hard coded to {1, 1, -1}
                daughter_g4lv = unrefl_g4lv;
                flip_z_ = true;
            }
        }

        daughter_lv_ = build_vgdaughter(daughter_g4lv);
        G4VG_ENSURE(daughter_lv_);
    }

    //! Using Geant4 daughter physical volume, place the VecGeom daughter
    void operator()(G4VPhysicalVolume const* g4pv) const
    {
        G4VG_EXPECT(g4pv);

        VGPlacedVolume const* vgpv = nullptr;
        if (reflection_factory_)
        {
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

            auto const& daughters = mother_lv_->GetDaughters();
            G4VG_ASSERT(daughters.size() > 0);
            vgpv = daughters[daughters.size() - 1];
        }
        else
        {
            auto transform = build_transform(convert_transform_, *g4pv);
            auto* placed
                = daughter_lv_->Place(g4pv->GetName().c_str(), &transform);
            G4VG_ASSERT(placed);
            placed->SetCopyNo(g4pv->GetCopyNo());
            mother_lv_->PlaceDaughter(placed);
            vgpv = placed;
        }

        // Add the newly placed daughter to the map
        G4VG_ASSERT(vgpv);
        auto id = vgpv->id();
        placed_pv_->resize(std::max<std::size_t>(placed_pv_->size(), id + 1),
                           nullptr);
        (*placed_pv_)[id] = g4pv;
    }

    //! Place replica daughters: see ReplicaUpdater, ParamUpdater
    template<class F>
    void operator()(G4VPhysicalVolume* g4pv, F&& update_pv) const
    {
        for (int j = 0, jmax = g4pv->GetMultiplicity(); j < jmax; ++j)
        {
            // Modify the volume's position and place the copy
            update_pv(j, g4pv);
            g4pv->SetCopyNo(j);
            (*this)(g4pv);
        }
    }

  private:
    bool reflection_factory_;
    Transformer const& convert_transform_;
    VecPv* placed_pv_{nullptr};
    VGLogicalVolume* mother_lv_{nullptr};
    VGLogicalVolume* daughter_lv_{nullptr};
    bool flip_z_{false};
};

struct ReplicaUpdater
{
    void operator()(int copy_no, G4VPhysicalVolume* g4pv)
    {
        // TODO: check and error if the replica uses the kRaxis replication
        nav_.ComputeTransformation(copy_no, g4pv);
    }

    G4ReplicaNavigation nav_;
};

struct ParamUpdater
{
    void operator()(int copy_no, G4VPhysicalVolume* g4pv)
    {
        param_->ComputeTransformation(copy_no, g4pv);
    }

    G4VPVParameterisation* param_;
};
}  // namespace

//---------------------------------------------------------------------------//
//! Construct with scale
Converter::Converter(Options const& options)
    : options_{options}
    , convert_scale_{std::make_unique<Scaler>(options.scale)}
    , convert_transform_{std::make_unique<Transformer>(*convert_scale_)}
    , convert_solid_{std::make_unique<SolidConverter>(
          *convert_scale_, *convert_transform_, options.compare_volumes)}
    , convert_lv_{std::make_unique<LogicalVolumeConverter>(
          *convert_solid_, options_.append_pointers)}
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

    G4VG_LOG(status) << "Converting Geant4 geometry";

    // Recurse through physical volumes once to build underlying LV
    std::unordered_set<G4LogicalVolume const*> all_g4lv;
    all_g4lv.reserve(G4LogicalVolumeStore::GetInstance()->size());
    LVMapVisitor{options_.reflection_factory,
                 &all_g4lv}(g4world->GetLogicalVolume());

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
    auto* world_pv = world_lv->Place(g4world->GetName().c_str(), &trans);
    G4VG_ASSERT(world_pv);
    G4VG_ASSERT(world_pv->id() == placed_volumes_.size());
    placed_volumes_.push_back(g4world);

    result_type result;
    result.world = world_pv;
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
    using size_type = decltype(mother_g4lv->GetNoDaughters());
    for (size_type i = 0, imax = mother_g4lv->GetNoDaughters(); i != imax; ++i)
    {
        // Get daughter volume
        G4VPhysicalVolume* g4pv = mother_g4lv->GetDaughter(i);
        G4VG_ASSERT(g4pv);

        DaughterPlacer place_daughter(convert_daughter,
                                      options_.reflection_factory,
                                      *convert_transform_,
                                      &placed_volumes_,
                                      g4pv->GetLogicalVolume(),
                                      mother_lv);

        switch (g4pv->VolumeType())
        {
            case EVolume::kNormal:
                // Place daughter, accounting for reflection
                place_daughter(g4pv);
                break;
            case EVolume::kReplica:
                // Place daughter in each replicated location
                place_daughter(g4pv, ReplicaUpdater{});
                break;
            case EVolume::kParameterised:
                // Place each paramterized instance of the daughter
                G4VG_ASSERT(g4pv->GetParameterisation());
                place_daughter(g4pv, ParamUpdater{g4pv->GetParameterisation()});
                break;
            default:
                G4VG_LOG(error)
                    << "Unsupported type '"
                    << TypeDemangler<G4VPhysicalVolume>{}(*g4pv)
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
