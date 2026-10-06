# 05 — Analysis

This is the end of the chain: the pipeline that takes a reconstructed ROOT file, extracts the
`γ p → γ ρ⁰ p → γ π⁺ π⁻ p` events from it, and produces the final plots. It runs **locally at
CEA** (`dphnpct50`), in Python with ROOT RDataFrame.

One important point to understand right away: **the same code processes the real data and the
simulation**. You give it either the real data or the ROOT file coming out of the simulation chain,
and it applies exactly the same treatment to both. This is what makes the comparison honest — if
the two sides do not look alike at the end, it is not because they were processed differently.

`phase1_pipeline.py` is the version of the analysis retained for the internship report
("version A"), kept exactly as it was used, without any modification. The input file is set at the
top of the script (`FICHIER = "g12_Total.root"`); the tree is detected automatically.

## What the pipeline does

The script is written as a sequence of blocks run one after the other.

**1. The cuts.** Five cuts are applied one after the other, each restricting the sample a little
more toward the exclusive final state of interest:

| Cut | Condition | What it removes |
|---|---|---|
| Exclusivity | \|`miss_m_tot`\| < 0.2 GeV and \|`miss_E_tot`\| < 0.2 GeV | events with missing energy-momentum, i.e. an undetected particle |
| ρ⁰ window | 0.1 < M(π⁺π⁻) < 1.1 GeV | everything outside this (fixed) π⁺π⁻ invariant-mass window |
| Baryon veto | reject M(pπ⁺) < 1.35 GeV and M(pπ⁻) < 1.80 GeV | events going through a baryon resonance (Δ, and up to the N* region for pπ⁻): not our channel |
| ξ cut | 0.04 < ξ < 0.33 | events outside the relevant skewness domain |
| Hard kinematic cuts | −u′ > 1, −t′ > 1, −t < 0.5 GeV², M²(γρ) > 2 GeV² | what lies outside the domain of validity of factorisation |

The number of surviving events is counted at each step and printed — this gives the reduction table
used in the report.

**2. The ρ⁰ fit.** On the π⁺π⁻ invariant-mass spectrum **after cuts** (25 bins, 0.1–1.1 GeV), the
signal is fitted by a **Gaussian** on top of a decreasing exponential background, over 0.1–1.1 GeV.
The spectrum before cuts is shown without a fit.

**3. The pulls.** To judge the quality of the fit, the pipeline builds the pull distribution
((data − fit)/√fit, bin by bin) and displays it on a three-panel canvas — spectrum with the fit
superimposed, pulls below, and the distribution of the pulls on the left.

**4. The control histograms.** Missing mass and energy, M(pπ±), Dalitz plot, kinematic
correlations (−t′ vs −u′, M²(γρ) vs −u′, M²(γρ) vs ξ), and 2D histograms of momentum vs θ and θ vs φ
for each of the four final-state particles are booked, together with dashed red lines marking where
the cuts are.

**5. The outputs.** The sample after all cuts is written as a tree `T_final` in
`<input>_<tree>_phase1.root`.

### Reading the invariant-mass spectrum

The π⁺π⁻ spectrum does not contain only the ρ⁰, and the spurious structures all have a physical
explanation. Nothing to worry about when you discover them:

* a **narrow peak at 0.498 GeV**: this is the K_S⁰, which also decays into π⁺π⁻;
* a **broad structure at low mass**, coming from several superimposed contributions — the Drell
  continuum, Söding interference, the f₀(500), and Δ⁺⁺ reflections.

## ⚠️ Known points in the code (documented, not fixed)

The code is kept exactly as it was used. The following points were noticed and are left for
whoever takes over:

* `h_t_final` is filled with `t` on [−0.5, 0] instead of −t, although its title says "−t".
* The dashed line marking the ρ window on the raw spectrum is drawn at 0.3 GeV, whereas the cut is
  at 0.1 GeV.
* The starting value of the background amplitude in the fit is read with a bin index taken from
  the raw histogram (500 bins), applied to the 25-bin histogram: it falls outside the histogram and
  starts at 0. The fit still converges.
* The line `h_pull_rho_final = make_pull(...)` at the end of the file is indented inside
  `draw_with_pull`, so `h_pull_rho_final` is not defined when `draw_with_pull` is called just after:
  the last block stops with a `NameError` unless that line is de-indented.
* This file contains the selection, the fit and the pull canvas; the part that draws the control
  canvases and exports them to PDF/PNG is not in it, and no output file is explicitly opened for
  writing before the final `Write()` calls. The plots in `Resultats/` come from the run on
  `g12_Total.root` made at CEA.

## Folder contents

```
05_analysis/
├── README.md              ← this file
├── phase1_pipeline.py     ← the pipeline (version A, unmodified)
└── Resultats/             ← output plots of the run on g12_Total.root, one PNG per canvas
```
