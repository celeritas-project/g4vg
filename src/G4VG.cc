//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
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
Converted convert(G4VPhysicalVolume const* world, Options options)
{
    using Converter = g4vg::impl::Converter;

    // Construct converter
    Converter convert{[&options] {
        Converter::Options geocel_opts;
        geocel_opts.verbose = options.verbose;
        geocel_opts.compare_volumes = options.compare_volumes;
        return geocel_opts;
    }()};

    // Convert
    auto geocel_result = convert(world);

    // Remap output to remove volume IDs
    Converted converted;
    converted.world = geocel_result.world;
    converted.volumes.reserve(geocel_result.volumes.size());
    for (auto&& [lv, vid] : geocel_result.volumes)
    {
        converted.volumes.insert({lv, vid.unchecked_get()});
    }

    return converted;
}

//---------------------------------------------------------------------------//
}  // namespace g4vg
