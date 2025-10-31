//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file NestedReplicaParametrization.test.cc
//---------------------------------------------------------------------------//

#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>
#include <G4PVParameterised.hh>
#include <G4PVPlacement.hh>
#include <G4PVReplica.hh>
#include <G4SystemOfUnits.hh>
#include <G4ThreeVector.hh>
#include <gtest/gtest.h>

#include "G4VG.hh"
#include "ICRP110PhantomNestedParameterisation.hh"
#include "TestBase.hh"

namespace g4vg
{
namespace test
{
//---------------------------------------------------------------------------//
using NestedReplicaParametrizationTestBase = TestBase;

//---------------------------------------------------------------------------//
class NestedReplicaParametrizationTest
    : public NestedReplicaParametrizationTestBase
{
  protected:
    std::string basename() const final { return "voxel"; }
    G4VPhysicalVolume* build_world() final;
};

G4VPhysicalVolume* NestedReplicaParametrizationTest::build_world()
{
    G4Material* mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

    auto* world_s = new G4Box("world_solid", 100 * cm, 100 * cm, 100 * cm);
    auto* world_l = new G4LogicalVolume(world_s, mat, "world");
    auto* world_p = new G4PVPlacement(G4Transform3D{},
                                      world_l,
                                      "world_pv",
                                      /* parent = */ nullptr,
                                      /* many = */ false,
                                      /* copy_no = */ 0);

    // Build materials
    auto elH = std::make_unique<G4Element>(
        "Hydrogen", "H", /* Z = */ 1., /* A = */ 1.01 * g / mole);

    std::vector<G4Material*> materials;
    materials.push_back(mat);
    for (int i = 1; i < 8; ++i)
    {
        auto* new_mat = new G4Material(
            "h" + std::to_string(i * 10), i * 0.1 * g / cm3, 1, kStateGas);
        new_mat->AddElement(elH.get(), 1);
        materials.push_back(new_mat);
    }

    // Voxel grid parameters
    std::array<G4int, 3> num_voxels = {2, 3, 5};
    G4int total_voxels = num_voxels[0] * num_voxels[1] * num_voxels[2];

    std::vector<std::size_t> material_indices(total_voxels);
    for (G4int iz = 0; iz < num_voxels[2]; ++iz)
    {
        for (G4int iy = 0; iy < num_voxels[1]; ++iy)
        {
            for (G4int ix = 0; ix < num_voxels[0]; ++ix)
            {
                G4int index = ix + num_voxels[0] * (iy + num_voxels[1] * iz);
                material_indices[index] = index % materials.size();
            }
        }
    }

    G4ThreeVector half_voxel_dim(0.5 * cm, 0.5 * cm, 0.5 * cm);

    auto* container_s = new G4Box("phantom_container",
                                  num_voxels[0] * half_voxel_dim.x(),
                                  num_voxels[1] * half_voxel_dim.y(),
                                  num_voxels[2] * half_voxel_dim.z());
    auto* container_l
        = new G4LogicalVolume(container_s, mat, "phantom_container");

    G4ThreeVector container_pos(0., 0., 0.);
    new G4PVPlacement(/* rotation = */ nullptr,
                      container_pos,
                      container_l,
                      "phantom_container",
                      /* parent = */ world_l,
                      /* many = */ false,
                      /* copy_no = */ 1);

    // Y-axis replica
    auto* rep_y_s = new G4Box("rep_y",
                              num_voxels[0] * half_voxel_dim.x(),
                              half_voxel_dim.y(),
                              num_voxels[2] * half_voxel_dim.z());
    auto* rep_y_l = new G4LogicalVolume(rep_y_s, mat, "rep_y");
    new G4PVReplica("rep_y",
                    rep_y_l,
                    container_l,
                    kYAxis,
                    num_voxels[1],
                    half_voxel_dim.y() * 2.);

    // X-axis replica
    auto* rep_x_s = new G4Box("rep_x",
                              half_voxel_dim.x(),
                              half_voxel_dim.y(),
                              num_voxels[2] * half_voxel_dim.z());
    auto* rep_x_l = new G4LogicalVolume(rep_x_s, mat, "rep_x");
    new G4PVReplica("rep_x",
                    rep_x_l,
                    rep_y_l,
                    kXAxis,
                    num_voxels[0],
                    half_voxel_dim.x() * 2.);

    // Parameterized voxels along Z-axis
    auto* voxel_s = new G4Box(
        "voxel", half_voxel_dim.x(), half_voxel_dim.y(), half_voxel_dim.z());
    auto* voxel_l = new G4LogicalVolume(voxel_s, mat, "voxel");

    auto* param = new VoxelParameterisation(
        half_voxel_dim, materials, num_voxels, material_indices);

    new G4PVParameterised(
        "voxel", voxel_l, rep_x_l, kZAxis, num_voxels[2], param);

    return world_p;
}

TEST_F(NestedReplicaParametrizationTest, default_options)
{
    auto result = this->run(Options{});

    TestResult ref;
    ref.lv_name = {
        "world",
        "phantom_container",
        "rep_y",
        "rep_x",
        "voxel",
    };
    ref.pv_name = {
        "phantom_container",
        "rep_y",
        "rep_x",
        "voxel",
        "world_pv",
    };

    result.expect_eq(ref);
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
