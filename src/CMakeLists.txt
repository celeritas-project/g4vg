#------------------------------- -*- cmake -*- -------------------------------#
# Copyright G4VG contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

if(BUILD_SHARED_LIBS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

#-----------------------------------------------------------------------------#
# IMPLEMENTATION CODE
#-----------------------------------------------------------------------------#

add_library(g4vg_impl OBJECT
  g4vg_impl/Assert.cc
  g4vg_impl/Converter.cc
  g4vg_impl/LogicalVolumeConverter.cc
  g4vg_impl/SolidConverter.cc
)
target_include_directories(g4vg_impl
  PRIVATE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)
target_compile_features(g4vg_impl PUBLIC cxx_std_17)
target_compile_definitions(g4vg_impl PRIVATE G4VG_DEBUG=$<BOOL:${G4VG_DEBUG}>)

install(TARGETS g4vg_impl
  EXPORT g4vg-targets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  COMPONENT runtime
)

if(Geant4_VERSION VERSION_LESS 10.6)
  set(_g4vg_impl_libs ${Geant4_LIBRARIES})
else()
  # Geant4 might be built with static and/or shared libraries, so link
  # static-static and shared-shared if possible, falling back to shared-static if needed.
  set(_g4_link_ext )
  if((NOT Geant4_shared_FOUND) OR (Geant4_static_FOUND AND NOT BUILD_SHARED_LIBS))
    set(_g4_link_ext "-static")
  endif()

  # Whilst geocel contains the primary Geant4/VecGeom dependency, we must also
  # declare the geometry/gdml deps explicitly so that clients can pick them
  # up correctly through LINK_DEPENDENT_LIBRARIES should the linker require it.
  # See: https://discourse.cmake.org/t/use-cases-for-imported-link-dependent-libraries
  set(_g4vg_impl_libs
    Geant4::G4geometry${_g4_link_ext}
    $<IF:$<TARGET_EXISTS:Geant4::G4gdml${_g4_link_ext}>,Geant4::G4gdml${_g4_link_ext},Geant4::G4persistency${_g4_link_ext}>
  )
endif()

target_link_libraries(g4vg_impl
  PRIVATE ${_g4vg_impl_libs}
  PUBLIC VecGeom::vecgeom
)

#-----------------------------------------------------------------------------#
# Add the library
cuda_rdc_add_library(g4vg
  G4VG.cc
  $<TARGET_OBJECTS:g4vg_impl>
)
cuda_rdc_target_link_libraries(g4vg
  PRIVATE
    g4vg_impl
)
cuda_rdc_target_include_directories(g4vg
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Alias the library for downstream code
add_library(G4VG::g4vg ALIAS g4vg)
target_compile_features(g4vg PUBLIC cxx_std_11)

# Install all targets to lib/
install(TARGETS g4vg
  EXPORT g4vg-targets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  COMPONENT runtime
)

# Install the only public source header
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/G4VG.hh"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  COMPONENT development
)

#-----------------------------------------------------------------------------#
