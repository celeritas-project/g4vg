//------------------------------- -*- C++ -*- -------------------------------//
// Copyright G4VG contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file TestBase.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <vector>
#include <gtest/gtest.h>

class G4VPhysicalVolume;

namespace g4vg
{
struct Options;
namespace test
{
//---------------------------------------------------------------------------//
struct TestResult
{
    //! Name of the Geant4 volume corresponding to the VecGeom LV
    std::vector<std::string> lv_name;
    std::vector<double> solid_capacity;
    std::vector<std::string> pv_name;
    std::vector<int> copy_no;
    std::vector<std::string> nested;

    void print_ref() const;
    void expect_eq(TestResult const& reference) const;
};

//---------------------------------------------------------------------------//
class TestBase : public ::testing::Test
{
  protected:
    void SetUp() override;
    void TearDown() override;

    virtual std::string basename() const = 0;
    virtual G4VPhysicalVolume* build_world() = 0;

    G4VPhysicalVolume const* g4world() const { return world_; }

    TestResult run(Options const& options)
    {
        TestResult result;
        this->run_impl(options, result);
        return result;
    }

    std::string logical_volume_name(unsigned int lv_id) const;

  private:
    G4VPhysicalVolume* world_{nullptr};

    void run_impl(Options const& options, TestResult& result);
};

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace g4vg
