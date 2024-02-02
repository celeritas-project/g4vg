#---------------------------------*-CMake-*----------------------------------#
# Copyright 2020-2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.14...3.28)
project(G4VG VERSION 0.1.0 LANGUAGES CXX)

#----------------------------------------------------------------------------#
# Utilities

include(GNUInstallDirs)

macro(g4vg_set_default name value)
  if(NOT DEFINED ${name})
    message(VERBOSE "G4VG: set default ${name}=${value}")
    set(${name} "${value}")
  endif()
endmacro()

#----------------------------------------------------------------------------#
# Options

option(G4VG_BUILD_TESTS "Build G4VG unit tests" OFF)
g4vg_set_default(BUILD_TESTING ${G4VG_BUILD_TESTS})

g4vg_set_default(CMAKE_CXX_EXTENSIONS OFF)

#----------------------------------------------------------------------------#
# Add code

add_subdirectory(external)
add_subdirectory(src)

#----------------------------------------------------------------------------#
# Add tests

if(BUILD_TESTING)
  include(CTest)
endif()
if(G4VG_BUILD_TESTS)
  if(NOT GTest_FOUND)
    find_package(GTest 1.10 REQUIRED)
  endif()
  add_subdirectory(test)
endif()

#----------------------------------------------------------------------------#
# Export CMake for installation downstream

set(G4VG_INSTALL_CMAKECONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/G4VG")

# Install 'G4VGTargets.cmake', included by G4VGConfig.cmake, which
# references the targets we install.
install(EXPORT g4vg-targets
  FILE G4VGTargets.cmake
  NAMESPACE G4VG::
  DESTINATION "${G4VG_INSTALL_CMAKECONFIGDIR}"
  COMPONENT development
)


