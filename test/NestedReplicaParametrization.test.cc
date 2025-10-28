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
#include <G4SystemOfUnits.hh>
#include <G4Box.hh>
#include <G4TransportationManager.hh>
#include <G4PVReplica.hh>
#include <G4PVParameterised.hh>
#include "ICRP110PhantomNestedParameterisation.hh"

#include "G4VG.hh"
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

std::vector<G4Material*> GetMaterialICRU()
{   
    auto * nist = G4NistManager::Instance();
    G4double A = 1.01*g/mole;
    G4double Z;
    auto elH = new G4Element ("Hydrogen","H",Z = 1.,A);

    std::vector<G4Material*> pMaterials;
    pMaterials.push_back(nist->FindOrBuildMaterial("G4_AIR"));//0
    auto * h10 =  new G4Material("h10", 0.1*g/cm3, 1,kStateGas);
    h10->AddElement(elH,1);
    pMaterials.push_back(h10);//1
    auto * h20 =  new G4Material("h20", 0.2*g/cm3, 1,kStateGas);
    h20->AddElement(elH,1);
    pMaterials.push_back(h20);//2
    auto * h30 =  new G4Material("h30", 0.3*g/cm3, 1,kStateGas);
    h30->AddElement(elH,1);
    pMaterials.push_back(h30);//3
    auto * h40 =  new G4Material("h40", 0.4*g/cm3, 1,kStateGas);
    h40->AddElement(elH,1);
    pMaterials.push_back(h40);//4     
    auto * h50 =  new G4Material("h50", 0.5*g/cm3, 1,kStateGas);
    h50->AddElement(elH,1);
    pMaterials.push_back(h50);//5
    auto * h60 =  new G4Material("h60", 0.6*g/cm3, 1,kStateGas);
    h60->AddElement(elH,1);
    pMaterials.push_back(h60);//6
    auto * h70 =  new G4Material("h70", 0.7*g/cm3, 1,kStateGas);
    h70->AddElement(elH,1);
    pMaterials.push_back(h70);//7   
    return pMaterials;
}
void PlacePhantomInVolume(G4LogicalVolume* logicVolume,std::vector<G4Material*> pMaterials)
{
    G4Material* matAir = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

    G4int fNVoxelX = 2, fNVoxelY = 3, fNVoxelZ = 5;
    G4int fNVoxels = fNVoxelX * fNVoxelY * fNVoxelZ;
    size_t* fMateIDs = new size_t[fNVoxels];

    // Assign material IDs to each voxel here. For simplicity, we assign them in a round-robin fashion.
    for (G4int iz = 0; iz < fNVoxelZ; ++iz) {
        for (G4int iy = 0; iy < fNVoxelY; ++iy) {
            for (G4int ix = 0; ix < fNVoxelX; ++ix) {
                G4int index = ix + fNVoxelX * (iy + fNVoxelY * iz);
                fMateIDs[index] = index % pMaterials.size();
            }
        }
    }


    G4double fVoxelHalfDimZ=0.5*cm,fVoxelHalfDimY=0.5*cm,fVoxelHalfDimX = 0.5*cm;
    G4Box* fContainer_solid = new G4Box("phantomContainer",fNVoxelX*fVoxelHalfDimX*mm,
                               fNVoxelY*fVoxelHalfDimY*mm,
                               fNVoxelZ*fVoxelHalfDimZ*mm);

  auto fContainer_logic = new G4LogicalVolume( fContainer_solid,
                                                            matAir,
                                                            "phantomContainer",
                                                             nullptr, nullptr, nullptr);

    auto fMaxX = fNVoxelX*fVoxelHalfDimX*mm; // Max X along X axis of the voxelised geometry
    auto fMaxY = fNVoxelY*fVoxelHalfDimY*mm; // Max Y
    auto fMaxZ = fNVoxelZ*fVoxelHalfDimZ*mm; // Max Z

    auto fMinX = -fNVoxelX*fVoxelHalfDimX*mm;// Min X
    auto fMinY = -fNVoxelY*fVoxelHalfDimY*mm;// Min Y
    auto fMinZ = -fNVoxelZ*fVoxelHalfDimZ*mm;// Min Z

    //G4ThreeVector posCentreVoxels(0*cm,0*cm,-35*cm);
    G4ThreeVector posCentreVoxels((fMinX+fMaxX)/2.,(fMinY+fMaxY)/2.,(fMinZ+fMaxZ)/2.);

    //auto*  fPhantomContainer =
   new G4PVPlacement(nullptr,                     // rotation
                      posCentreVoxels,
                      fContainer_logic,     // The logic volume
                      "phantomContainer",  // Name
                      logicVolume,         // Mother
                      false,            // No op. bool.
                      1);              // Copy number

G4String yRepName("RepY");
   G4VSolid* solYRep = new G4Box(yRepName,fNVoxelX*fVoxelHalfDimX,
                                  fVoxelHalfDimY, fNVoxelZ*fVoxelHalfDimZ);
   auto logYRep = new G4LogicalVolume(solYRep,matAir,yRepName);
   new G4PVReplica(yRepName,logYRep,fContainer_logic,kYAxis, fNVoxelY,fVoxelHalfDimY*2.);


//--- Slice the phantom along X axis
   G4String xRepName("RepX");
   G4VSolid* solXRep = new G4Box(xRepName,fVoxelHalfDimX,fVoxelHalfDimY,
                                  fNVoxelZ*fVoxelHalfDimZ);
   auto logXRep = new G4LogicalVolume(solXRep,matAir,xRepName);
   new G4PVReplica(xRepName,logXRep,logYRep,kXAxis,fNVoxelX,fVoxelHalfDimX*2.);


   //----- Voxel solid and logical volumes
   //--- Slice along Z axis
   G4VSolid* solidVoxel = new G4Box("phantom",fVoxelHalfDimX, fVoxelHalfDimY,fVoxelHalfDimZ);
   auto logicVoxel = new G4LogicalVolume(solidVoxel,matAir,"phantom");


   // Parameterisation to define the material of each voxel
   G4ThreeVector halfVoxelSize(fVoxelHalfDimX,fVoxelHalfDimY,fVoxelHalfDimZ);

   auto param =  new ICRP110PhantomNestedParameterisation(halfVoxelSize, pMaterials);

   new G4PVParameterised("phantom",    // their name
                          logicVoxel, // their logical volume
                          logXRep,      // Mother logical volume
                          kZAxis,       // Are placed along this axis
                          fNVoxelZ,      // Number of cells
                          param);       // Parameterisation

    param -> SetMaterialIndices(fMateIDs); // fMateIDs is  the vector with Material ID associated to each voxel, from ASCII input data files.
    param -> SetNoVoxel(fNVoxelX,fNVoxelY,fNVoxelZ);
}

