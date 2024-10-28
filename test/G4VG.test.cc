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
#include <VecGeom/volumes/LogicalVolume.h>
#include <VecGeom/volumes/UnplacedVolume.h>
#include <geocel/ScopedGeantExceptionHandler.hh>
#include <gtest/gtest.h>

#include "g4vg_test_config.h"

using VGLV = vecgeom::LogicalVolume;

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
    void TearDown() override;

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
    celeritas::ScopedGeantExceptionHandler scope_exceptions;
    G4GDMLParser gdml_parser;
    gdml_parser.SetStripFlag(true);
    gdml_parser.Read(filename, /* validate_gdml_schema = */ false);

    // Save world volume
    world_ = gdml_parser.GetWorldVolume();
    ASSERT_TRUE(world_) << "GDML parser did not return world volume";

    // Save the basename
    loaded_basename = this_basename;
}

void G4VGTestBase::TearDown()
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
    EXPECT_EQ(25, converted.logical_volumes.size());

    // Set world in VecGeom manager
    auto& vg_manager = vecgeom::GeoManager::Instance();
    vg_manager.RegisterPlacedVolume(converted.world);
    vg_manager.SetWorldAndClose(converted.world);

    // Check volumes
    std::vector<std::string> ordered_g4_names(converted.logical_volumes.size());
    std::vector<double> ordered_vg_capacities(ordered_g4_names.size());

    for (std::size_t vgid = 0; vgid < ordered_g4_names.size(); ++vgid)
    {
        // Save Geant4 name
        auto* g4lv = converted.logical_volumes[vgid];
        if (!g4lv)
        {
            continue;
        }
        std::string const& g4name = g4lv->GetName();
        ordered_g4_names[vgid] = g4name;

        // Save VecGeom name
        auto* vglv = vg_manager.FindLogicalVolume(vgid);
        ASSERT_TRUE(vglv);
        std::string vgname{vglv->GetName()};
        EXPECT_EQ(0, vgname.find(g4name)) << "Expected Geant4 name '" << g4name
                                          << "' to be at the start of "
                                             "VecGeom name '"
                                          << vgname << "'";

        // Check volume
        auto* vguv = vglv->GetUnplacedVolume();
        ASSERT_TRUE(vguv);
        ordered_vg_capacities[vgid] = vguv->Capacity();
    }

    std::vector<std::string> const expected_g4_names
        = {"box500",   "cone1",     "para1",      "sphere1",    "parabol1",
           "trap1",    "trd1",      "trd2",       "trd3",       "trd3_refl",
           "tube100",  "",          "",           "",           "",
           "boolean1", "polycone1", "genPocone1", "ellipsoid1", "tetrah1",
           "orb1",     "polyhedr1", "hype1",      "elltube1",   "ellcone1",
           "arb8b",    "arb8a",     "xtru1",      "World"};
    EXPECT_EQ(expected_g4_names, ordered_g4_names);

    std::vector<double> const expected_capacities
        = {1.25e+08,    1.14982e+08, 3.36e+08,    1.13846e+08, 1.13099e+08,
           1.512e+08,   1.4e+08,     1.4e+08,     1.4e+08,     1.4e+08,
           1.13097e+07, 0,           0,           0,           0,
           1.16994e+08, 2.72926e+07, 2.08567e+08, 4.41582e+07, 1.06667e+08,
           2.68083e+08, 2.23013e+08, 7.75367e+07, 1.50796e+08, 4.96372e+06,
           6.81667e+08, 6.05e+08,    4.505e+06,   1.08e+11};
    ASSERT_EQ(expected_capacities.size(), ordered_vg_capacities.size());
    for (std::size_t i = 0; i != expected_capacities.size(); ++i)
    {
        EXPECT_NEAR(expected_capacities[i], ordered_vg_capacities[i], 1e6);
    }
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
