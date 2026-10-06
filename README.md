# g12-chiral-odd-gpd

**Exclusive photoproduction `γ p → γ ρ⁰ p → γ π⁺ π⁻ p` on CLAS g12 data from Jefferson Lab,
as a probe of chiral-odd GPDs (transversity GPD H_T).**

This repository gathers the code, configuration files and documentation for the whole chain,
from raw data to final plots, so that someone who has never spoken to the author can pick up the
work where it stops.

---

## If you are taking over this project, read this first

The principle of the work is simple to state: we compare what we measure with what we simulate.

On one side, the **real data** from the CLAS g12 experiment (Jefferson Lab, 2008) are converted
from BOS (the native CLAS6 format) to ROOT, then passed through an analysis pipeline that selects
`γ π⁺ π⁻ p` events and extracts the ρ⁰ signal. On the other side, a **Monte Carlo generator**
produces the same events theoretically; they are sent through a simulation of the detector, then
reconstructed exactly like the real data. If both sides look alike, the chain is understood.

The goal of this internship is **not** to measure the spin density matrix element ρ⁰₀₀, but to
**prepare the way** toward that measurement. Concretely, the validation obtained is a **visual,
side-by-side comparison** of the data/simulation distributions (ρ⁰ mass profile, production
observables).

---

## The work done

The project consists of building two independent branches that end in the same type of plots,
so that they can be compared. Here is what was done on each side.

### Simulation side: producing events and passing them through a virtual detector

**The generator.** A Monte Carlo generator (TCSGen, heavily adapted for this channel) randomly
draws γ p → γ ρ⁰ p events, builds the four-vectors of all particles, and decays the ρ⁰ into
π⁺π⁻. But randomly drawn events do not represent the physics: they have to be weighted. Each
event therefore receives a weight computed from the theoretical cross-section tables provided by
M. Saad, so that the kinematic regions favoured by theory are correctly represented. The result is
written to a ROOT file.

**From ROOT to BOS.** The CLAS detector simulation is software from the 2000s that cannot read
ROOT: it expects the BOS format, native to CLAS6. A conversion code (BOSwrite) therefore had to be
used, retrieved from the JLab computing farm and adapted to our reaction; it reads the generator
file and rewrites it in the expected format. This step required making the structure of the ROOT
tree produced by the generator match exactly what BOSwrite expects to read.

**The simulation and reconstruction chain.** The events then go through three successive
programs, the same ones that were used on the real data at the time of the experiment: gsim
simulates the passage of the particles through the detector, gpp degrades the response to make it
realistic, and a1c reconstructs the tracks — that is, it starts again from the detector signals to
find which particles went through, exactly as is done on real data.

**Getting the right format.** At the output of this chain, the simulation file could not be read
directly by the converter to ROOT, which systematically crashed on it while it processed the real
data files without any problem. Finding the cause required a fairly long diagnostic effort, at the
end of which it turned out that the file produced by the simulation, although valid, did not have
exactly the same shape as the real data files. A small converter written in C (mkdst) put it back
into the right format and unblocked the chain (see `03_clas6_reconstruction/` for the full
diagnosis and the code).

**From BOS to ROOT.** The cleaned file finally goes through Pierre Chatagnon's conversion code
(based on RootBeer), which produces a usable ROOT file.

### Real-data side: fetching the actual measurements

The g12 data are stored on the JLab servers in BOS format: about 60,000 files, some thirty
terabytes. The work consisted of converting them to ROOT with the same code by Pierre, then
gathering them into a single working file. At this scale the conversion cannot be launched in one
go: it had to be split up, the failing jobs monitored, relaunched, and the integrity of each output
checked before merging.

### Convergence: the same analysis on both sides

Both branches then produce the same type of ROOT file, and exactly the same analysis code
processes them: selection of γ π⁺ π⁻ p events by five successive cuts, fit of the ρ⁰ peak on the
invariant-mass spectrum, control plots. Using the same code on both sides is essential: it
guarantees that a difference observed between data and simulation comes from the physics or the
detector, and not from the way the files were processed.

