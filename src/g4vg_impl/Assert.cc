//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/Assert.cc
//---------------------------------------------------------------------------//
#include "Assert.hh"

#include <sstream>

namespace g4vg
{
namespace
{
//---------------------------------------------------------------------------//
/*!
 * Construct a debug assertion message for printing.
 */
std::string build_debug_error_msg(char const* cond, char const* file, int line)
{
    std::ostringstream msg;
    msg << file << ':' << line << ':' << "\ng4vg: assertion failure: " << cond;
    return msg.str();
}

//---------------------------------------------------------------------------//
/*!
 * Construct a debug assertion message for printing.
 */
std::string build_runtime_error_msg(char const* which,
                                    std::string what,
                                    std::string cond,
                                    std::string file,
                                    int line)
{
    std::ostringstream msg;

    msg << "g4vg: " << (which ? which : "unknown") << " error: " << what
        << '\n'
        << (file.empty() ? "unknown source" : file);
    if (line && !file.empty())
    {
        msg << ':' << line;
    }

    msg << ':';
    if (!cond.empty())
    {
        msg << " '" << cond << "' failed";
    }
    else
    {
        msg << " failure";
    }

    return msg.str();
}

//---------------------------------------------------------------------------//
}  // namespace

//---------------------------------------------------------------------------//
/*!
 * Construct a debug exception from detailed attributes.
 */
DebugError::DebugError(char const* cond, char const* file, int line)
    : std::logic_error(build_debug_error_msg(cond, file, line))
{
}

//---------------------------------------------------------------------------//
// Default destructor to anchor vtable
DebugError::~DebugError() = default;

//---------------------------------------------------------------------------//
/*!
 * Construct a runtime error from detailed descriptions.
 */
RuntimeError::RuntimeError(char const* which,
                           std::string what,
                           std::string cond,
                           std::string file,
                           int line)
    : std::runtime_error(build_runtime_error_msg(which, what, cond, file, line))
    , what_(std::move(what))

{
}

//---------------------------------------------------------------------------//
// Default destructor to anchor vtable
RuntimeError::~RuntimeError() = default;

//---------------------------------------------------------------------------//
// String constants for throwing
char const RuntimeError::validate_err_str[] = "runtime";

//---------------------------------------------------------------------------//
}  // namespace g4vg
