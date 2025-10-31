//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Custom.test.cc
//---------------------------------------------------------------------------//

#include <array>
#include <vector>
#include <G4Box.hh>
#include <G4DisplacedSolid.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>
#include <G4Orb.hh>
#include <G4PVParameterised.hh>
#include <G4PVPlacement.hh>
#include <G4PVReplica.hh>
#include <G4SolidStore.hh>
#include <G4SystemOfUnits.hh>
#include <G4ThreeVector.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VTouchable.hh>
#include <gtest/gtest.h>

#include "G4VG.hh"
#include "G4VNestedParameterisation.hh"
#include "G4VTouchable.hh"
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
class VoxelParameterisation final : public G4VNestedParameterisation
{
  public:
    VoxelParameterisation(G4ThreeVector const& half_voxel_size,
                          std::vector<G4Material*> materials,
                          std::array<int, 3> num_voxels,
                          std::vector<std::size_t> material_indices);

    ~VoxelParameterisation() override = default;

    G4Material* ComputeMaterial(G4VPhysicalVolume* currentVol,
                                int const repNo,
                                G4VTouchable const* parentTouch) override;

    int GetNumberOfMaterials() const override { return materials_.size(); }
    G4Material* GetMaterial(int i) const override { return materials_[i]; }

    void ComputeTransformation(int const no,
                               G4VPhysicalVolume* currentPV) const override;

    void ComputeDimensions(G4Box&,
                           int const,
                           G4VPhysicalVolume const*) const override;

    using G4VNestedParameterisation::ComputeDimensions;
    using G4VNestedParameterisation::ComputeMaterial;

  private:
    G4ThreeVector half_dim_;
    std::array<int, 3> num_voxels_;  // {nx, ny, nz}
    std::vector<G4Material*> materials_;
    std::vector<std::size_t> material_indices_;
};

VoxelParameterisation::VoxelParameterisation(
    G4ThreeVector const& half_voxel_size,
    std::vector<G4Material*> materials,
    std::array<int, 3> num_voxels,
    std::vector<std::size_t> material_indices)
    : half_dim_(half_voxel_size)
    , num_voxels_(num_voxels)
    , materials_(std::move(materials))
    , material_indices_(std::move(material_indices))
{
}

G4Material*
VoxelParameterisation::ComputeMaterial(G4VPhysicalVolume* physVol,
                                       int const iz,
                                       G4VTouchable const* parentTouch)
{
    if (parentTouch == nullptr)
        return materials_[0];

    int ix = parentTouch->GetReplicaNumber(0);
    int iy = parentTouch->GetReplicaNumber(1);

    int copyID = ix + num_voxels_[0] * (iy + num_voxels_[1] * iz);
    std::size_t matIndex = material_indices_[copyID];
    G4Material* mate = materials_[matIndex];

    physVol->GetLogicalVolume()->SetMaterial(mate);

    return mate;
}

void VoxelParameterisation::ComputeTransformation(
    int const copyNo, G4VPhysicalVolume* physVol) const
{
    G4double z_offset = (2. * static_cast<double>(copyNo) + 1.) * half_dim_.z()
                        - half_dim_.z() * num_voxels_[2];
    physVol->SetTranslation(G4ThreeVector(0., 0., z_offset));
}

void VoxelParameterisation::ComputeDimensions(G4Box& box,
                                              int const,
                                              G4VPhysicalVolume const*) const
{
    box.SetXHalfLength(half_dim_.x());
    box.SetYHalfLength(half_dim_.y());
    box.SetZHalfLength(half_dim_.z());
}

//---------------------------------------------------------------------------//
class NestedReplicaParametrizationTest : public CustomTestBase
{
  protected:
    std::string basename() const final { return "voxel"; }
    G4VPhysicalVolume* build_world() final;
};

