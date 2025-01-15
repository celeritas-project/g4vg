//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/Assert.hh
//---------------------------------------------------------------------------//
#pragma once

#include <stdexcept>

#if !defined(G4VG_DEBUG)
#    warning "G4VG_DEBUG should be defined as a private target definition"
#endif

//---------------------------------------------------------------------------//
/*!
 * \def G4VG_UNLIKELY(condition)
 *
 * Mark the result of this condition to be "unlikely".
 *
 * This asks the compiler to move the section of code to a "cold" part of the
 * instructions, improving instruction locality. It should be used primarily
 * for error checking conditions.
 */
#if defined(__clang__) || defined(__GNUC__)
// GCC and Clang support the same builtin
#    define G4VG_UNLIKELY(COND) __builtin_expect(!!(COND), 0)
#else
// No other compilers seem to have a similar builtin
#    define G4VG_UNLIKELY(COND) (COND)
#endif

/*!
 * \def G4VG_UNREACHABLE
 *
 * Mark a point in code as being impossible to reach in normal execution.
 *
 * See https://clang.llvm.org/docs/LanguageExtensions.html#builtin-unreachable
 * or https://msdn.microsoft.com/en-us/library/1b3fsfxw.aspx
 * or
 * https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#__builtin_unreachable
 *
 * (The "unreachable" and "assume" compiler optimizations for CUDA are only
 * available in API version 11.3 or higher, which is encoded as
 * \code major*1000 + minor*10 \endcode).
 *
 * \note This macro should not generally be used; instead, the macro \c
 * G4VG_ASSERT_UNREACHABLE() defined in base/Assert.hh should be used instead
 * (to provide a more detailed error message in case the point *is* reached).
 */
#if (!defined(__CUDA_ARCH__) && (defined(__clang__) || defined(__GNUC__))) \
    || defined(__NVCOMPILER)                                               \
    || (defined(__CUDA_ARCH__) && CUDART_VERSION >= 11030)                 \
    || defined(__HIP_DEVICE_COMPILE__)
#    define G4VG_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#    define G4VG_UNREACHABLE __assume(false)
#else
#    define G4VG_UNREACHABLE
#endif

//---------------------------------------------------------------------------//

#define G4VG_DEBUG_THROW_(MSG, WHICH) \
    throw ::g4vg::DebugError(MSG, __FILE__, __LINE__)
#define G4VG_DEBUG_ASSERT_(COND, WHICH)      \
    do                                       \
    {                                        \
        if (G4VG_UNLIKELY(!(COND)))          \
        {                                    \
            G4VG_DEBUG_THROW_(#COND, WHICH); \
        }                                    \
    } while (0)
#define G4VG_NDEBUG_ASSUME_(COND)   \
    do                              \
    {                               \
        if (G4VG_UNLIKELY(!(COND))) \
        {                           \
            ::g4vg::unreachable();  \
        }                           \
    } while (0)
#define G4VG_NOASSERT_(COND)    \
    do                          \
    {                           \
        if (false && (COND)) {} \
    } while (0)

#define G4VG_DEBUG_FAIL(MSG, WHICH)    \
    do                                 \
    {                                  \
        G4VG_DEBUG_THROW_(MSG, WHICH); \
        ::g4vg::unreachable();         \
    } while (0)

#define G4VG_RUNTIME_THROW(WHICH, WHAT, COND) \
    throw ::g4vg::RuntimeError(WHAT, COND, __FILE__, __LINE__, )

#if G4VG_DEBUG
#    define G4VG_EXPECT(COND) G4VG_DEBUG_ASSERT_(COND, precondition)
#    define G4VG_ASSERT(COND) G4VG_DEBUG_ASSERT_(COND, internal)
#    define G4VG_ENSURE(COND) G4VG_DEBUG_ASSERT_(COND, postcondition)
#    define G4VG_ASSUME(COND) G4VG_DEBUG_ASSERT_(COND, assumption)
#    define G4VG_ASSERT_UNREACHABLE() \
        G4VG_DEBUG_FAIL("unreachable code point encountered", unreachable)
#else
#    define G4VG_EXPECT(COND) G4VG_NOASSERT_(COND)
#    define G4VG_ASSERT(COND) G4VG_NOASSERT_(COND)
#    define G4VG_ENSURE(COND) G4VG_NOASSERT_(COND)
#    define G4VG_ASSERT_UNREACHABLE() ::g4vg::unreachable()
#endif

#define G4VG_VALIDATE(COND, MSG)                                       \
    do                                                                 \
    {                                                                  \
        if (G4VG_UNLIKELY(!(COND)))                                    \
        {                                                              \
            std::ostringstream g4vg_runtime_msg_;                      \
            g4vg_runtime_msg_ MSG;                                     \
            G4VG_RUNTIME_THROW(::g4vg::RuntimeError::validate_err_str, \
                               g4vg_runtime_msg_.str(),                \
                               #COND);                                 \
        }                                                              \
    } while (0)

namespace g4vg
{
//---------------------------------------------------------------------------//

//! Invoke undefined behavior
[[noreturn]] inline void unreachable()
{
    G4VG_UNREACHABLE;
}

//---------------------------------------------------------------------------//
/*!
 * Error thrown by G4VG assertions.
 */
class DebugError : public std::logic_error
{
  public:
    // Construct from debug attributes
    explicit DebugError(char const* condition, char const* file, int line);

    DebugError(DebugError const&) = default;
    DebugError(DebugError&&) = default;

    // Default destructor to anchor vtable
    ~DebugError() override;
};

//---------------------------------------------------------------------------//
/*!
 * Error thrown by working code from unexpected runtime conditions.
 */
class RuntimeError : public std::runtime_error
{
  public:
    // Construct from details
    explicit RuntimeError(char const* which,
                          std::string what,
                          std::string condition,
                          std::string file,
                          int line);

    RuntimeError(RuntimeError const&) = default;
    RuntimeError(RuntimeError&&) = default;

    // Default destructor to anchor vtable
    ~RuntimeError() override;

    //!@{
    //! String constants for "which" error message
    static char const validate_err_str[];
    //!@}
};

//---------------------------------------------------------------------------//
}  // namespace g4vg
