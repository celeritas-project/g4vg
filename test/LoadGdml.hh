//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file LoadGdml.hh
//---------------------------------------------------------------------------//
#pragma once

#include <regex>
#include <string>
#include <G4GDMLParser.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4SolidStore.hh>
#include <G4Version.hh>

namespace g4vg
{
namespace test
{
//---------------------------------------------------------------------------//
static bool search_pointer(std::string const& s, std::smatch& ptr_match)
{
    // Search either for th end of the expression, or an underscore that likely
    // indicates a _refl or _PV suffix
    static std::regex const ptr_regex{"0x[0-9a-f]{4,16}(?=_|$)"};
    return std::regex_search(s, ptr_match, ptr_regex);
}

//---------------------------------------------------------------------------//
//! Remove pointer address from inside geometry names
template<class StoreT>
inline void excise_pointers(StoreT& obj_store)
{
    std::smatch sm;
    for (auto* obj : obj_store)
    {
        if (!obj)
            continue;

        std::string const& name = obj->GetName();
        if (search_pointer(name, sm))
        {
            std::string new_name = name.substr(0, sm.position());
            new_name += name.substr(sm.position() + sm.length());
            obj->SetName(new_name);
        }
    }
#if G4VERSION_NUMBER >= 1100
    obj_store.UpdateMap();  // Added by geommng-V10-07-00
#endif
}

//---------------------------------------------------------------------------//
/*!
 * Load a GDML file, removing pointers without stripping 'refl' suffixes.
 */
inline G4VPhysicalVolume* load_gdml(std::string const& filename)
{
    G4GDMLParser gdml_parser;
    gdml_parser.SetStripFlag(false);
    gdml_parser.Read(filename, /* validate_gdml_schema = */ false);

    excise_pointers(*G4SolidStore::GetInstance());
    excise_pointers(*G4PhysicalVolumeStore::GetInstance());
    excise_pointers(*G4LogicalVolumeStore::GetInstance());

    return gdml_parser.GetWorldVolume();
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