G4VPhysicalVolume* NestedReplicaParametrizationTest::build_world()
{
    // Voxel grid parameters
    std::array<int, 3> const num_voxels = {2, 3, 5};
    int const total_voxels = num_voxels[0] * num_voxels[1] * num_voxels[2];
    G4ThreeVector half_voxel_dim(0.5 * cm, 0.5 * cm, 0.5 * cm);

    // Build materials
    auto elH = std::make_unique<G4Element>(
        "Hydrogen", "H", /* Z = */ 1., /* A = */ 1.01 * g / mole);

    std::vector<G4Material*> materials;
    for (int i = 0; i < 8; ++i)
    {
        auto* new_mat = new G4Material(
            "h" + std::to_string(i), (i + 1) * 0.1 * g / cm3, 1, kStateGas);
        new_mat->AddElement(elH.get(), 1);
        materials.push_back(new_mat);
    }

    std::vector<std::size_t> material_indices(total_voxels);
    for (int iz = 0; iz < num_voxels[2]; ++iz)
    {
        for (int iy = 0; iy < num_voxels[1]; ++iy)
        {
            for (int ix = 0; ix < num_voxels[0]; ++ix)
            {
                int index = ix + num_voxels[0] * (iy + num_voxels[1] * iz);
                material_indices[index] = index % materials.size();
            }
        }
    }

    // Create world
    G4Material* air = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    auto* world_s = new G4Box("world_solid", 100 * cm, 100 * cm, 100 * cm);
    auto* world_lv = new G4LogicalVolume(world_s, air, "world");
    auto* world_pv = new G4PVPlacement(G4Transform3D{},
                                       world_lv,
                                       "world_pv",
                                       /* parent = */ nullptr,
                                       /* many = */ false,
                                       /* copy_no = */ 0);

    auto* container_s = new G4Box("phantom_container",
                                  num_voxels[0] * half_voxel_dim.x(),
                                  num_voxels[1] * half_voxel_dim.y(),
                                  num_voxels[2] * half_voxel_dim.z());
    auto* container_lv
        = new G4LogicalVolume(container_s, air, "phantom_container");

    G4ThreeVector container_pos(0., 0., 0.);
    new G4PVPlacement(/* rotation = */ nullptr,
                      container_pos,
                      container_lv,
                      "phantom_container",
                      /* parent = */ world_lv,
                      /* many = */ false,
                      /* copy_no = */ 1);

    // Y-axis replica
    auto* rep_y_s = new G4Box("rep_y",
                              num_voxels[0] * half_voxel_dim.x(),
                              half_voxel_dim.y(),
                              num_voxels[2] * half_voxel_dim.z());
    auto* rep_y_lv = new G4LogicalVolume(rep_y_s, air, "rep_y");
    new G4PVReplica("rep_y",
                    rep_y_lv,
                    container_lv,
                    kYAxis,
                    num_voxels[1],
                    half_voxel_dim.y() * 2.);

    // X-axis replica
    auto* rep_x_s = new G4Box("rep_x",
                              half_voxel_dim.x(),
                              half_voxel_dim.y(),
                              num_voxels[2] * half_voxel_dim.z());
    auto* rep_x_lv = new G4LogicalVolume(rep_x_s, air, "rep_x");
    new G4PVReplica("rep_x",
                    rep_x_lv,
                    rep_y_lv,
                    kXAxis,
                    num_voxels[0],
                    half_voxel_dim.x() * 2);

    // Create individual voxel, parameterized along z axis
    auto* voxel_s = new G4Box(
        "voxel", half_voxel_dim.x(), half_voxel_dim.y(), half_voxel_dim.z());
    auto* voxel_lv = new G4LogicalVolume(voxel_s, air, "voxel");
    auto* param = new VoxelParameterisation(
        half_voxel_dim, materials, num_voxels, std::move(material_indices));
    auto* voxel_pv = new G4PVParameterised(
        "voxel", voxel_lv, rep_x_lv, kZAxis, num_voxels[2], param);

    return world_pv;
}

TEST_F(NestedReplicaParametrizationTest, default_options)
{
    auto result = this->run(Options{});
    result.print_ref();

    TestResult ref;
    ref.lv_name = {
        "world",
        "phantom_container",
        "rep_y",
        "rep_x",
        "voxel",
    };
    ref.solid_capacity = {
        8000000000,
        30000,
        10000,
        5000,
        1000,
    };
    ref.pv_name = {
        "voxel",
        "voxel",
        "voxel",
        "voxel",
        "voxel",
        "rep_x",
        "rep_x",
        "rep_y",
        "rep_y",
        "rep_y",
        "phantom_container",
        "world_pv",
    };
    ref.copy_no = {
        0,
        1,
        2,
        3,
        4,
        0,
        1,
        0,
        1,
        2,
        1,
        0,
    };

    result.expect_eq(ref);
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