G4VPhysicalVolume* NestedReplicaParametrization::build_world()
{
    G4Material* mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

    auto* world_s = new G4Box("world_solid", 100*cm, 100*cm, 100*cm);
    auto* world_l = new G4LogicalVolume(world_s, mat, "world");
    auto* world_p = new G4PVPlacement(G4Transform3D{},
                                      world_l,
                                      "world_pv",
                                      /* parent = */ nullptr,
                                      /* many = */ false,
                                      /* copy_no = */ 0);
    

    //PlacePhantomInVolume(world_l, GetMaterialNIST());
    PlacePhantomInVolume(world_l, GetMaterialICRU());
                           

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
    ref.solid_capacity = {
        (200*200*200)*cm3,
        1*cm3
    };
    ref.pv_name = {
        "baseVoxel_pv",
        "world_pv",

    };
    ref.copy_no = {
        0,
        0
    };
    
    // Boolean volume calculation differs across platforms
    EXPECT_TRUE(result.solid_capacity.size() >= 2
                && result.solid_capacity[1] > 0);
    //ref.solid_capacity[2] = 0;
    //result.solid_capacity[2] = 0;


    // G4VPhysicalVolume* pVol = G4TransportationManager::GetTransportationManager()->GetNavigator(const_cast<G4VPhysicalVolume*>(g4world()))
    //                            ->LocateGlobalPointAndSetup(G4ThreeVector(0,0,0));
    // G4Material* pMat = pVol->GetLogicalVolume()->GetMaterial();
    // EXPECT_EQ(pMat->GetName(), "G4_WATER");

    result.expect_eq(ref);
}
//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
