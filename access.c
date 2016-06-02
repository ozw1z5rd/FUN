#include <unistd.h>
#include <stdio.h>
#include <errno.h>

void usage()
{
  fprintf(stderr,"usage: access [-rwx] file ...\n");
  exit(1);
}

int main(int argc,char *argv[])
{
  int opt,i;
  int amode=0;
  while((opt=getopt(argc,argv,"rwx"))!=-1)
    switch(opt) {
      case 'r':
	amode=amode|R_OK;
	break;
      case 'w':
	amode=amode|W_OK;
	break;
      case 'x':
	amode=amode|X_OK;
	break;
      default:
	usage();
	break;
    }
  if(!amode)
    amode=F_OK;
  if(argc-optind==0)
    usage();
  for(i=optind;i<argc;i++)
    if(access(argv[i],amode)==-1)
      if(errno==EACCES)
	printf("%s: access denied\n", argv[i]);
      else {
	perror(argv[i]);
	exit(1);
      }
    else
      printf("%s: access permitted\n",argv[i]);
  exit(0);
}
