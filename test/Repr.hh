//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Repr.hh
//---------------------------------------------------------------------------//
#pragma once

#include <iterator>
#include <ostream>
#include <type_traits>

namespace g4vg
{
namespace test
{
//---------------------------------------------------------------------------//

template<typename T, typename = void>
struct is_iterable : std::false_type
{
};

template<typename T>
struct is_iterable<T,
                   std::void_t<decltype(std::begin(std::declval<T&>())),
                               decltype(std::end(std::declval<T&>()))>>
    : std::true_type
{
};

template<typename T>
constexpr bool is_iterable_v = is_iterable<T>::value;

//---------------------------------------------------------------------------//
template<class T>
struct Repr
{
    T value;
};

// Deduction guide for perfect forwarding (preserves reference qualifiers)
template<class T>
Repr(T&&) -> Repr<std::decay_t<T>>;

// Deduction guide for `const&` preservation (prevents decay)
template<class T>
Repr(T const&) -> Repr<T const&>;

//---------------------------------------------------------------------------//
template<class T>
std::ostream& operator<<(std::ostream& os, Repr<T> const& r)
{
    using DecayedT = std::decay_t<T>;
    if constexpr (std::is_same_v<DecayedT, std::string>)
    {
        os << '"' << r.value << '"';
    }
    else if constexpr (is_iterable_v<DecayedT>)
    {
        os << '{';
        for (auto const& v : r.value)
        {
            os << Repr{v} << ", ";
        }
        os << '}';
    }
    else if constexpr (std::is_floating_point_v<DecayedT>)
    {
        os << std::setprecision(15) << r.value;
    }
    else
    {
        os << r.value;
    }

    return os;
}

//---------------------------------------------------------------------------//
template<class T>
Repr<T> repr(T const& v)
{
    return Repr<T>{v};
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
