//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file G4VG.test.cc
//---------------------------------------------------------------------------//
#include "G4VG.hh"

#include <G4GDMLParser.hh>
#include <VecGeom/management/GeoManager.h>

#include "g4vg_test_config.h"

namespace g4vg
{
namespace test
{
//---------------------------------------------------------------------------//

class G4VGTestBase : public ::testing::Test
{
  protected:
    virtual std::string basename() const = 0;

    void SetUp() override;
    void TearDown();

    G4VPhysicalVolume const* g4world() const { return world_; }

  private:
    G4VPhysicalVolume* world_{nullptr};
};

//---------------------------------------------------------------------------//
/*!
 * Load Geant4 geometry during setup.
 */
void G4VGTestBase::SetUp()
{
    // Guard against loading multiple geometry in the same run
    static std::string loaded_basename{};
    std::string this_basename = this->basename();

    if (!loaded_basename.empty())
    {
        if (this_basename != loaded_basename)
        {
            GTEST_SKIP() << "Cannot run two separate geometries in the same "
                            "execution: loaded "
                         << loaded_basename << " but this geometry is "
                         << this_basename;
        }
        // Otherwise the loaded file matches the current one; exit early
        return;
    }
    // Set the basename to a temporary value in case something goes wrong
    loaded_basename = "<FAILURE>";

    // Construct absolute path to GDML input
    std::string filename = g4vg_source_dir;
    filename += "/test/data/";
    filename += this_basename;
    filename += ".gdml";

    // Load and strip pointers
    G4GDMLParser gdml_parser;
    gdml_parser.SetStripFlag(true);
    gdml_parser.Read(filename, /* validate_gdml_schema = */ false);

    // Save world volume
    world_ = gdml_parser.GetWorldVolume();
    ASSERT_TRUE(world_) << "GDML parser did not return world volume";

    // Save the basename
    loaded_basename = this_basename;
}

void G4VGTest::TearDown()
{
    vecgeom::GeoManager::Instance().Clear();
}

//---------------------------------------------------------------------------//
class SolidsTest : public G4VGTestBase
{
  protected:
    std::string basename() const override { return "solids"; }
};

TEST_F(SolidsTest, default_options)
{
    auto converted = g4vg::convert(this->g4world());
    ASSERT_TRUE(converted.world);
    EXPECT_EQ(0, converted.volumes.size());

    // TODO: test
    converted.world->PrintContent();

    // Set world in VecGeom manager
    auto& vg_manager = vecgeom::GeoManager::Instance();
    vg_manager.RegisterPlacedVolume(converted.world);
    vg_manager.SetWorldAndClose(converted.world);
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
