//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/LogicalVolumeConverter.cc
//---------------------------------------------------------------------------//
#include "LogicalVolumeConverter.hh"

#include <G4LogicalVolume.hh>
#include <G4VSolid.hh>
#include <VecGeom/management/GeoManager.h>
#include <VecGeom/volumes/LogicalVolume.h>
#include <VecGeom/volumes/UnplacedVolume.h>

#include "Assert.hh"
#include "GDMLUtils.hh"
#include "PrintableLV.hh"
#include "SolidConverter.hh"

namespace g4vg
{
//---------------------------------------------------------------------------//
/*!
 * Construct with solid conversion helper.
 */
LogicalVolumeConverter::LogicalVolumeConverter(SolidConverter& convert_solid)
    : convert_solid_(convert_solid)
{
    G4VG_EXPECT(!vecgeom::GeoManager::Instance().IsClosed());
}

//---------------------------------------------------------------------------//
/*!
 * Convert a Geant4 logical volume to a VecGeom LogicalVolume.
 *
 * This uses a cache to look up any previously converted volume.
 */
auto LogicalVolumeConverter::operator()(arg_type lv) -> result_type
{
    auto [cache_iter, inserted] = cache_.insert({&lv, nullptr});
    if (inserted)
    {
        // First time converting the volume
        cache_iter->second = this->construct_base(lv);
    }

    G4VG_ENSURE(cache_iter->second);
    return cache_iter->second;
}

//---------------------------------------------------------------------------//
/*!
 * Construct a mapping from G4 logical volume to logical volume ID.
 */
auto LogicalVolumeConverter::make_volume_map() const -> VecLv
{
    VecLv result;
    result.reserve(cache_.size());

    for (auto&& [g4lv, lv] : cache_)
    {
        G4VG_ASSERT(lv);
        auto id = lv->id();
        result.resize(std::max<std::size_t>(result.size(), id + 1), nullptr);
        result[id] = g4lv;
    }
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Convert the raw logical volume from geant4 to vecgeom.
 */
auto LogicalVolumeConverter::construct_base(arg_type g4lv) -> result_type
{
    vecgeom::VUnplacedVolume const* shape = nullptr;
    try
    {
        shape = convert_solid_(*g4lv.GetSolid());
    }
    catch (g4vg::RuntimeError const& e)
    {
        VECGEOM_LOG(error) << "Failed to convert solid type '"
                           << g4lv.GetSolid()->GetEntityType() << "' named '"
                           << g4lv.GetSolid()->GetName()
                           << "': " << e.what_minimal();
        shape = this->convert_solid_.to_sphere(*g4lv.GetSolid());
        VECGEOM_LOG(warning)
            << "Replaced unknown solid with sphere with capacity "
            << shape->Capacity() << " [len^3]";
        VECGEOM_LOG(info) << "Unsupported solid belongs to logical volume "
                          << PrintableLV{&g4lv};
    }

    std::string name = g4lv.GetName();
    if (name.find("0x") == std::string::npos)
    {
        // No pointer address: add one
        name = make_gdml_name(g4lv);
    }

    return new vecgeom::LogicalVolume(name.c_str(), shape);
}

//---------------------------------------------------------------------------//
}  // namespace g4vg
