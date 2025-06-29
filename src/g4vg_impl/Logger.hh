//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/Logger.hh
//---------------------------------------------------------------------------//
#pragma once

#include <VecGeom/base/Version.h>

#if VECGEOM_VERSION >= 0x010208

// Use VecGeom logger directly
#    include <VecGeom/management/Logger.h>
#    define G4VG_LOG(LEVEL)                          \
        ::vecgeom::logger()(VECGEOM_CODE_PROVENANCE, \
                            ::vecgeom::LogLevel::LEVEL)

#else

// Hack for vecgeom before logger implementation: note that newlines will be
// missing :) but it's better than not compiling right?
#    include <iostream>
#    define G4VG_LOG(LEVEL) std::cout << "G4VG " << #LEVEL << ": "

#endif
