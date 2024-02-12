#----------------------------------*-CMake-*----------------------------------#
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}" G4VG_SOURCE_DIR)
configure_file(g4vg_test_config.h.in g4vg_test_config.h @ONLY)

add_executable(g4vg_test
  G4VG.test.cc
)
target_link_libraries(g4vg_test
  GTest::GTest GTest::gtest_main # For testing
  G4VG::g4vg # Code to be tested
  VecGeom::vecgeom # To build and check VecGeom objects
  ${Geant4_LIBRARIES} # To set up and load Geant4
  Celeritas::geocel # To wrap geant4 exceptions
)
target_include_directories(g4vg_test
  PUBLIC "${PROJECT_BINARY_DIR}/test"
)
add_test(NAME g4vg_test COMMAND "$<TARGET_FILE:g4vg_test>")

#-----------------------------------------------------------------------------#
