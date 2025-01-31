#------------------------------- -*- cmake -*- -------------------------------#
# Copyright G4VG contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

# - Raw link test
add_executable(g4vg_link_test G4VG.link.test.cc)
cuda_rdc_target_link_libraries(g4vg_link_test PRIVATE G4VG::g4vg)

# - Primary code test
file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}" G4VG_SOURCE_DIR)
configure_file(g4vg_test_config.h.in g4vg_test_config.h @ONLY)

add_executable(g4vg_test
  G4VG.test.cc
)
cuda_rdc_target_link_libraries(g4vg_test
  GTest::GTest GTest::gtest_main # For testing
  G4VG::g4vg # Code to be tested
  VecGeom::vecgeom # To build and check VecGeom objects
  ${Geant4_LIBRARIES} # To set up and load Geant4
)
cuda_rdc_target_include_directories(g4vg_test
  PUBLIC "${PROJECT_BINARY_DIR}/test"
)
add_test(NAME g4vg_test COMMAND "$<TARGET_FILE:g4vg_test>")

#-----------------------------------------------------------------------------#
