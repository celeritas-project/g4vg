//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file G4VG.test.cc
//---------------------------------------------------------------------------//
#include "G4VG.hh"

#include <G4Material.hh>
#include <G4RunManager.hh>
#include <VecGeom/management/GeoManager.h>
#include <VecGeom/volumes/LogicalVolume.h>
#include <VecGeom/volumes/PlacedVolume.h>
#include <VecGeom/volumes/UnplacedVolume.h>
#include <gtest/gtest.h>

#include "LoadGdml.hh"
#include "Repr.hh"
#include "g4vg_test_config.h"

using VGLV = vecgeom::LogicalVolume;
using VGPV = vecgeom::VPlacedVolume;
using std::cout;
using std::endl;

namespace g4vg
{
namespace test
{
//---------------------------------------------------------------------------//
struct TestResult
{
    //! Name of the Geant4 volume corresponding to the VecGeom LV
    std::vector<std::string> lv_name;
    std::vector<double> solid_capacity;
    std::vector<std::string> pv_name;
    std::vector<int> copy_no;

    void print_ref() const;
    void expect_eq(TestResult const& reference) const;
};

void TestResult::print_ref() const
{
    // clang-format: off
    cout << "/***** REFERENCE RESULT *****/\n"
            "TestResult ref;\n"
            "ref.lv_name = "
         << repr(lv_name)
         << ";\n"
            "ref.solid_capacity = "
         << repr(solid_capacity)
         << ";\n"
            "ref.pv_name = "
         << repr(pv_name)
         << ";\n"
            "ref.copy_no = "
         << repr(copy_no)
         << ";\n"
            "result.expect_eq(ref);\n"
            "/***** END REFERENCE RESULT *****/\n";
    // clang-format: on
}

void TestResult::expect_eq(TestResult const& ref) const
{
    EXPECT_EQ(lv_name, ref.lv_name);
    ASSERT_EQ(solid_capacity.size(), ref.solid_capacity.size());
    ASSERT_EQ(ref.lv_name.size(), ref.solid_capacity.size());
    for (std::size_t i = 0; i != solid_capacity.size(); ++i)
    {
        if (ref.solid_capacity[i] == 0.0)
        {
            EXPECT_EQ(solid_capacity[i], 0)
                << "Solid for " << i << " = " << lv_name[i];
            continue;
        }
        else if (solid_capacity[i] == 0.0)
        {
            ADD_FAILURE() << "Got zero capacity for " << i << " = "
                          << lv_name[i];
            continue;
        }

        EXPECT_LT(std::fabs(solid_capacity[i] / ref.solid_capacity[i] - 1),
                  1e-4)
            << "Solid for " << lv_name[i] << " capacity is wrong: got "
            << solid_capacity[i] << " but expected " << ref.solid_capacity[i];
    }
    EXPECT_EQ(pv_name, ref.pv_name) << repr(pv_name);
    EXPECT_EQ(copy_no, ref.copy_no) << repr(copy_no);
}

//---------------------------------------------------------------------------//
class G4VGTestBase : public ::testing::Test
{
  protected:
    virtual std::string basename() const = 0;

    void SetUp() override;
    void TearDown() override;

    G4VPhysicalVolume const* g4world() const { return world_; }

    TestResult run(Options const& options)
    {
        TestResult result;
        this->run_impl(options, result);
        return result;
    }

  private:
    G4VPhysicalVolume* world_{nullptr};

    void run_impl(Options const& options, TestResult& result);
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
        if (this_basename == loaded_basename)
        {
            // Otherwise the loaded file matches the current one; exit early
            return;
        }
        else
        {
            // Reset geometry and materials
            G4RunManager::GetRunManager()->ReinitializeGeometry();
            G4Material::GetMaterialTable()->clear();
        }
    }
    // Set the basename to a temporary value in case something goes wrong
    loaded_basename = "<FAILURE>";

    // Construct absolute path to GDML input
    std::string filename = g4vg_source_dir;
    filename += "/test/data/";
    filename += this_basename;
    filename += ".gdml";

