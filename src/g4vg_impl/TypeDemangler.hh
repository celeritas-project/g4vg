//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/TypeDemangler.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4Version.hh>
#if G4VERSION_NUMBER >= 1070
#    include <G4Backtrace.hh>
#endif

namespace g4vg
{
//---------------------------------------------------------------------------//
/*!
 * Utility function for demangling C++ types (specifically with GCC).
 *
 * See:
 * http://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
 * Example:
 * \code
    std::string int_type = demangled_typeid_name(typeid(int).name());
    TypeDemangler<Base> demangle;
    std::string static_type = demangle();
    std::string dynamic_type = demangle(Derived());
   \endcode
 */
template<class T>
class TypeDemangler
{
  public:
    // Get the pretty typename of the instantiated type (static)
    inline std::string operator()() const;
    // Get the *dynamic* pretty typename of a variable (dynamic)
    inline std::string operator()(T const&) const;
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Get the pretty typename of the instantiated type (static).
 */
template<class T>
inline std::string TypeDemangler<T>::operator()() const
{
#if G4VERSION_NUMBER >= 1070
    return G4Demangle<T>();
#else
    return std::string(typeid(T).name());
#endif
}

//---------------------------------------------------------------------------//
/*!
 * Get the *dynamic* pretty typename of a variable (dynamic).
 */
template<class T>
inline std::string TypeDemangler<T>::operator()(T const& t) const
{
#if G4VERSION_NUMBER >= 1070
    return G4Demangle(typeid(t).name());
#else
    return std::string(typeid(t).name());
#endif
}

}  // namespace g4vg
