//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/LogicalVolumeConverter.hh
//---------------------------------------------------------------------------//
#pragma once

#include <unordered_map>
#include <vector>

//---------------------------------------------------------------------------//
// Forward declarations
//---------------------------------------------------------------------------//

class G4LogicalVolume;

namespace vecgeom
{
inline namespace cxx
{
class LogicalVolume;
}
}  // namespace vecgeom
//---------------------------------------------------------------------------//

namespace g4vg
{
//---------------------------------------------------------------------------//
class SolidConverter;

//---------------------------------------------------------------------------//
/*!
 * Convert a Geant4 base LV to a VecGeom LV.
 *
 * This does not convert or add any of the daughters, which must be placed as
 * physical volumes.
 */
class LogicalVolumeConverter
{
  public:
    //!@{
    //! \name Type aliases
    using arg_type = G4LogicalVolume const&;
    using result_type = vecgeom::LogicalVolume*;
    using VecLv = std::vector<G4LogicalVolume const*>;
    //!@}

  public:
    LogicalVolumeConverter(SolidConverter& convert_solid, bool append_pointers);

    // Convert a volume
    result_type operator()(arg_type);

    // Construct a mapping from G4 logical volume to logical volume ID
    VecLv make_volume_map() const;

  private:
    //// DATA ////

    SolidConverter& convert_solid_;
    bool append_pointers_{false};
    std::unordered_map<G4LogicalVolume const*, result_type> cache_;

    //// HELPER FUNCTIONS ////

    // Convert an LV that's not in the cache
    result_type construct_base(arg_type);
};

//---------------------------------------------------------------------------//
}  // namespace g4vg
