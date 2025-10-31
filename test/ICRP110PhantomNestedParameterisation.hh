#ifndef ICRP110PhantomNestedParameterisation_HH
#define ICRP110PhantomNestedParameterisation_HH

#include <array>
#include <vector>

#include "G4ThreeVector.hh"
#include "G4VNestedParameterisation.hh"
#include "G4VTouchable.hh"

class G4VPhysicalVolume;
class G4Material;

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

    int GetNumberOfMaterials() const override;
    G4Material* GetMaterial(int idx) const override;

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

#endif
