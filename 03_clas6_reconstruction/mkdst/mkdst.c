/* mkdst.c v2 : cooked -> DST-like. -sN saute les N premiers evenements. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ntypes.h>
#include <bostypes.h>

static const char *DROP[] = {
  "CALL","DC0","DC1","DCH","DHCL","DOCA","ECH","GPAR",
  "HBER","HBID","HBLA","HBTR","HDPL","MCEV","MCTK","MCVX",
  "SC","SC1","SCH","SCR","STH","STN0","STN1","TBLA", NULL
};

static void dropname(const char *name)
{
  char nm[8]; int nr;
  memset(nm,' ',4); nm[4]='\0';
  memcpy(nm, name, strlen(name)>4?4:strlen(name));
  for (nr = 0; nr <= 700; nr++)
    bosNdrop(bcs_.iw, nm, nr);
}

int main(int argc, char **argv)
{
  char cmd[300];
  char *inf = NULL, *outf = NULL;
  int  IN = 1, OUT = 7, nev = 0, nwr = 0, skip = 0, i;

  for (i = 1; i < argc; i++) {
    if (argv[i][0]=='-' && argv[i][1]=='s') skip = atoi(argv[i]+2);
    else if (!inf)  inf  = argv[i];
    else            outf = argv[i];
  }
  if (!inf || !outf) { fprintf(stderr,"usage: %s [-sN] in.bos out.bos\n",argv[0]); return 1; }

  initbos();
  bnames(-1);

  unlink(outf);
  sprintf(cmd,"OPEN BOSOUTPUT UNIT=%d FILE=\"%s\" WRITE STATUS=NEW RECL=3600", OUT, outf);
  if (!fparm_c(cmd)) { fprintf(stderr,"open out fail: %s\n", strerror(errno)); return 1; }
  sprintf(cmd,"OPEN BOSINPUT UNIT=%d FILE=\"%s\" READ", IN, inf);
  if (!fparm_c(cmd)) { fprintf(stderr,"open in fail: %s\n", strerror(errno)); return 1; }

  while (getBOS(&bcs_, IN, "E")) {
    nev++;
    if (nev > skip) {
      for (i = 0; DROP[i]; i++) dropname(DROP[i]);
      putBOS(&bcs_, OUT, "E");
      nwr++;
    }
    if (nev % 500 == 0) { fprintf(stderr,"  %d evts\r", nev); fflush(stderr); }
    dropAllBanks(&bcs_, "E");
    cleanBanks(&bcs_);
  }
  putBOS(&bcs_, OUT, "0");
  sprintf(cmd,"CLOSE BOSINPUT UNIT=%d",  IN);  fparm_c(cmd);
  sprintf(cmd,"CLOSE BOSOUTPUT UNIT=%d", OUT); fparm_c(cmd);
  fprintf(stderr,"\nlus=%d ecrits=%d (skip=%d) -> %s\n", nev, nwr, skip, outf);
  return 0;
}
