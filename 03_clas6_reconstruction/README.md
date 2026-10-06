# 03 — CLAS6 reconstruction and conversion to DST format

This folder contains the step that turns the Monte Carlo events, once converted to BOS by
BOSwrite, into a file that RootBeer can read. There are two pieces. The first is the CLAS6
simulation and reconstruction. The second, and the most important one in this folder, is the small
program `mkdst`. It is what unblocked the end of the chain.

```
03_clas6_reconstruction/
├── README.md
├── run_chain.sh          full chain gsim_bat → gpp → a1c → mkdst
└── mkdst/
    ├── mkdst.c           "a1c output → DST" converter
    └── build_mkdst.sh    compilation (+ production) of mkdst
```

## The chain in brief

The `.evt` file produced by BOSwrite goes successively through three CLAS6 programs. `gsim_bat`
simulates the detector with GEANT. `gpp` makes this simulation realistic: it degrades the
resolutions and switches off the channels that were dead during data taking. `a1c` then
reconstructs the tracks and identifies the particles (the "cooking"). These three commands are
those of Josh Bryce's recipe (University of York). The script `run_chain.sh` takes them as they
are; only the file names change.

Everything is run inside the CLAS6 container
(`apptainer shell /group/clas/builds/centos79-clas6.sif`). The ifarm host uses tcsh, whereas the
inside of the container uses bash. The two cards `ffread.g12` and `prlink_tg-90pm30.bos` are not in
the repository. They were in `/work/clas/clasg12/madani/gen/` on ifarm.

The output of `a1c` (`cooked_out.bos`) is physically correct. It is nevertheless not usable as it
is, and this is what `mkdst` fixes.

## Why mkdst exists

On the raw output of `a1c`, RootBeer (`rbtest`) crashes with a segmentation fault, in
`TBos::DataServer` / `TBos::BankScanner`, before even the first event. A real g12 DST, for
example `56363.A17` (taken directly from the detector), converts without any problem in the same
session. The problem therefore comes neither from RootBeer nor from ROOT, but from the file.

A bank-by-bank comparison showed what distinguishes this file from a real DST. The difference is
not a matter of format: the common banks have exactly the same structure. It is a matter of
content. The `a1c` output keeps everything the reconstruction handled internally: the generator
parameters (`GPAR`), the Monte Carlo truth banks (`MCEV`, `MCTK`, `MCVX`) and the whole series of
low-level banks (drift-chamber, calorimeter and time-of-flight hits…). A production DST keeps only
about fifty of them. mkdst builds that DST. It is written on the model of `catbos.c`, taken from
the CLAS6 source code, and relies directly on the BOS library of the container. It reads the
cooked file event by event. In each event, it removes 24 banks: GPAR, CALL, the Monte Carlo truth
banks (MCEV, MCTK, MCVX) and all the hit banks (DC0, DC1, DCH, DHCL, DOCA, ECH, HBER, HBID, HBLA,
HBTR, HDPL, SC, SC1, SCH, SCR, STH, STN0, STN1, TBLA). It then rewrites the slimmed event into a file
with 3600-byte records, like a production DST. All other banks are kept, TAGR included: it gives
the tagged photon energy and remains essential for the analysis.

Removing the banks relies on three details, each of which is necessary. First, the function
`bosNdrop` receives `bcs_.iw` as its first argument, that is, the integer array of the BOS
structure, and not the address of the structure. Next, each bank name is padded with spaces to
exactly 4 characters (`"SC  "`, `"DC0 "`…). Finally, the removal is repeated for all instance
numbers from 0 to 700, so as to reach every sector and every segment.

The first event of the file must also be removed. a1c writes at the beginning a set-up
pseudo-event containing GPAR and HEAD. Once GPAR is removed, only an orphan HEAD remains. RootBeer
crashes when this pseudo-event and the TAGR bank are present together. The option `-s1` skips this
pseudo-event: it is part of the solution, and the production command always uses it.

## Compiling mkdst

Compilation is done in the CLAS6 container, in the working directory where `cooked_out.bos`
(output of a1c) is located:

```bash
apptainer shell /group/clas/builds/centos79-clas6.sif
cd /work/clas/clasg12/madani/gen
bash build_mkdst.sh
```

The script does three things. First, it creates a small file `wcs_def.c` containing a single line
(`int wcs_[700008];`). It defines the array `wcs_` that the BOS library expects but that none of
the available libraries provides. Next, it compiles:

```bash
gcc -o mkdst mkdst.c wcs_def.c -I/opt/clas/include \
  -Wl,--start-group /opt/clas/trunk/build/lib/libc_bos_io.a /opt/clas/trunk/build/lib/libbosio.a -Wl,--end-group \
  -L/opt/cernlib/2005/lib -lpacklib -lmathlib -lgfortran -lm -lnsl
```

The `--start-group` / `--end-group` options are necessary: the two BOS libraries call each other,
and the linker must scan them together. Finally, the script directly runs the production
(`./mkdst -s1 cooked_out.bos sim_dst_final.bos`). If you only want to compile, stop after the
`gcc` line.

Compilation only works in the container. The headers (`/opt/clas/include`), the BOS libraries and
CERNLIB only exist there.

## Using mkdst

```bash
./mkdst -s1 cooked_out.bos sim_dst_final.bos
```

The first argument is the raw `a1c` output, the second the DST file to create. The option `-sN`
skips the first N events; in production, `-s1` is always used. If it already exists, the output
file is deleted and rewritten. At the end, the program prints the number of events read and written
(`lus=… ecrits=… (skip=1)`). The number written must equal the number read minus one.

The resulting file is then converted to ROOT with `rbtest`, outside the container, under ROOT 6
(see `04_bos_to_root_RootBeer`).

In `run_chain.sh`, `mkdst` is the fourth and last step. The script calls it as `./mkdst`: the
compiled binary must therefore be in the working directory.
