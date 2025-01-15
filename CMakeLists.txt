#------------------------------- -*- cmake -*- -------------------------------#
# Copyright G4VG contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.14...3.28)
project(G4VG VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

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
# Dependencies
# Found here because we need the targets visible to both import Celeritas and
# to declare them as-needed in g4vg itself for correct downstream use

find_package(Geant4 REQUIRED)
find_package(VecGeom REQUIRED)

# Celeritas *might* be coming from a superproject...
if(NOT Celeritas_FOUND)
  find_package(Celeritas QUIET)
  if(NOT Celeritas_FOUND)
    add_subdirectory(external)
  endif()
endif()

#----------------------------------------------------------------------------#
# Add code

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

# Install 'G4VGConfig.cmake'
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/G4VGConfigVersion.cmake"
  COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  "cmake/G4VGConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/G4VGConfig.cmake"
  INSTALL_DESTINATION "${G4VG_INSTALL_CMAKECONFIGDIR}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/G4VGConfigVersion.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/G4VGConfig.cmake"
  DESTINATION "${G4VG_INSTALL_CMAKECONFIGDIR}"
  COMPONENT development
)

# Install 'G4VGTargets.cmake', included by G4VGConfig.cmake, which
# references the targets we install.
install(EXPORT g4vg-targets
  FILE G4VGTargets.cmake
  NAMESPACE G4VG::
  DESTINATION "${G4VG_INSTALL_CMAKECONFIGDIR}"
  COMPONENT development
)
