/* Obrácení obsahu souboru namapovaného do pameti */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc,char *argv[])
{
  int fd,fsz;
  char *addr,*p1,*p2,c;
  if(argc!=2) {
    fprintf(stderr,"usage: %s filename\n",argv[0]);
    exit(1);
  }
  if((fd=open(argv[1],O_RDWR))==-1) {
    perror(argv[1]);
    exit(1);
  }
  if((fsz=lseek(fd,0,SEEK_END))==-1) {
    perror("lseek()");
    exit(1);
  }
  if((p1=addr=mmap(0,fsz,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0))==MAP_FAILED) {
    perror("mmap()");
    exit(1);
  }
  p2=p1+fsz-1;
  while(p1<p2) {
    c=*p1;
    *p1++=*p2;
    *p2--=c;
  }
  if(munmap(addr,fsz)==-1) {
    perror("munmap()");
    exit(1);
  }
  if(close(fd)==-1) {
    perror("close()");
    exit(1);
  }
  exit(0);
}
