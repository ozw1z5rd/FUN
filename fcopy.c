/* Kopie souboru (pouzití knihovny stdio) */

#include <stdio.h>

#define OLOST "some output lost\n"

#define BUFSIZE 4096

char buf[BUFSIZE];

int main(int argc,char *argv[])
{
  FILE *inf,*outf;
  int ilen;
  if(argc!=3) {
    fprintf(stderr,"usage: copy src_file dest_file\n");
    exit(1);
  }
  if(!(inf=fopen(argv[1],"r"))) {
    perror(argv[1]);
    exit(1);
  }
  if(!(outf=fopen(argv[2],"w"))) {
    perror(argv[2]);
    fclose(inf);
    exit(1);
  }
  do {
    if((ilen=fread(buf,sizeof(char),BUFSIZE,inf))<BUFSIZE&&ferror(inf)) {
      perror(argv[1]);
      fclose(inf);
      fclose(outf);
      exit(1);
    }
    if(fwrite(buf,sizeof(char),ilen,outf)<ilen) {
      perror(argv[2]);
      fclose(inf);
      fclose(outf);
      exit(1);
    }
  } while(ilen==BUFSIZE);
  fclose(inf);
  fclose(outf);
  exit(0);
}
