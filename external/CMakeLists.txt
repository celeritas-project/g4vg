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
  # Current tip of celeritas develop - adjust as needed
  URL https://github.com/celeritas-project/celeritas/archive/5742b0c924a67bf02ec0ce9fc85719a70f7857e0.zip
)

# $ git "describe" "--tags" "--match" "v*" 5742b0c924; format as below
set(Celeritas_GIT_DESCRIBE "0.5.0" "-dev.9" "5742b0c92")
# Load Celeritas
FetchContent_MakeAvailable(Celeritas)

#-----------------------------------------------------------------------------#
