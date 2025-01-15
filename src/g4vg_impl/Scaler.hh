//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4vg_impl/Scaler.hh
//---------------------------------------------------------------------------//
#pragma once

#include <utility>
#include <G4ThreeVector.hh>
#include <G4TwoVector.hh>

namespace g4vg
{
//---------------------------------------------------------------------------//
/*!
 * Convert a unit from Geant4 scale to another.
 */
class Scaler
{
  public:
    //! Construct with numeric value of 1mm in native system
    explicit Scaler(double one_mm) : scale_{one_mm}
    {
        G4VG_EXPECT(scale_ > 0);
    }

    //! Convert a positional scalar
    double operator()(double val) const { return val * scale_; }

    //! Convert a two-vector
    std::pair<double, double> operator()(G4TwoVector const& vec) const
    {
        return {(*this)(vec.x()), (*this)(vec.y())};
    }

    //! Convert a three-vector
    vecgeom::Vector3D<double> operator()(G4ThreeVector const& vec) const
    {
        return {(*this)(vec.x()), (*this)(vec.y()), (*this)(vec.z())};
    }

  private:
    double scale_;
};

//---------------------------------------------------------------------------//
}  // namespace g4vg
