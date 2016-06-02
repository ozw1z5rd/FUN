/* Výpis obsahu adresáre */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>

int main(int argc,char **argv)
{
  int i;
  DIR *d;
  struct dirent *de;
  char *alt_argv[]={
    "",
    ".",
    0
  };
  if(argc<2) { /* Není zadán adresár, pouzij aktuální */
    alt_argv[0]=argv[0];
    argv=alt_argv;
    argc=2;
  }
  for(i=1;i<argc;i++) {
    if(!(d=opendir(argv[i]))) {
      perror(argv[i]);
      exit(1);
    }
    errno=0;
    while((de=readdir(d)))
      printf("%s\n",de->d_name);
    if(errno) {
      perror("readdir()");
      exit(1);
    }
    if(closedir(d)==-1) {
      perror("closedir()");
      exit(1);
    }
  }
  exit(0);
}
