#----------------------------------*-CMake-*----------------------------------#
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

#-----------------------------------------------------------------------------#
# Add the library
cuda_rdc_add_library(g4vg SHARED
  G4VG.cc
)
cuda_rdc_target_link_libraries(g4vg
  PRIVATE Celeritas::geocel
)
cuda_rdc_target_include_directories(g4vg
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Alias the library for downstream code
add_library(G4VG::g4vg ALIAS g4vg)

# Install all targets to lib/
install(TARGETS g4vg
  EXPORT g4vg-targets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  COMPONENT runtime
)

# C++ source headers
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT development
  FILES_MATCHING REGEX ".*\\.hh?$"
)

#-----------------------------------------------------------------------------#
