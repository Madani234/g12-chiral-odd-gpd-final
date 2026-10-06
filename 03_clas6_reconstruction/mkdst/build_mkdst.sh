#!/bin/bash
# A lancer DANS le conteneur CLAS6 :
#   apptainer shell /group/clas/builds/centos79-clas6.sif
#   cd /work/clas/clasg12/madani/gen
#   bash build_mkdst.sh

# Symbole wcs_ requis a l'edition de liens
echo 'int wcs_[700008];' > wcs_def.c

# Compilation (--start-group/--end-group : references circulaires entre les deux libs BOS)
gcc -o mkdst mkdst.c wcs_def.c -I/opt/clas/include \
  -Wl,--start-group /opt/clas/trunk/build/lib/libc_bos_io.a /opt/clas/trunk/build/lib/libbosio.a -Wl,--end-group \
  -L/opt/cernlib/2005/lib -lpacklib -lmathlib -lgfortran -lm -lnsl

# Production (le -s1 fait partie de la solution)
rm -f sim_dst_final.bos
./mkdst -s1 cooked_out.bos sim_dst_final.bos
