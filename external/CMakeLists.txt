#----------------------------------*-CMake-*----------------------------------#
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

include(FetchContent)

macro(g4vg_set_default name value)
  if(NOT DEFINED ${name})
    message(VERBOSE "G4VG: set default ${name}=${value}")
    set(${name} "${value}")
  endif()
endmacro()

# Load Celeritas
FetchContent_Declare(
  Celeritas
  EXCLUDE_FROM_ALL
  GIT_REPOSITORY file:///Users/seth/Code/celeritas
  GIT_TAG        HEAD
)

# $ git "describe" "--tags" "--match" "v*" 5742b0c924; format as below
set(Celeritas_GIT_DESCRIBE "0.5.0" "-dev.9" "5742b0c92")

# Minimally build Celeritas
g4vg_set_default(CELERITAS_BUILD_DEMOS OFF)
g4vg_set_default(CELERITAS_BUILD_DOCS OFF)
g4vg_set_default(CELERITAS_BUILD_TESTS OFF)

g4vg_set_default(CELERITAS_USE_CUDA OFF)
g4vg_set_default(CELERITAS_USE_Geant4 ON)
g4vg_set_default(CELERITAS_USE_HIP OFF)
g4vg_set_default(CELERITAS_USE_HepMC3 OFF)
g4vg_set_default(CELERITAS_USE_JSON OFF)
g4vg_set_default(CELERITAS_USE_MPI OFF)
g4vg_set_default(CELERITAS_USE_OpenMP OFF)
g4vg_set_default(CELERITAS_USE_ROOT OFF)
g4vg_set_default(CELERITAS_USE_SWIG OFF)
g4vg_set_default(CELERITAS_USE_VecGeom ON)

# Use VecGeom, mm, and double precision
g4vg_set_default(CELERITAS_CORE_GEO VecGeom)
g4vg_set_default(CELERITAS_UNITS CLHEP)
g4vg_set_default(CELERITAS_REAL_TYPE double)

# Build static libraries for celeritas
g4vg_set_default(BUILD_SHARED_LIBS OFF)
g4vg_set_default(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Load Celeritas
FetchContent_MakeAvailable(Celeritas)

#-----------------------------------------------------------------------------#
