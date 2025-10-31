//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Custom.test.cc
//---------------------------------------------------------------------------//

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
#include <G4TransportationManager.hh>
#include <gtest/gtest.h>

#include "G4VG.hh"
#include "ICRP110PhantomNestedParameterisation.hh"
#include "TestBase.hh"

namespace g4vg
{
namespace test
{
//---------------------------------------------------------------------------//
using CustomTestBase = TestBase;

//---------------------------------------------------------------------------//
class NestedReplicaParametrization : public CustomTestBase
{
  protected:
    std::string basename() const final { return "voxel"; }
    G4VPhysicalVolume* build_world() final;
};

void PlacePhantomInVolume(G4LogicalVolume* logicVolume)
{
    auto* nist = G4NistManager::Instance();
    G4Material* matAir
        = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    auto elH = std::make_unique<G4Element>(
        "Hydrogen", "H", /* Z = */ 1., /* A = */ 1.01 * g / mole);

    std::vector<G4Material*> materials;
    materials.push_back(matAir);  // 0
    for (int i = 1; i < 8; ++i)
    {
        auto mat = std::make_unique<G4Material>(
            "h" + std::to_string(i * 10), i * 0.1 * g / cm3, 1, kStateGas);
        mat->AddElement(elH.get(), 1);
        materials.push_back(mat.release());
    }

    G4int fNVoxelX = 2, fNVoxelY = 3, fNVoxelZ = 5;
    G4int fNVoxels = fNVoxelX * fNVoxelY * fNVoxelZ;
    size_t* fMateIDs = new size_t[fNVoxels];

    // Assign material IDs to each voxel here. For simplicity, we assign them
    // in a round-robin fashion.
    for (G4int iz = 0; iz < fNVoxelZ; ++iz)
    {
        for (G4int iy = 0; iy < fNVoxelY; ++iy)
        {
            for (G4int ix = 0; ix < fNVoxelX; ++ix)
            {
                G4int index = ix + fNVoxelX * (iy + fNVoxelY * iz);
                fMateIDs[index] = index % materials.size();
            }
        }
    }

    G4double fVoxelHalfDimZ = 0.5 * cm, fVoxelHalfDimY = 0.5 * cm,
             fVoxelHalfDimX = 0.5 * cm;
    G4Box* fContainer_solid = new G4Box("phantomContainer",
                                        fNVoxelX * fVoxelHalfDimX * mm,
                                        fNVoxelY * fVoxelHalfDimY * mm,
                                        fNVoxelZ * fVoxelHalfDimZ * mm);

    auto fContainer_logic = new G4LogicalVolume(
        fContainer_solid, matAir, "phantomContainer", nullptr, nullptr, nullptr);

    auto fMaxX = fNVoxelX * fVoxelHalfDimX * mm;  // Max X along X axis of the
                                                  // voxelised geometry
    auto fMaxY = fNVoxelY * fVoxelHalfDimY * mm;  // Max Y
    auto fMaxZ = fNVoxelZ * fVoxelHalfDimZ * mm;  // Max Z

    auto fMinX = -fNVoxelX * fVoxelHalfDimX * mm;  // Min X
    auto fMinY = -fNVoxelY * fVoxelHalfDimY * mm;  // Min Y
    auto fMinZ = -fNVoxelZ * fVoxelHalfDimZ * mm;  // Min Z

    // G4ThreeVector posCentreVoxels(0*cm,0*cm,-35*cm);
    G4ThreeVector posCentreVoxels(
        (fMinX + fMaxX) / 2., (fMinY + fMaxY) / 2., (fMinZ + fMaxZ) / 2.);

    new G4PVPlacement(nullptr,  // rotation
                      posCentreVoxels,
                      fContainer_logic,  // The logic volume
                      "phantomContainer",  // Name
                      logicVolume,  // Mother
                      false,  // No op. bool.
                      1);  // Copy number

    G4String yRepName("RepY");
    G4VSolid* solYRep = new G4Box(yRepName,
                                  fNVoxelX * fVoxelHalfDimX,
                                  fVoxelHalfDimY,
                                  fNVoxelZ * fVoxelHalfDimZ);
    auto logYRep = new G4LogicalVolume(solYRep, matAir, yRepName);
    new G4PVReplica(yRepName,
                    logYRep,
                    fContainer_logic,
                    kYAxis,
                    fNVoxelY,
                    fVoxelHalfDimY * 2.);

    //--- Slice the phantom along X axis
    G4String xRepName("RepX");
    G4VSolid* solXRep = new G4Box(
        xRepName, fVoxelHalfDimX, fVoxelHalfDimY, fNVoxelZ * fVoxelHalfDimZ);
    auto logXRep = new G4LogicalVolume(solXRep, matAir, xRepName);
    new G4PVReplica(
        xRepName, logXRep, logYRep, kXAxis, fNVoxelX, fVoxelHalfDimX * 2.);

    //----- Voxel solid and logical volumes
    //--- Slice along Z axis
    G4VSolid* solidVoxel
        = new G4Box("phantom", fVoxelHalfDimX, fVoxelHalfDimY, fVoxelHalfDimZ);
    auto logicVoxel = new G4LogicalVolume(solidVoxel, matAir, "phantom");

    // Parameterisation to define the material of each voxel
    G4ThreeVector halfVoxelSize(fVoxelHalfDimX, fVoxelHalfDimY, fVoxelHalfDimZ);

    auto param
        = new ICRP110PhantomNestedParameterisation(halfVoxelSize, materials);

    new G4PVParameterised("phantom",  // their name
                          logicVoxel,  // their logical volume
                          logXRep,  // Mother logical volume
                          kZAxis,  // Are placed along this axis
                          fNVoxelZ,  // Number of cells
                          param);  // Parameterisation

    param->SetMaterialIndices(fMateIDs);  // fMateIDs is  the vector with
                                          // Material ID associated to each
                                          // voxel, from ASCII input data
                                          // files.
    param->SetNoVoxel(fNVoxelX, fNVoxelY, fNVoxelZ);
}

G4VPhysicalVolume* NestedReplicaParametrization::build_world()
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

    PlacePhantomInVolume(world_l);

    return world_p;
}
TEST_F(NestedReplicaParametrization, default_options)
{
    auto result = this->run(Options{});

    TestResult ref;
    ref.lv_name = {
        "world",
        "baseVoxel",
    };
    ref.solid_capacity = {(200 * 200 * 200) * cm3, 1 * cm3};
    ref.pv_name = {
        "baseVoxel_pv",
        "world_pv",
    };
    ref.copy_no = {0, 0};
    ref.print_ref();

#if 0
    G4VPhysicalVolume* pVol =
    G4TransportationManager::GetTransportationManager()->GetNavigator(const_cast<G4VPhysicalVolume*>(g4world()))
                               ->LocateGlobalPointAndSetup(G4ThreeVector(0,0,0));
    G4Material* pMat = pVol->GetLogicalVolume()->GetMaterial();
    EXPECT_EQ(pMat->GetName(), "G4_WATER");
#endif

    result.expect_eq(ref);
}
//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
