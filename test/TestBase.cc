//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file TestBase.cc
//---------------------------------------------------------------------------//
#include "TestBase.hh"

#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4RunManager.hh>
#include <G4VNestedParameterisation.hh>
#include <VecGeom/management/GeoManager.h>
#include <VecGeom/volumes/LogicalVolume.h>
#include <VecGeom/volumes/PlacedVolume.h>
#include <VecGeom/volumes/UnplacedVolume.h>
#include <gtest/gtest.h>

#include "G4VG.hh"
#include "Repr.hh"

using std::cout;

using VGPV = vecgeom::VPlacedVolume;

namespace g4vg
{
namespace test
{
//---------------------------------------------------------------------------//

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
         << repr(copy_no) << ";\n";
    if (!nested.empty())
    {
        cout << "ref.nested = " << repr(nested) << ";\n";
    }
    cout << "result.expect_eq(ref);\n"
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
    EXPECT_EQ(nested, ref.nested) << repr(nested);
}

//---------------------------------------------------------------------------//
/*!
 * Load Geant4 geometry during setup.
 */
void TestBase::SetUp()
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

    // Save world volume
    world_ = this->build_world();
    ASSERT_TRUE(world_) << "GDML parser did not return world volume";

    // Save the basename
    loaded_basename = this_basename;
}

void TestBase::TearDown()
{
    vecgeom::GeoManager::Instance().Clear();
}

std::string TestBase::logical_volume_name(unsigned int lv_id) const
{
    // Check that a pointer was appended
    auto& vg_manager = vecgeom::GeoManager::Instance();
    auto* vglv = vg_manager.FindLogicalVolume(lv_id);
    EXPECT_TRUE(vglv);
    return vglv ? vglv->GetName() : std::string{};
}

// Use a separate "void" function to test due to ASSERT_ macros
void TestBase::run_impl(Options const& options, TestResult& result)
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

    // Process nested volume
    for (auto* g4pv : converted.nested_pv)
    {
        ASSERT_TRUE(g4pv);
        std::ostringstream os;

        os << g4pv->GetName() << ':';
        if (auto* nested = dynamic_cast<G4VNestedParameterisation*>(
                g4pv->GetParameterisation()))
        {
            os << nested->GetNumberOfMaterials();
        }
        else
        {
            os << "ERROR";
        }
        result.nested.push_back(std::move(os).str());
    }
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