    // Save world volume
    world_ = load_gdml(filename);
    ASSERT_TRUE(world_) << "GDML parser did not return world volume";

    // Save the basename
    loaded_basename = this_basename;
}

void G4VGTestBase::TearDown()
{
    vecgeom::GeoManager::Instance().Clear();
}

// Use a separate "void" function to test due to ASSERT_ macros
void G4VGTestBase::run_impl(Options const& options, TestResult& result)
{
    // Convert
    auto converted = g4vg::convert(this->g4world(), options);
    ASSERT_TRUE(converted.world);

    // Set world in VecGeom manager
    auto& vg_manager = vecgeom::GeoManager::Instance();
    vg_manager.RegisterPlacedVolume(converted.world);
    vg_manager.SetWorldAndClose(converted.world);

    // Set up result sizes
    result.lv_name.reserve(converted.logical_volumes.size());
    result.solid_capacity.reserve(converted.logical_volumes.size());
    result.pv_name.reserve(converted.physical_volumes.size());
    result.copy_no.reserve(converted.physical_volumes.size());

    // Process logical volumes
    for (std::size_t vgid = 0; vgid < converted.logical_volumes.size(); ++vgid)
    {
        // Check the LV exists
        auto* vglv = vg_manager.FindLogicalVolume(vgid);
        ASSERT_TRUE(vglv);
        std::string vgname{vglv->GetName()};

        // Save Geant4 name
        auto* g4lv = converted.logical_volumes[vgid];
        if (g4lv)
        {
            std::string const& g4name = g4lv->GetName();
            result.lv_name.push_back(g4name);

            // Save VecGeom name
            EXPECT_EQ(0, vgname.find(g4name))
                << "Expected Geant4 name '" << g4name
                << "' to be at the start of VecGeom name '" << vgname << "'";
        }
        else if (vgname.find("[TEMP]") == 0)
        {
            // Don't add capacity for temporary volumes
            continue;
        }

        // Check solid capacity/volume
        auto* vguv = vglv->GetUnplacedVolume();
        ASSERT_TRUE(vguv);
        result.solid_capacity.push_back(vguv->Capacity());
    }

    // Process physical volumes
    for (std::size_t vgid = 0; vgid < converted.physical_volumes.size(); ++vgid)
    {
        // Save Geant4 name
        auto* g4pv = converted.physical_volumes[vgid];
        if (!g4pv)
        {
            continue;
        }
        std::string const& g4name = g4pv->GetName();
        result.pv_name.push_back(g4name);

        // Check VecGeom name
        VGPV* vgpv = vg_manager.FindPlacedVolume(vgid);
        ASSERT_TRUE(vgpv);
        std::string vgname{vgpv->GetName()};
        EXPECT_EQ(0, vgname.find(g4name))
            << "Expected Geant4 name '" << g4name
            << "' to be at the start of VecGeom name '" << vgname << "'";

        // Save VecGeom copy number (may differ from single G4 volume if it's a
        // "stamped" instance of a replica/parameterised volume)
        result.copy_no.push_back(vgpv->GetCopyNo());
    }
}

//---------------------------------------------------------------------------//
class SolidsTest : public G4VGTestBase
{
  protected:
    std::string basename() const override { return "solids"; }

