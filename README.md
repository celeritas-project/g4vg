# G4VG

G4VG builds a [VecGeom][vg] geometry representation from a [Geant4][g4] user application using the in-memory Geant4 volume representation. The code is entirely derivative from [Celeritas][cel], written primarily by @sethrj and @mrguilima , and adapted to an independent repository with the help of @drbenmorgan and supervision of @agheata et al. The original code was inspired by similar conversion work done by @sawenzel and @jonapost in [G4VecGeomNav][vgnav].

[vg]: https://gitlab.cern.ch/VecGeom/VecGeom
[g4]: https://www.geant4.org
[cel]: https://github.com/celeritas-project/celeritas
[vgnav]: https://gitlab.cern.ch/VecGeom/g4vecgeomnav/-/blob/master/src/G4VecGeomConverter.cxx

# Installation

G4VG can be installed with [Spack][spack], or using CMake to configure and
build, using ``CMAKE_PREFIX_PATH`` to inform the configuration script of your
existing VecGeom and Geant4 installations.

[spack-start]: https://spack.readthedocs.io/en/latest/getting_started.html

# Development

A [spack environment](./scripts/spack.yaml) can be used to set up your
development toolchain. Please install pre-commit to ensure that code is
formatted properly:
```console
$ pre-commit install
pre-commit installed at .git/hooks/pre-commit
```
