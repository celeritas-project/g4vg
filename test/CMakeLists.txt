#----------------------------------*-CMake-*----------------------------------#
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}" CELERITAS_SOURCE_DIR)
configure_file(g4vg_test_config.h.in g4vg_test_config.h @ONLY)

add_executable(g4vg_test
  G4VG.test.cc
)
target_link_libraries(g4vg_test
  GTest::GTest GTest::gtest_main
  G4VG::g4vg
  VecGeom::vecgeom
  ${Geant4_LIBRARIES}
)

#-----------------------------------------------------------------------------#