    static TestResult base_ref();
};

TestResult SolidsTest::base_ref()
{
    TestResult ref;
    ref.lv_name = {
        "box500",   "cone1",    "para1",     "sphere1",    "parabol1",
        "trap1",    "trd1",     "trd2",      "trd3",       "trd3_refl",
        "tube100",  "boolean1", "polycone1", "genPocone1", "ellipsoid1",
        "tetrah1",  "orb1",     "polyhedr1", "hype1",      "elltube1",
        "ellcone1", "arb8b",    "arb8a",     "xtru1",      "World",
    };
    ref.solid_capacity = {
        1.25e+08,
        114982291.12138642,
        3.36e+08,
        113845922.09897856,
        113098592.16629399,
        1.512e+08,
        1.4e+08,
        1.4e+08,
        1.4e+08,
        1.4e+08,
        11309733.552923255,
        116997640.39705618,
        27292586.178061325,
        208566845.61332238,
        44158226.338858128,
        106666666.66666667,
        268082573.10632899,
        223012581.98167434,
        77536732.14828524,
        150796447.37231007,
        4963716.3926718729,
        681666666.66666675,
        6.05e+08,
        4.505e+06,
        108000000000,
    };
    ref.pv_name = {
        "box500_PV",   "cone1_PV",     "para1_PV",      "sphere1_PV",
        "parabol1_PV", "trap1_PV",     "trd1_PV",       "reflNormal",
        "reflected",   "reflected",    "tube100_PV",    "boolean1_PV",
        "orb1_PV",     "polycone1_PV", "hype1_PV",      "polyhedr1_PV",
        "tetrah1_PV",  "arb8a_PV",     "arb8b_PV",      "ellipsoid1_PV",
        "elltube1_PV", "ellcone1_PV",  "genPocone1_PV", "xtru1_PV",
        "World_PV",
    };
    ref.copy_no = std::vector<int>(ref.pv_name.size(), 0);
    return ref;
}

TEST_F(SolidsTest, default_options)
{
    auto result = this->run(Options{});
    result.expect_eq(this->base_ref());

    {
        // Check that a pointer was appended
        auto& vg_manager = vecgeom::GeoManager::Instance();
        auto* vglv = vg_manager.FindLogicalVolume(0);
        ASSERT_TRUE(vglv);
        std::string name{vglv->GetName()};
        EXPECT_EQ(0, name.find("box5000x"))
            << "Expected pointer suffix at the end of VecGeom name '" << name
            << "'";
    }
}

TEST_F(SolidsTest, scale)
{
    Options opts;
    opts.compare_volumes = true;
    opts.scale = 0.1;  // i.e., target unit system is cm
    auto result = this->run(opts);

    auto ref = this->base_ref();
    for (auto& v : ref.solid_capacity)
    {
        v *= 0.001;
    }
    result.expect_eq(ref);
}

TEST_F(SolidsTest, no_pointers)
{
    Options opts;
    opts.compare_volumes = true;
    opts.append_pointers = false;
    auto result = this->run(opts);

    auto ref = this->base_ref();
    result.expect_eq(ref);

    {
        // Check that pointers were not appended
        auto& vg_manager = vecgeom::GeoManager::Instance();
        auto* vglv = vg_manager.FindLogicalVolume(0);
        ASSERT_TRUE(vglv);
        std::string name{vglv->GetName()};
        EXPECT_EQ("box500", name);
    }
}

//---------------------------------------------------------------------------//
class MultiLevelTest : public G4VGTestBase
{
  protected:
    std::string basename() const override { return "multi-level"; }

    static TestResult base_ref();
};

TestResult MultiLevelTest::base_ref()
{
    TestResult ref;
    ref.lv_name = {
        "sph",
        "tri",
        "box",
        "box2",
        "world",
    };
    ref.solid_capacity = {
        33510.321638291127,
        20784.609690826528,
        3.375e+06,
        3.375e+06,
        1.10592e+08,
    };
    ref.pv_name = {
        "topsph1",
        "boxsph1",
        "boxsph2",
        "boxtri",
        "topbox1",
        "boxsph1",
        "boxsph2",
        "boxtri",
        "topbox2",
        "topbox3",
        "topbox4",
        "world_PV",
    };
    ref.copy_no = {0, 31, 32, 1, 21, 41, 42, 11, 22, 23, 24, 0};
    return ref;
}

TEST_F(MultiLevelTest, default_options)
{
    auto result = this->run(Options{});
    result.expect_eq(this->base_ref());
}

TEST_F(MultiLevelTest, no_refl_factory)
{
    Options opts;
    opts.append_pointers = false;
    opts.reflection_factory = false;
    opts.compare_volumes = true;
    opts.verbose = true;
    auto result = this->run(opts);

    auto ref = this->base_ref();
    ref.lv_name = {
        "sph",
        "tri",
        "box",
        "box2",
        "tri_refl",
        "world",
        "box_refl",
        "sph_refl",
    };
    ref.solid_capacity = {
        33510.321638291127,
        20784.609690826528,
        3.375e+06,
        3.375e+06,
        -20784.609690826528,
        1.10592e+08,
        -3.375e+06,
        -33510.321638291127,
    };
    ref.pv_name = {
        "topsph1",
        "boxsph1",
        "boxsph2",
        "boxtri",
        "topbox1",
        "boxsph1",
        "boxsph2",
        "boxtri",
        "topbox2",
        "topbox3",
        "boxsph1",
        "boxsph2",
        "boxtri",
        "topbox4",
        "world_PV",
    };
    ref.copy_no = {0, 31, 32, 1, 21, 41, 42, 11, 22, 23, 31, 32, 1, 24, 0};
    result.expect_eq(ref);
}

//---------------------------------------------------------------------------//
class CmsEeBackDeeTest : public G4VGTestBase
{
  protected:
    std::string basename() const override { return "cms-ee-back-dee"; }

