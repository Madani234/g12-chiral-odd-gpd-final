# 01 — TCSGen Monte Carlo generator

Event generator for the exclusive reaction

```
γ p → γ ρ⁰ p → γ π⁺ π⁻ p
```

weighted by M. Saad's theoretical cross-section tables. It runs **locally at CEA** and produces
the ROOT file `MonteCarlo_Two_To_Three.root`, which is then transferred to ifarm to be converted
to BOS by BOSwrite (see `02_bos_conversion_BOSwrite/`).

## 1. Origin of the code

The generator is a deep adaptation of **TCSGen** (original author: R. Paremuzyan "rafopar",
2020), a Timelike Compton Scattering generator for CLAS. The 2→3-body kinematics, the weighting by
the theoretical tables and the double-tree output (see §4) are specific to this project.

The main program is `src/MonteCarlo_GammaRho_Generator.cpp`. It was renamed for this repository:
its header still says `Test1_v3_BOS.cpp`, and on the CEA machine the file was called `Test1.cpp`.

## 2. Compilation and execution

Compilation with the Makefile (requires ROOT — version 6.36.04 used at CEA):

```bash
make            # builds the libs (TTCSKine, KinFuncs, TTCSCrs, GPDs) then the binary
./Test1.exe
```

The Makefile first builds a shared library `lib/libTest1.so` from `src/TTCSKine.cc`,
`src/KinFunctions.cc`, `src/TTCSCrs.cc`, `src/GPDs.cc` (headers in `include/`), then links the main
program against it and against ROOT (`root-config --cflags --libs`).

The program opens two files **in the directory it is launched from**: `Input.dat` and
`table_clean.dat`. In this repository the table is stored in `input/cross_sections/`, so copy
(or link) it next to the binary before running:

```bash
ln -s input/cross_sections/table_clean.dat .
```

### Configuration: `Input.dat`

The program reads its parameters from the text file `Input.dat` (key/value pairs), placed in the
launch directory. Keys: `Nsim` (number of events), `Eb` (electron beam energy), `EgMin`/`EgMax`
(photon energy bounds), `Q2Cut`, `tLim`, `Seed` (random seed), `vzMin`/`vzMax` (vertex), `LUND`
(LUND output on/off — obsolete in the final version), `Mn`, `M1`/`M2` (masses of the two produced
particles: ρ⁰ and photon), `psfM1`/`psfM2`, `Massefille1`…`Massefille4` (daughter masses),
`DecayP1`/`DecayP2` (which particle is decayed). The minimum invariant mass is computed as
`M1 + M2`; if it cannot be reached with `EgMin`, the program automatically adjusts `EgMin` and
prints it.

## 3. Physics and weighting

The program reads the parameters from `Input.dat`, then generates `Nsim` events of the reaction.

For each event, it randomly draws the photon energy, the transfer t and the angles, then builds
the four-vectors through Lorentz transformations. The ρ⁰ is then decayed into π⁺π⁻ in its rest
frame, before the pions are brought back into the laboratory frame.

The events are generated according to phase space and then weighted to reproduce the cross
section. The program computes −u′, looks up the corresponding cross section in Saad's table and
builds the weight from sigma, R_t and the phase-space factor (psf). Events outside the domain of
validity are rejected.

The different components of the weight are kept in separate branches, so that the weighting can
be changed at analysis time without regenerating the events.

Each accepted event is then stored in the `tr1` and `mytree` trees. The first contains the full
Monte Carlo information, while the second is adapted to the format expected by BOSwrite for the
detector simulation. Control histograms are also saved.

### Open theoretical point

The cross-section table assumes a **well-defined polarisation**. The **L+T mixture is not
handled**; Saad offered to send the table for the other polarisation.

## 4. Output: `MonteCarlo_Two_To_Three.root`

The generator produces a single file, `MonteCarlo_Two_To_Three.root`, containing two trees and
several control histograms.

The `tr1` tree contains the full information of each event: kinematics, Mandelstam variables,
−u′, −t′, angles and weight components.

The `mytree` tree contains only the information needed by BOSwrite: beam, particles and vertex.
Its structure must stay compliant with the format expected by BOSwrite (conversion to BOS).

Both trees correspond to the same events, filled in the same order: entry k of `tr1` is entry k of
`mytree`. `mytree` is used in the simulation chain on ifarm, while `tr1` stays in the ROOT file and
holds the Monte Carlo truth and the weights. Note that `ievent` is the generator loop index (it
skips rejected draws): BOSwrite writes the entry index k, not `ievent`, in `HEAD.nevent`. The
matching after reconstruction must therefore use k (see the open points in the main README).

The file uses ZLIB compression so as to stay compatible between the ROOT versions used at CEA and
on ifarm. Finally, control histograms make it possible to check the distributions of momenta,
angles and the main kinematic correlations.

**`tr1` — analysis tree** (full kinematics + weight):

| Branch(es) | Type | Content |
|---|---|---|
| `L_fille1`, `L_fille2`, `L_fille3`, `L_fille4` | TLorentzVector | four-vectors of the daughter particles |
| `L_P1`, `L_P2` | TLorentzVector | ρ⁰ and outgoing photon |
| `L_prot`, `L_miss` | TLorentzVector | recoil proton, missing four-vector |
| `Eg`, `Q2`, `t`, `u`, `s` | double | kinematic / Mandelstam variables |
| `u_prime`, `t_prime` | double | −u′ and −t′ (factorisation variables) |
| `cos_theta_H`, `cos_theta_H2` | double | helicity angle(s) |
| `sigma`, `psf`, `jac`, `R_t`, `flux_factor` | double | weight components |
| `Masse1_evt`, `Masse2_evt` | double | masses of the event |
| `vz` | double | vertex |
| `ievent` | int | generator loop index (≠ entry index k written by BOSwrite in `HEAD.nevent`) |

## 5. Folder contents

```
01_generator_TCSGen/
├── README.md                ← this file
├── Makefile
├── Input.dat                ← configuration actually read by the program
├── src/
│   ├── MonteCarlo_GammaRho_Generator.cpp   ← main program (writes tr1 + mytree)
│   └── TTCSKine.cc, KinFunctions.cc, TTCSCrs.cc, GPDs.cc
├── include/                 ← headers of the four libraries above
├── input/cross_sections/
│   └── table_clean.dat      ← Saad's table {SgN, M², −u′, σ+, σ−, σx, σy}
│                               (SgN ∈ [6, 20] in steps of 0.5)
```
