#ifndef ICRP110PhantomNestedParameterisation_HH
#define ICRP110PhantomNestedParameterisation_HH

#include <map>
#include <vector>

#include "G4ThreeVector.hh"
#include "G4Types.hh"
#include "G4VNestedParameterisation.hh"
#include "G4VTouchable.hh"

class G4VPhysicalVolume;
class G4VSolid;
class G4Material;
class G4VisAttributes;

class G4Box;
class G4Tubs;
class G4Trd;
class G4Trap;
class G4Cons;
class G4Sphere;
class G4Ellipsoid;
class G4Orb;
class G4Torus;
class G4Para;
class G4Polycone;
class G4Polyhedra;
class G4Hype;

class ICRP110PhantomNestedParameterisation : public G4VNestedParameterisation
{
  public:
    explicit ICRP110PhantomNestedParameterisation(G4ThreeVector const& voxelSize,
                                                  std::vector<G4Material*>& mat,
                                                  G4int fnX_ = 0,
                                                  G4int fnY_ = 0,
                                                  G4int fnZ_ = 0);
    // the total number of voxels along X, Y and Z
    // are initialised to zero

    ~ICRP110PhantomNestedParameterisation();

    virtual G4Material* ComputeMaterial(G4VPhysicalVolume* currentVol,
                                        G4int const repNo,
                                        G4VTouchable const* parentTouch);
    G4int GetNumberOfMaterials() const;
    G4Material* GetMaterial(G4int idx) const;

    G4int GetMaterialIndex(G4int copyNo) const;

    void SetMaterialIndices(size_t* matInd) { fMaterialIndices = matInd; }
    // This method passes the information of the matID associated to each voxel
    // from the DetectorConstruction to the NestedParameterisation class

    void SetNoVoxel(G4int nx, G4int ny, G4int nz);
    // This method passes the total number of voxels along X, Y and Z from
    // the DetectorConstruction to the NestedParameterisation class

    void
    ComputeTransformation(G4int const no, G4VPhysicalVolume* currentPV) const;

    // Additional standard Parameterisation methods,
    // which can be optionally defined, in case solid is used.
    void ComputeDimensions(G4Box&, G4int const, G4VPhysicalVolume const*) const;
    using G4VNestedParameterisation::ComputeDimensions;
    using G4VNestedParameterisation::ComputeMaterial;

  private:
    G4double fdX, fdY, fdZ;  // Half of the voxels along X, Y and Z
    G4int fnX, fnY, fnZ;  // Number of voxels along X, Y and Z
    std::vector<G4Material*> fMaterials;  // Vector with materials
    size_t* fMaterialIndices;  // Index of the material associated to each
                               // voxel
};
#endif
