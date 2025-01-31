//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/PrintableLV.hh
//---------------------------------------------------------------------------//
#pragma once

#include <ostream>
#include <G4LogicalVolume.hh>

namespace g4vg
{
//---------------------------------------------------------------------------//
/*!
 * Wrapper around a G4LogicalVolume to get a descriptive output.
 */
struct PrintableLV
{
    G4LogicalVolume const* lv{nullptr};
};

//---------------------------------------------------------------------------//
/*!
 * Print the logical volume name, ID, and address.
 */
inline std::ostream& operator<<(std::ostream& os, PrintableLV const& plv)
{
    if (plv.lv)
    {
        os << '"' << plv.lv->GetName() << "\"@"
           << static_cast<void const*>(plv.lv)
           << " (ID=" << plv.lv->GetInstanceID() << ')';
    }
    else
    {
        os << "{null G4LogicalVolume}";
    }
    return os;
}

//---------------------------------------------------------------------------//
}  // namespace g4vg
