#----------------------------------*-CMake-*----------------------------------#
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

include(FetchContent)

# Load Celeritas
FetchContent_Declare(
  Celeritas
  EXCLUDE_FROM_ALL # see below...
  URL https://github.com/celeritas-project/celeritas/archive/0f74ea935d72bed23b82fad98402bc68dd606d94.zip
)

# $ git "describe" "--tags" "--match" "v*" <hash> ->
# v0.5.0-dev-27-g0f74ea935
set(Celeritas_GIT_DESCRIBE "0.5.0" "-dev.27" "0f74ea935")

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
g4vg_set_default(CELERITAS_REAL_TYPE double)

# Build static libraries for celeritas
g4vg_set_default(BUILD_SHARED_LIBS OFF)
g4vg_set_default(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_VERSION VERSION_LESS 3.28)
  # See https://gitlab.kitware.com/cmake/cmake/-/issues/20167
  set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL YES)
endif()

# Load Celeritas
FetchContent_MakeAvailable(Celeritas)

#-----------------------------------------------------------------------------#
