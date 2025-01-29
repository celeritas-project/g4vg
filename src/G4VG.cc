//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file G4VG.cc
//---------------------------------------------------------------------------//
#include "G4VG.hh"

#include "g4vg_impl/Converter.hh"

namespace g4vg
{
//---------------------------------------------------------------------------//
/*!
 * Convert a Geant4 geometry to a VecGeom geometry.
 *
 * Return the new world volume and a mapping of Geant4 logical volumes to
 * VecGeom-based volume IDs.
 */
Converted convert(G4VPhysicalVolume const* world)
{
    return convert(world, {});
}

//---------------------------------------------------------------------------//
/*!
 * Convert with custom options.
 */
Converted convert(G4VPhysicalVolume const* world, Options const& options)
{
    using Converter = g4vg::Converter;

    // Construct converter
    Converter convert{options};

    // Convert
    return convert(world);
}

//---------------------------------------------------------------------------//
}  // namespace g4vg
