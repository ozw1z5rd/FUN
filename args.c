/* Výpis argumentu programu */

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h> 
/* Protoze pod Linuxem pridává basename navíc '/' na zacátek, musíme jeste
   udelat malou úpravu */
#define __USE_GNU
#undef basename
#include <string.h>

int main(int argc,char *argv[])
{
  int i;
  char *path;
  /* dirname() i basename() mohou zmenit retezec, proto si udeláme kopii */
  if(!(path=malloc(strlen(argv[0])+1))) {
    perror("malloc()");
    exit(1);
  }
  strcpy(path,argv[0]);
  printf("program dirname=\"%s\"\n",dirname(path));
  free(path);
  printf("program basename=\"%s\"\n",basename(argv[0]));
  for(i=0;i<argc;i++)
    printf("argv[%d]=\"%s\"\n",i,argv[i]);
  exit(0);
}