    static TestResult base_ref();
};

TestResult CmsEeBackDeeTest::base_ref()
{
    TestResult ref;
    ref.lv_name = {
        "EEBackPlate",
        "EESRing",
        "EEBackQuad",
        "EEBackDee",
        "EEBackQuad_refl",
        "EEBackPlate_refl",
        "EESRing_refl",
    };
    ref.solid_capacity = {
        132703256.27150133,
        29960299.032288227,
        929420652.20822978,
        1858841304.4164596,
        -929420652.20822978,
        -132703256.27150133,
        -29960299.032288227,
    };
    ref.pv_name = {
        "EEBackPlate",
        "EESRing",
        "EEBackQuad",
        "EEBackPlate",
        "EESRing",
        "EEBackQuad",
        "EEBackDee_PV",
    };
    ref.copy_no = {
        1,
        1,
        1,
        1,
        1,
        2,
        0,
    };
    return ref;
}

TEST_F(CmsEeBackDeeTest, no_refl_factory)
{
    Options opts;
    opts.append_pointers = false;
    opts.reflection_factory = false;
    opts.verbose = true;
    opts.compare_volumes = true;
    auto result = this->run(opts);
    result.expect_eq(this->base_ref());
}

//---------------------------------------------------------------------------//
class ReplicaTest : public G4VGTestBase
{
  protected:
    std::string basename() const override { return "B5"; }

