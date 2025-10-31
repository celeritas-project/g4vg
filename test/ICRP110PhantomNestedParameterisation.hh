#ifndef ICRP110PhantomNestedParameterisation_HH
#define ICRP110PhantomNestedParameterisation_HH

#include <array>
#include <vector>

#include "G4ThreeVector.hh"
#include "G4Types.hh"
#include "G4VNestedParameterisation.hh"
#include "G4VTouchable.hh"

class G4VPhysicalVolume;
class G4Material;

class VoxelParameterisation final : public G4VNestedParameterisation
{
  public:
    VoxelParameterisation(G4ThreeVector const& half_voxel_size,
                          std::vector<G4Material*> materials,
                          std::array<G4int, 3> num_voxels,
                          std::vector<std::size_t> material_indices);

    ~VoxelParameterisation() override = default;

    G4Material* ComputeMaterial(G4VPhysicalVolume* currentVol,
                                G4int const repNo,
                                G4VTouchable const* parentTouch) override;

    G4int GetNumberOfMaterials() const override;
    G4Material* GetMaterial(G4int idx) const override;

    void ComputeTransformation(G4int const no,
                               G4VPhysicalVolume* currentPV) const override;

    void ComputeDimensions(G4Box&,
                           G4int const,
                           G4VPhysicalVolume const*) const override;

    using G4VNestedParameterisation::ComputeDimensions;
    using G4VNestedParameterisation::ComputeMaterial;

  private:
    G4ThreeVector half_dim_;
    std::array<G4int, 3> num_voxels_;  // {nx, ny, nz}
    std::vector<G4Material*> materials_;
    std::vector<std::size_t> material_indices_;
};

#endif
