//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/CeleritasUtils.hh
//---------------------------------------------------------------------------//
#pragma once

#include <cstddef>

namespace g4vg
{
//---------------------------------------------------------------------------//
// TYPES
//---------------------------------------------------------------------------//

using real_type = double;
using size_type = std::size_t;
using VolumeId = size_type;

template<class T, size_type N>
using Array = std::array<T, N>;
//---------------------------------------------------------------------------//
}  // namespace g4vg
