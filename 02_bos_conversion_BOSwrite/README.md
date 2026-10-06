# BOSwrite — from ROOT to BOS

This is the bridge between the generator and the detector simulation. The generator produces a
ROOT file; `gsim`, which simulates the passage of particles through CLAS, is a software package
from the 2000s that can only read BOS, the native CLAS6 format. BOSwrite performs the
translation: it reads the `mytree` tree from the generated file and rewrites it in BOS format,
ready to be injected into the reconstruction chain.

## The two files, and what was modified

The folder contains two versions of the code, each corresponding to a different channel.

| File | Role |
|---|---|
| `BOSwriteBackup.cc` | the original version, written for a channel other than ours |
| `BOSwrite_v1.1.cc` | the version adapted to our channel `γ p → γ ρ⁰ p → γ π⁺ π⁻ p` — this is the one that was used |
| `README_original.txt` | the README delivered with the original code by its author |

The adaptation was done on both sides at once, and this is the important point to understand
if you take over this work.

- generator side: the generator was modified to produce a ROOT file in the exact format
  that BOSwrite can read (see `01_generator_TCSGen`);
- BOSwrite side: the code was modified to match our channel, since the original version
  handled a different reaction.

In other words, the two ends were brought into agreement. If you change the channel,
this work will have to be redone on both sides — modifying only one of them is not enough.

## What BOSwrite expects as input

The ROOT file must contain a tree named exactly `mytree`, with these branches and
no other names.

| Branch | Type |
|---|---|
| `beam` | TLorentzVector |
| `Qp1`, `Qp2` | TLorentzVector |
| `Sp1`, `Sp2` | TLorentzVector |
| `Rp1`, `Rp2` | TLorentzVector |
| `Vertex1`, `Vertex2` | TVector3 |

The branches are declared in pointer style, with a split level of 0.

### Two constraints that are not immediately obvious

**The ROOT file compression.** The generator runs under ROOT 6 at the CEA, BOSwrite under
ROOT 5.34 in the container. The file must be written using ZLIB compression
(`SetCompressionSettings(101)` in the generator), otherwise it cannot be read here.

**The tree name.** `mytree` is not configurable on the BOSwrite side: it is hard-coded in
the code. If your tree has another name, you need to change the generator, or modify the `.cc`.

## Compilation and execution

Complete sequence, verified to work on ifarm, from the BOSwrite directory:

```bash
cd /work/clas/clasg12/madani/BOSwrite
./StartApptainer.sh                     # enters the container
source SourceAfterApptainer.sh          # CRITICAL STEP — loads ROOT and the BOS libs
export CLAS_CALDB_RUNINDEX=calib_user.RunIndexg12
unset SCONSFLAGS                        # must be done EVERY session
rm -f build/obj/BOSwrite_v1.1.o build/bin/BOSwrite_v1.1
scons -j8
./build/bin/BOSwrite_v1.1 /work/clas/clasg12/madani/gen/MonteCarlo_Two_To_Three.root -R1
```

The generator ROOT file must first be transferred from the CEA to ifarm using `scp`.


## ⚠️ What is missing from this folder

The files below are necessary for compilation, but could not be retrieved before access to ifarm
was lost. They are all in the original folder on the farm.

```
/work/clas/clasg12/madani/BOSwrite
```

| Missing file | What it is for |
|---|---|
| `sconstruct` | the SCons build script — this is what `scons` reads |
| `build_config` | the associated compilation configuration |
| `StartApptainer.sh` | starts the Apptainer container |
| `SourceAfterApptainer.sh` | loads ROOT and the BOS libraries once inside |
| `recsis/` | folder delivered with the original package; its exact contents were not inventoried — check whether it contains configuration files read at runtime |
| `build/` | build tree (recreated by SCons, no need to retrieve it) |

In practice, the `.cc` file alone cannot be compiled. Anyone taking over this work with JLab
access should first retrieve the complete folder above, then replace the `.cc` with the one from
this repository.
