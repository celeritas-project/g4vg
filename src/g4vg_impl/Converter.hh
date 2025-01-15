//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/Converter.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "G4VG.hh"

namespace g4vg
{
//---------------------------------------------------------------------------//
class Scaler;
class Transformer;
class SolidConverter;
class LogicalVolumeConverter;

//---------------------------------------------------------------------------//
/*!
 * Create an in-memory VecGeom model from an in-memory Geant4 model.
 *
 * Return the new world volume and a mapping of Geant4 logical volumes to
 * VecGeom-based volume IDs.
 */
class Converter
{
  public:
    //!@{
    //! \name Type aliases
    using arg_type = G4VPhysicalVolume const*;
    using VecPv = std::vector<G4VPhysicalVolume const*>;
    using result_type = g4vg::Converted;
    //!@}

  public:
    // Construct with options
    explicit Converter(Options const&);

    // Default destructor
    ~Converter();

    // Convert the world
    result_type operator()(arg_type);

  private:
    using VGLogicalVolume = vecgeom::LogicalVolume;

    Options options_;
    int depth_{0};

    std::unique_ptr<Scaler> convert_scale_;
    std::unique_ptr<Transformer> convert_transform_;
    std::unique_ptr<SolidConverter> convert_solid_;
    std::unique_ptr<LogicalVolumeConverter> convert_lv_;
    std::unordered_set<VGLogicalVolume const*> built_daughters_;
    VecPv placed_volumes_;

    VGLogicalVolume* build_with_daughters(G4LogicalVolume const* mother_g4lv);
};

//---------------------------------------------------------------------------//
}  // namespace g4vg