    static TestResult base_ref();
};

TestResult ReplicaTest::base_ref()
{
    TestResult ref;
    ref.lv_name = {
        "magneticLogical",
        "hodoscope1Logical",
        "wirePlane1Logical",
        "chamber1Logical",
        "firstArmLogical",
        "hodoscope2Logical",
        "wirePlane2Logical",
        "chamber2Logical",
        "cellLogical",
        "EMcalorimeterLogical",
        "HadCalScintiLogical",
        "HadCalLayerLogical",
        "HadCalCellLogical",
        "HadCalColumnLogical",
        "HadCalorimeterLogical",
        "secondArmLogical",
        "worldLogical",
    };
    ref.solid_capacity = {
        6283185307.179586,
        400000.0,
        240000.0,
        2.4e+07,
        3.6e+10,
        400000.0,
        360000.0,
        3.6e+07,
        6.75e+06,
        5.4e+08,
        900000.0,
        4.5e+06,
        9e+07,
        1.8e+08,
        1.8e+09,
        1.12e+11,
        2.4e+12,
    };
    ref.pv_name = {
        "magneticPhysical",
    };
    // 15 direct placements
    ref.pv_name.insert(ref.pv_name.end(), 15, "hodoscope1Physical");
    ref.pv_name.push_back("wirePlane1Physical");
    // 5 direct placements
    ref.pv_name.insert(ref.pv_name.end(), 5, "chamber1Physical");
    ref.pv_name.push_back("firstArmPhysical");
    // 25 direct placements
    ref.pv_name.insert(ref.pv_name.end(), 25, "hodoscope2Physical");
    ref.pv_name.push_back("wirePlane2Physical");
    // 5 direct placements
    ref.pv_name.insert(ref.pv_name.end(), 5, "chamber2Physical");
    // 20x4 = 80 parameterized placements of cellLogical
    ref.pv_name.insert(ref.pv_name.end(), 80, "cellLogical_param");
    ref.pv_name.push_back("EMcalorimeterPhysical");
    ref.pv_name.push_back("HadCalScintiPhysical");
    // 20 replicas of HadCalLayerLogical
    ref.pv_name.insert(ref.pv_name.end(), 20, "HadCalLayerLogical_PV");
    // 2 replicas of HadCalCellLogical
    ref.pv_name.insert(ref.pv_name.end(), 2, "HadCalCellLogical_PV");
    // 10 replicas of HadCalColumnLogical
    ref.pv_name.insert(ref.pv_name.end(), 10, "HadCalColumnLogical_PV");
    ref.pv_name.push_back("HadCalorimeterPhysical");
    ref.pv_name.push_back("fSecondArmPhys");
    ref.pv_name.push_back("worldLogical_PV");

    ref.copy_no = {
        0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 0,  0,
        1,  2,  3,  4,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 0,  0,  1,  2,  3,  4,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
        54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
        72, 73, 74, 75, 76, 77, 78, 79, 0,  0,  0,  1,  2,  3,  4,  5,  6,  7,
        8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0,  1,  0,  1,  2,  3,
        4,  5,  6,  7,  8,  9,  0,  0,  0,
    };
    return ref;
}

TEST_F(ReplicaTest, default_options)
{
    auto result = this->run(Options{});
    result.expect_eq(this->base_ref());
}

//---------------------------------------------------------------------------//
class ZnenvTest : public G4VGTestBase
{
  protected:
    std::string basename() const override { return "znenv"; }

    static TestResult base_ref();
};

TestResult ZnenvTest::base_ref()
{
    TestResult ref;
    ref.lv_name = {
        "ZNF1",
        "ZNG1",
        "ZNF2",
        "ZNG2",
        "ZNF3",
        "ZNG3",
        "ZNF4",
        "ZNG4",
        "ZNST",
        "ZNSL",
        "ZN1",
        "ZNTX",
        "ZNEU",
        "ZNENV",
        "World",
    };
    ref.solid_capacity = {
        104.634670318625,
        360,
        104.634670318625,
        360,
        104.634670318625,
        360,
        104.634670318625,
        360,
        10240,
        112640,
        1239040,
        2478080,
        4956160,
        5252243.52,
        2000000000,
    };
    ref.pv_name = {
        "ZNF1_1",   "ZNG1_1",  "ZNF2_1",  "ZNG2_1",  "ZNF3_1",  "ZNG3_1",
        "ZNF4_1",   "ZNG4_1",  "ZNST_PV", "ZNST_PV", "ZNST_PV", "ZNST_PV",
        "ZNST_PV",  "ZNST_PV", "ZNST_PV", "ZNST_PV", "ZNST_PV", "ZNST_PV",
        "ZNST_PV",  "ZNSL_PV", "ZNSL_PV", "ZNSL_PV", "ZNSL_PV", "ZNSL_PV",
        "ZNSL_PV",  "ZNSL_PV", "ZNSL_PV", "ZNSL_PV", "ZNSL_PV", "ZNSL_PV",
        "ZN1_PV",   "ZN1_PV",  "ZNTX_PV", "ZNTX_PV", "ZNEU_1",  "WorldBoxPV",
        "World_PV",
    };
    ref.copy_no = {
        1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 2,  3, 4, 5, 6, 7, 8, 9, 10,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 1, 0, 1, 1, 1, 0,
    };
    return ref;
}

TEST_F(ZnenvTest, default_options)
{
    auto result = this->run(Options{});
    // result.print_ref();
    result.expect_eq(this->base_ref());
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
