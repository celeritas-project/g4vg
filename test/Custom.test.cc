//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Custom.test.cc
//---------------------------------------------------------------------------//

#include <G4DisplacedSolid.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>
#include <G4Orb.hh>
#include <G4PVPlacement.hh>
#include <G4SolidStore.hh>
#include <G4ThreeVector.hh>
#include <gtest/gtest.h>

#include "G4VG.hh"
#include "TestBase.hh"

namespace g4vg
{
namespace test
{
//---------------------------------------------------------------------------//
using CustomTestBase = TestBase;

//---------------------------------------------------------------------------//
class DisplacedTestBase : public CustomTestBase
{
  protected:
    std::string basename() const final { return "displaced"; }
    G4VPhysicalVolume* build_world() final;
};

G4VPhysicalVolume* DisplacedTestBase::build_world()
{
    G4Material* mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

    auto* world_s = new G4Orb("world_solid", 100);
    auto* world_l = new G4LogicalVolume(world_s, mat, "world");
    auto* world_p = new G4PVPlacement(G4Transform3D{},
                                      world_l,
                                      "world_pv",
                                      /* parent = */ nullptr,
                                      /* many = */ false,
                                      /* copy_no = */ 0);

    auto* dright_s = new G4Orb("dright_solid", 10);
    auto* dright_l = new G4LogicalVolume(dright_s, mat, "dright");
    new G4PVPlacement(/* rotation = */ nullptr,
                      G4ThreeVector(25.0, 0.0, 0.0),
                      dright_l,
                      "dright_pv",
                      /* parent = */ world_l,
                      /* many = */ false,
                      /* copy_no = */ 0);

    auto* dleft_underlying = new G4Orb("dleft_base", 10);
    auto* dleft_s = new G4DisplacedSolid(
        "dleft_solid", dleft_underlying, nullptr, G4ThreeVector(-25.0, 0, 0));
    auto* dleft_l = new G4LogicalVolume(dleft_s, mat, "dleft");
    new G4PVPlacement(/* rotation = */ nullptr,
                      G4ThreeVector(0.0, 0.0, 0.0),
                      dleft_l,
                      "dleft_pv",
                      /* parent = */ world_l,
                      /* many = */ false,
                      /* copy_no = */ 0);

    return world_p;
}

TEST_F(DisplacedTestBase, default_options)
{
    auto result = this->run(Options{});

    TestResult ref;
    ref.lv_name = {
        "world",
        "dright",
        "dleft",
    };
    ref.solid_capacity = {
        4188790.20478639,
        4188.79020478639,
        4199.11397533979,
    };
    ref.pv_name = {
        "dright_pv",
        "dleft_pv",
        "world_pv",
    };
    ref.copy_no = {
        0,
        0,
        0,
    };
    // Boolean volume calculation differs across platforms
    EXPECT_TRUE(result.solid_capacity.size() >= 3
                && result.solid_capacity[2] > 0);
    ref.solid_capacity[2] = 0;
    result.solid_capacity[2] = 0;
    result.expect_eq(ref);
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
