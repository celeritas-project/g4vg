#------------------------------- -*- cmake -*- -------------------------------#
# Copyright Celeritas contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

include(FetchContent)

# Load Celeritas
FetchContent_Declare(
  Celeritas
  EXCLUDE_FROM_ALL # see below...
  URL https://github.com/celeritas-project/celeritas/archive/909e2d1136e3dd330363d60056137014daa82761.zip
)

# $ git "describe" "--tags" "--match" "v*" <hash> ->
# v0.5.1-6-g909e2d113
set(Celeritas_GIT_DESCRIBE "0.5.1" "-6" "909e2d113")

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
g4vg_set_default(CELERITAS_USE_Python OFF)
g4vg_set_default(CELERITAS_USE_ROOT OFF)
g4vg_set_default(CELERITAS_USE_SWIG OFF)
g4vg_set_default(CELERITAS_USE_VecGeom ON)

# Use VecGeom, mm, and double precision
g4vg_set_default(CELERITAS_CORE_GEO VecGeom)
g4vg_set_default(CELERITAS_UNITS CLHEP)
g4vg_set_default(CELERITAS_double double)

# Build static libraries for celeritas
g4vg_set_default(BUILD_SHARED_LIBS OFF)
g4vg_set_default(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Load Celeritas
if(CMAKE_VERSION VERSION_LESS 3.28)
  # See https://gitlab.kitware.com/cmake/cmake/-/issues/20167
  if(NOT celeritas_POPULATED)
    FetchContent_Populate(Celeritas)
    add_subdirectory(${celeritas_SOURCE_DIR} ${celeritas_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
else()
  FetchContent_MakeAvailable(Celeritas)
endif()

#-----------------------------------------------------------------------------#