---

## IMPORTANT: if the code does not compile, environment files are missing

This repository contains the code of each step, but not everything around it. Several pieces
essential to compiling and running could not be retrieved before the end of the internship, for
lack of access to the machines: build scripts, container environment files, simulation
configuration cards, CLAS6 libraries.

This is not an omission that can be repaired from the repository: these files are specific to the
JLab computing farm and only make sense there. Code that refuses to compile therefore does not
necessarily signal a bug — most of the time, it signals that it is missing its original
environment.

The procedure is always the same: open an account on ifarm, go back to the original folder of the
step concerned, retrieve its complete directory tree, then replace the code files there with the
ones from this repository. The README of each folder lists precisely what is missing and the exact
path where to find it:

| Step | Original folder on ifarm |
|---|---|
| ROOT → BOS conversion | `/work/clas/clasg12/madani/BOSwrite/` |
| Simulation and reconstruction | `/work/clas/clasg12/madani/gen/` and the container `/group/clas/builds/centos79-clas6.sif` |
| BOS → ROOT conversion | `/work/clas/clasg12/madani/rootbeer6.0/` |

If these folders have been deleted in the meantime, the READMEs give enough information to
rebuild the essentials, and two contacts can help: Pierre Chatagnon (CEA Saclay) for the
conversion and analysis part, Josh Bryce (University of York) for the CLAS6 reconstruction recipe
and the simulation cards.

## The repository at a glance

The folders are numbered in the order in which the data flow, not by language: going through them
from 01 to 05, you follow the path of an event from the generator to the final plot.

```
g12-chiral-odd-gpd/
├── README.md                        ← this file
│
├── 01_generator_TCSGen/             ← the Monte Carlo generator (runs at CEA)
├── 02_bos_conversion_BOSwrite/      ← ROOT → BOS, to inject into the simulation (ifarm)
├── 03_clas6_reconstruction/         ← gsim → gpp → a1c → mkdst repack (ifarm)
├── 04_bos_to_root_RootBeer/         ← BOS → ROOT (Pierre's conversion code)
└── 05_analysis/                     ← the RDataFrame analysis pipeline
```

Each folder has its own README explaining its role, how to run it, and what can go wrong.

---

## What is not in the repository

The data are not versioned — they weigh from a few hundred megabytes to several terabytes, and
GitHub caps files at 100 MB anyway. They live on the machines, and their paths are given in the
README of each step: the raw and cooked BOS files (`cooked_out.bos`, 842 MB), the ROOT files
(`MonteCarlo_Two_To_Three.root`, `g12_Total.root`, the per-batch outputs), and the `build/`
folders.

## Open points for what comes next

* **Polarisation of the cross-section tables**: the table provided by M. Saad assumes a
  well-defined polarisation — it remains to be confirmed whether it is that of the ρ or of the
  photon. Saad offered to send the table for the other polarisation. The **L+T mixture is not
  handled** in the simulation.
* **Computing the acceptance**, then extracting ρ⁰₀₀: the natural next step, now possible since
  the simulation branch goes all the way to ROOT. The MC truth (and the event weight) must be
  taken from the generator ROOT file — never from the DST. This matching is **not possible yet**:
  BOSwrite writes the `mytree` entry index (0, 1, 2…) in `HEAD.nevent`, not `ievent`, and
  `rbtest.C` stores no event number in the tree `T`. Until `HEAD.nevent` is added as a branch in
  `rbtest.C`, the simulation is analysed **unweighted** (pure phase space).

## Context and contacts

* **Internship supervisor**: Pierre Chatagnon (CEA Saclay)
* **Theory collaboration**: S. Wallon and M. Saad (cross-section tables)
* **CLAS6 reconstruction recipe / g12 contact**: Josh Bryce (University of York)
