# BOS → ROOT conversion of the CLAS g12 data

This folder documents how the real data of the CLAS g12 experiment (JLab), in the native CLAS6 BOS
format, were converted into ROOT files usable for the analysis.

* **`convert_bos_to_root.tcsh`**: the step-by-step procedure — splitting, conversion, recovery of
  failures, check, merge. Each stage is run separately so that the result can be checked in between:
  `tcsh convert_bos_to_root.tcsh {prepare|test|run|check|retry|merge}` (see the header of the
  script).
* **`rbtest.C`**: RootBeer macro written by Pierre Chatagnon: reads the BOS files, selects the
  events, writes the ROOT file.
* **`README.md`**: this file.

## 1. Splitting and reconstruction

The starting data are those of the g12 pass1 skim `2-2pos1neg_not_1ckaon1ctrk`, on `/cache`. This
represents about 60,000 BOS files.

1. **Master list.** All BOS files are listed, i.e. about 59,973 entries.
2. **Splitting.** The list is cut into batches of **50 files**, giving about 1200 lists, from
   `liste_0000` to `liste_1199`. This choice is empirical: with batches of about 600 files, there
   were about 18 % timeouts and about 16 % lost events.
3. **Conversion.** `rbtest.C` runs on each list, in the RootBeer 6.0 environment under tcsh,
   directly on ifarm. Each batch produces a file `g12_NNNN.root` and a log.
4. **Recovering failures.** Batches that timed out are found with `grep "timed out"` in the logs,
   then relaunched, until there are no timeouts left. This cycle recovered 65 missing files. Only
   one batch remained unrecoverable: `g12_0688`, a zombie file.
5. **Check.** The size of the `.root` files is checked before merging.
6. **Rebuilding the full dataset.** All batches are merged with `hadd` into a single file
   `g12_Total.root` (about 60 GB), then brought back to the local machine by `scp` through
   `login.jlab.org`.

---

## 2. Pierre's code: `rbtest.C`

### What it does

For each BOS file in the list it is given, the macro:

1. **reads three BOS banks**:
   * `PART`: the reconstructed particles (identifier and momentum);
   * `TAGR`: the photon tagger, which gives the energy of the incident photon;
   * `SCR`: the scintillators, used only for control histograms;
2. **selects the events** containing **exactly** one proton (GEANT PID 14), one π⁺ (8), one π⁻ (9)
   and one photon (1);
3. **builds the four-vectors**. Masses are assigned from the PID: m_π = 0.13957 GeV,
   m_p = 0.93827 GeV, massless photon. The incident photon is taken along z, with the energy of the
   first tagger hit (`TAGR[0]`). The target is a proton at rest;
4. **computes the kinematic quantities**, then writes the event into the output tree.

At the end of the run, the macro prints the total number of events read and the number of events
kept (`Found N events of interest`).

### Usage

```
rbtest(nEvents, input, output)
```

* `nEvents`: `0` to read everything;
* `input`: a BOS file, or a list of files preceded by `-L` (`"-L<list>"`);
* `output`: the ROOT file to create.

Example, run under tcsh after loading the RootBeer environment:

```
rootbeer -b -q 'rbtest.C(0,"-L'$f'","'$out'")'
```

### Structure of the ROOT files produced

Each file contains a tree **`T`** ("Tree with event particles"), with one entry per selected
event.

**Four-vectors** (`TLorentzVector`)

| Branch | Content |
|---|---|
| `photon_ini` | incident photon (beam) |
| `proton` | recoil proton |
| `piPlus`, `piMinus` | charged pions |
| `photon` | final-state photon |
| `missing` | missing four-vector: γ_ini + p_target − (π⁺ + π⁻ + p + γ) |

**Invariant masses** (`float`, in GeV)

| Branch | System |
|---|---|
| `mpipi` | π⁺π⁻ |
| `mrhophot` | π⁺π⁻γ |
| `mprotphot` | pγ |
| `mpipprot`, `mpimprot` | π⁺p, π⁻p |
| `mpipphot`, `mpimphot` | π⁺γ, π⁻γ |

**Exclusivity** (`float`)

| Branch | Content |
|---|---|
| `miss_m_tot` | total missing mass |
| `miss_E_tot` | total missing energy |

**Mandelstam variables** (`float`, in GeV²)

| Branch | Definition |
|---|---|
| `s` | (γ_ini + p_target)² |
| `t` | (p_target − p′)² |
| `t_prime` | (γ_ini − γ)² |
| `u_prime` | (γ_ini − π⁺ − π⁻)² |
| `u` | set to 0, not computed |

After the merge, `g12_Total.root` has exactly the same structure: the tree `T` gathers the events
of all batches.
