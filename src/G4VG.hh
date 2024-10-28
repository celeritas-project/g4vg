//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file G4VG.hh
//---------------------------------------------------------------------------//
#pragma once

#include <vector>

//---------------------------------------------------------------------------//
// FORWARD DECLARATIONS
//---------------------------------------------------------------------------//

class G4LogicalVolume;
class G4VPhysicalVolume;

namespace vecgeom
{
inline namespace cxx
{
class LogicalVolume;
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

    //! Value of 1mm in native unit system (0.1 for cm)
    double scale = 1;
};

//---------------------------------------------------------------------------//
/*!
 * Result from converting from Geant4 to VecGeom.
 */
struct Converted
{
    using VGPlacedVolume = vecgeom::VPlacedVolume;
    using VecLv = std::vector<G4LogicalVolume const*>;
    using VecPv = std::vector<G4VPhysicalVolume const*>;

    //! World pointer (host) corresponding to input Geant4 world
    VGPlacedVolume* world{nullptr};

    //! Geant4 LVs indexed by VecGeom LogicalVolume ID
    VecLv logical_volumes;
    //! Geant4 PVs indexed by VecGeom PlacedVolume ID
    VecPv physical_volumes;
};

//---------------------------------------------------------------------------//
// Convert a Geant4 geometry to a VecGeom geometry.
Converted convert(G4VPhysicalVolume const* world);

// Convert with custom options
Converted convert(G4VPhysicalVolume const* world, Options const& options);

//---------------------------------------------------------------------------//
}  // namespace g4vg
