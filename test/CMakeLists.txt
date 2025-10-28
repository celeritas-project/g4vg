#------------------------------- -*- cmake -*- -------------------------------#
# Copyright G4VG contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

# CMake-provided module for test discovery
include(GoogleTest)

# - Raw link test
add_executable(g4vg_link_test G4VG.link.test.cc)
cuda_rdc_target_link_libraries(g4vg_link_test PRIVATE G4VG::g4vg)

# - Primary code test
file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}" G4VG_SOURCE_DIR)
configure_file(g4vg_test_config.h.in g4vg_test_config.h @ONLY)

add_library(g4vg_test_lib
  TestBase.cc
  ICRP110PhantomNestedParameterisation.cc
  ICRP110PhantomMaterial_Female.cc
)
cuda_rdc_target_link_libraries(g4vg_test_lib PUBLIC
  GTest::GTest GTest::gtest_main # For testing
  G4VG::g4vg # Code to be tested
  VecGeom::vecgeom # To build and check VecGeom objects
  ${Geant4_LIBRARIES} # To set up and load Geant4
)
cuda_rdc_target_include_directories(g4vg_test_lib
  PUBLIC "${PROJECT_BINARY_DIR}/test"
)

foreach(name IN ITEMS Gdml Custom NestedReplicaParametrization)
  string(TOLOWER "${name}_test" tgt)
  add_executable(${tgt} ${name}.test.cc)
  cuda_rdc_target_link_libraries(${tgt} PUBLIC g4vg_test_lib)
  gtest_discover_tests(${tgt})
endforeach()

#-----------------------------------------------------------------------------#
