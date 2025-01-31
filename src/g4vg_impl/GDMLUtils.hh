//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/GDMLUtils.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4GDMLWriteStructure.hh>
#include <G4LogicalVolume.hh>
#include <G4ReflectionFactory.hh>

namespace g4vg
{
//---------------------------------------------------------------------------//
// FREE FUNCTIONS
//---------------------------------------------------------------------------//
// Generate the GDML name for a Geant4 logical volume
std::string make_gdml_name(G4LogicalVolume const&);

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Generate the GDML name for a Geant4 logical volume.
 */
inline std::string make_gdml_name(G4LogicalVolume const& lv)
{
    // Run the LV through the GDML export name generator so that the volume is
    // uniquely identifiable in VecGeom. Reuse the same instance to reduce
    // overhead: note that the method isn't const correct.
    static G4GDMLWriteStructure temp_writer;

    auto const* refl_factory = G4ReflectionFactory::Instance();
    if (auto const* unrefl_lv
        = refl_factory->GetConstituentLV(const_cast<G4LogicalVolume*>(&lv)))
    {
        // If this is a reflected volume, add the reflection extension after
        // the final pointer to match the converted VecGeom name
        std::string name
            = temp_writer.GenerateName(unrefl_lv->GetName(), unrefl_lv);
        name += refl_factory->GetVolumesNameExtension();
        return name;
    }

    return temp_writer.GenerateName(lv.GetName(), &lv);
}

//---------------------------------------------------------------------------//
}  // namespace g4vg
