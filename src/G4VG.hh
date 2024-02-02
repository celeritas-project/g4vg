//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file G4VG.hh
//---------------------------------------------------------------------------//
#pragma once

#include <unordered_map>

//---------------------------------------------------------------------------//
// FORWARD DECLARATIONS
//---------------------------------------------------------------------------//
class G4LogicalVolume;
class G4VPhysicalVolume;

namespace vecgeom
{
inline namespace cxx
{
class VPlacedVolume;
}  // namespace cxx
}  // namespace vecgeom
//---------------------------------------------------------------------------//

namespace g4vg
{
//---------------------------------------------------------------------------//
/*!
 * Construction options to pass to the converter.
 */
struct Options
{
    //! Print extra messages for debugging
    bool verbose{false};

    //! Perform conversion checks
    bool compare_volumes{false};

    //! TODO: allow client to use a different unit system (default: mm = 1)
    static constexpr double scale = 1;
};

//---------------------------------------------------------------------------//
/*!
 * Result from converting from Geant4 to VecGeom.
 */
struct Converted
{
    using VGPlacedVolume = vecgeom::VPlacedVolume;
    using MapLvVolId = std::unordered_map<G4LogicalVolume const*, unsigned int>;

    //! World pointer (host) corresponding to input Geant4 world
    VGPlacedVolume* world{nullptr};

    //! Map of Geant4 logical volumes to VecGeom LV IDs
    MapLvVolId volumes;
};

//---------------------------------------------------------------------------//
// Convert a Geant4 geometry to a VecGeom geometry.
Converted convert(G4VPhysicalVolume const* world);

// Convert with custom options
Converted convert(G4VPhysicalVolume const* world, Options options);

//---------------------------------------------------------------------------//
}  // namespace g4vg
