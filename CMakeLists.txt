#---------------------------------*-CMake-*----------------------------------#
# Copyright 2020-2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.14...3.28)
project(G4VG VERSION 0.1.0 LANGUAGES CXX)

#----------------------------------------------------------------------------#
# Includes and setup

include(GNUInstallDirs)

#----------------------------------------------------------------------------#
# Add code

add_subdirectory(external)

