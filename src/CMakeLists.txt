#----------------------------------*-CMake-*----------------------------------#
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

#-----------------------------------------------------------------------------#
# Add the library
cuda_rdc_add_library(g4vg SHARED
  G4VG.cc
  g4vg/Converter.cc
  g4vg/LogicalVolumeConverter.cc
  g4vg/SolidConverter.cc
)

# Whilst geocel contains the primary Geant4/VecGeom dependency, we must also
# declare the geometry/gdml deps explicitly so that clients can pick them
# up correctly through LINK_DEPENDENT_LIBRARIES should the linker require it.
# See: https://discourse.cmake.org/t/use-cases-for-imported-link-dependent-libraries
cuda_rdc_target_link_libraries(g4vg
  PRIVATE
    VecGeom::vecgeom
    Geant4::G4geometry
    $<IF:$<TARGET_EXISTS:Geant4::G4gdml>,Geant4::G4gdml,Geant4::G4persistency>
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
