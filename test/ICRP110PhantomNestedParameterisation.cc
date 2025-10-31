#include "ICRP110PhantomNestedParameterisation.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"

VoxelParameterisation::VoxelParameterisation(
    G4ThreeVector const& half_voxel_size,
    std::vector<G4Material*> materials,
    std::array<G4int, 3> num_voxels,
    std::vector<std::size_t> material_indices)
    : half_dim_(half_voxel_size)
    , num_voxels_(num_voxels)
    , materials_(std::move(materials))
    , material_indices_(std::move(material_indices))
{
}

G4Material*
VoxelParameterisation::ComputeMaterial(G4VPhysicalVolume* physVol,
                                       G4int const iz,
                                       G4VTouchable const* parentTouch)
{
    if (parentTouch == nullptr)
        return materials_[0];

    G4int ix = parentTouch->GetReplicaNumber(0);
    G4int iy = parentTouch->GetReplicaNumber(1);

    G4int copyID = ix + num_voxels_[0] * iy
                   + num_voxels_[0] * num_voxels_[1] * iz;
    std::size_t matIndex = material_indices_[copyID];
    G4Material* mate = materials_[matIndex];

    physVol->GetLogicalVolume()->SetMaterial(mate);

    return mate;
}

G4int VoxelParameterisation::GetNumberOfMaterials() const
{
    return materials_.size();
}

G4Material* VoxelParameterisation::GetMaterial(G4int i) const
{
    return materials_[i];
}

void VoxelParameterisation::ComputeTransformation(
    G4int const copyNo, G4VPhysicalVolume* physVol) const
{
    G4double z_offset = (2. * static_cast<double>(copyNo) + 1.) * half_dim_.z()
                        - half_dim_.z() * num_voxels_[2];
    physVol->SetTranslation(G4ThreeVector(0., 0., z_offset));
}

void VoxelParameterisation::ComputeDimensions(G4Box& box,
                                              G4int const,
                                              G4VPhysicalVolume const*) const
{
    box.SetXHalfLength(half_dim_.x());
    box.SetYHalfLength(half_dim_.y());
    box.SetZHalfLength(half_dim_.z());
}
