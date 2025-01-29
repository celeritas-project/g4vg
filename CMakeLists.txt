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
option(G4VG_DEBUG "Add runtime assertions" ON)
g4vg_set_default(BUILD_TESTING ${G4VG_BUILD_TESTS})

g4vg_set_default(CMAKE_CXX_EXTENSIONS OFF)

#----------------------------------------------------------------------------#
# Dependencies
# Found here because we need the targets visible to both import Celeritas and
# to declare them as-needed in g4vg itself for correct downstream use

find_package(Geant4 REQUIRED)
# Minimum version is 1.2.8 due to the use of VecGeom::Logger
find_package(VecGeom 1.2.8 REQUIRED)

# Celeritas *might* be coming from a superproject...
if(G4VG_BUILD_TESTS)
  if(NOT Celeritas_FOUND)
    find_package(Celeritas QUIET)
    if(NOT Celeritas_FOUND)
      add_subdirectory(external)
    endif()
  endif()
endif()

if(NOT Celeritas_FOUND)
  # Load CUDA RDC utils from VecGeom
  include(CudaRdcUtils)
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

set(G4VG_CMAKE_CONFIG_DIRECTORY
  "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(G4VG_INSTALL_CMAKECONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/G4VG")

# Build list of CMake files to install
set(_cmake_files)
if(VecGeom_VERSION VERSION_LESS 1.2.10)
  # Only install RDC utils if not available through vecgeom
  list(APPEND _cmake_files
    "${PROJECT_SOURCE_DIR}/cmake/CudaRdcUtils.cmake"
  )
endif()
foreach(_dep Geant4 VecGeom)
  list(APPEND _cmake_files "${PROJECT_SOURCE_DIR}/cmake/Find${_dep}.cmake")
endforeach()

# Save variables
set(G4VG_EXPORT_VARIABLES)
macro(g4vg_export_var varname)
  list(APPEND G4VG_EXPORT_VARIABLES "set(${varname} \"${${varname}}\")")
endmacro()
g4vg_export_var(G4VG_VERSION_STRING)
g4vg_export_var(G4VG_DEBUG)

list(APPEND G4VG_EXPORT_VARIABLES "\n# Hints for upstream dependencies")
foreach(_key
  VecCore_DIR VecGeom_DIR
  Geant4_DIR CLHEP_DIR ZLIB_DIR EXPAT_DIR XercesC_DIR PTL_DIR
  EXPAT_INCLUDE_DIR EXPAT_LIBRARY
  XercesC_LIBRARY XercesC_INCLUDE_DIR
)
  set(_val "${${_key}}")
  if(_val)
    set(_cache_val "$CACHE{${_key}}")
    if(_cache_val)
      list(APPEND G4VG_EXPORT_VARIABLES
        "set(${_key} \"${_cache_val}\" CACHE PATH \"Set by G4VGConfig.cmake\")"
      )
    else()
    list(APPEND G4VG_EXPORT_VARIABLES
      "if(NOT DEFINED ${_key})"
      "  set(${_key} \"${_val}\")"
      "endif()"
    )
  endif()
  endif()
endforeach()
list(JOIN G4VG_EXPORT_VARIABLES "\n" G4VG_EXPORT_VARIABLES)

# Install 'G4VGConfig.cmake'
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${G4VG_CMAKE_CONFIG_DIRECTORY}/G4VGConfigVersion.cmake"
  COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  "cmake/G4VGConfig.cmake.in"
  "${G4VG_CMAKE_CONFIG_DIRECTORY}/G4VGConfig.cmake"
  INSTALL_DESTINATION "${G4VG_INSTALL_CMAKECONFIGDIR}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
  "${G4VG_CMAKE_CONFIG_DIRECTORY}/G4VGConfigVersion.cmake"
  "${G4VG_CMAKE_CONFIG_DIRECTORY}/G4VGConfig.cmake"
  DESTINATION "${G4VG_INSTALL_CMAKECONFIGDIR}"
  COMPONENT development
)

# Export targets to the build tree
export(EXPORT g4vg-targets
  FILE "${G4VG_CMAKE_CONFIG_DIRECTORY}/G4VGTargets.cmake"
  NAMESPACE G4VG::
)

# Install 'G4VGTargets.cmake', included by G4VGConfig.cmake, which
# references the targets we install.
install(EXPORT g4vg-targets
  FILE G4VGTargets.cmake
  NAMESPACE G4VG::
  DESTINATION "${G4VG_INSTALL_CMAKECONFIGDIR}"
  COMPONENT development
)
