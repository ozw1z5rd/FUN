/* Kopie souboru (pouzití volání jádra) */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define USAGE "usage: copy src_file dest_file\n"
#define OLOST "some output lost\n"

#define BUFSIZE 4096

char buf[BUFSIZE];

int main(int argc,char *argv[])
{
  int inf,outf;
  int len,ilen,olen;
  if(argc!=3) {
    len=strlen(USAGE);
    write(2,USAGE,len);
    exit(1);
  }
  if((inf=open(argv[1],O_RDONLY))==-1) {
    perror(argv[1]);
    exit(1);
  }
  if((outf=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0666))==-1) {
    perror(argv[2]);
    close(inf);
    exit(1);
  }
  while((ilen=read(inf,buf,BUFSIZE))>0)
    if((olen=write(outf,buf,ilen))!=ilen) {
      if(olen==-1)
	perror(argv[2]);
      else { /* Toto (zápis méne nez ilen bajtu) by nemelo nastat. */
	len=strlen(OLOST);
	write(2,OLOST,len);
      }
      close(inf);
      close(outf);
      exit(1);
    }
  if(ilen==-1) {
    perror(argv[1]);
    close(inf); /* Nelze tuto dvojici close() sloucit s následující a presunout
		   pred if(ilen==-1), protoze by se mohl zmenit výsledek
		   perror() */
    close(outf);
    exit(1);
  }
  close(inf); /* Vsechna close() v tomto programu lze vypustit, soubory jsou
		 uzavreny pri ukoncení procesu */
  close(outf);
  exit(0);
}
