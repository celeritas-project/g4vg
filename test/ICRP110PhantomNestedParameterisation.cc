#include "ICRP110PhantomNestedParameterisation.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"

VoxelParameterisation::VoxelParameterisation(G4ThreeVector const& halfVoxelSize,
                                             std::vector<G4Material*>& mat,
                                             G4int fnX_,
                                             G4int fnY_,
                                             G4int fnZ_)
    : fdX(halfVoxelSize.x())
    , fdY(halfVoxelSize.y())
    , fdZ(halfVoxelSize.z())
    , fnX(fnX_)
    , fnY(fnY_)
    , fnZ(fnZ_)
    ,  // number of voxels along X, Y and Z
    fMaterials(mat)
    ,  // vector of defined materials
    fMaterialIndices(nullptr)  // vector which associates MaterialID to voxels
{
}

VoxelParameterisation::~VoxelParameterisation() {}

void VoxelParameterisation::SetNoVoxel(G4int nx, G4int ny, G4int nz)
{
    fnX = nx;
    fnY = ny;
    fnZ = nz;
}

G4Material*
VoxelParameterisation::ComputeMaterial(G4VPhysicalVolume* physVol,
                                       G4int const iz,
                                       G4VTouchable const* parentTouch)
{
    if (parentTouch == nullptr)
        return fMaterials[0];

    G4int ix = parentTouch->GetReplicaNumber(0);
    G4int iy = parentTouch->GetReplicaNumber(1);

    G4int copyID = ix + fnX * iy + fnX * fnY * iz;
    std::size_t matIndex = GetMaterialIndex(copyID);
    G4Material* mate = fMaterials[matIndex];

    if (true && physVol && G4VVisManager::GetConcreteInstance())
    {
        G4String mateName = fMaterials.at(matIndex)->GetName();
        std::string::size_type iuu = mateName.find("__");

        if (iuu != std::string::npos)
        {
            mateName = mateName.substr(0, iuu);  // Associate material
        }
    }
    physVol->GetLogicalVolume()->SetMaterial(mate);

    return mate;
}

G4int VoxelParameterisation::GetMaterialIndex(G4int copyNo) const
{
    return fMaterialIndices[copyNo];
}

G4int VoxelParameterisation::GetNumberOfMaterials() const
{
    return fMaterials.size();
}

G4Material* VoxelParameterisation::GetMaterial(G4int i) const
{
    return fMaterials[i];
}

void VoxelParameterisation::ComputeTransformation(
    G4int const copyNo, G4VPhysicalVolume* physVol) const
{
    physVol->SetTranslation(G4ThreeVector(
        0., 0., (2. * static_cast<double>(copyNo) + 1.) * fdZ - fdZ * fnZ));
}

void VoxelParameterisation::ComputeDimensions(G4Box& box,
                                              G4int const,
                                              G4VPhysicalVolume const*) const
{
    box.SetXHalfLength(fdX);
    box.SetYHalfLength(fdY);
    box.SetZHalfLength(fdZ);
}
