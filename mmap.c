#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define GETCHARS 8
#define BUFSIZE 128

int fd=-1;
char *fname;
int fsize;
char *mapped=MAP_FAILED;

void err_exit(const char *msg)
{
  perror(msg);
  if(mapped!=MAP_FAILED)
    munmap(mapped,fsize);
  if(fd!=-1)
    close(fd);
  exit(1);
}

int check_bounds(off_t *pos,size_t *bytes)
{
  if(*bytes<=0)
    return 0;
  if(*pos<0)
    pos=0;
  if(*pos>=fsize)
    *pos=fsize-1;
  if(*pos+*bytes>fsize)
    *bytes=fsize-*pos;
  return 1;
}

void write_at(int fd,off_t pos,const char *buf,size_t bytes)
{
  if(!check_bounds(&pos,&bytes))
    return;
  if(lseek(fd,pos,SEEK_SET)==-1)
    err_exit(fname);
  if(write(fd,buf,bytes)==-1)
    err_exit(fname);
}

void read_at(int fd,off_t pos,char *buf,size_t bytes)
{
  if(!check_bounds(&pos,&bytes))
    return;
  if(lseek(fd,pos,SEEK_SET)==-1)
    err_exit(fname);
  if(read(fd,buf,bytes)==-1)
    err_exit(fname);
}

int main(int argc,char *argv[])
{
  int pos;
  int len;
  char buf[BUFSIZE];
  char *s;
  if(argc!=2) {
    fprintf(stderr,"usage: mmap filename\n"
"input: <offset> ... write %d characters starting at <offset>\n"
"       <offset> <chars> ... write <chars> into file starting at <offset>\n",
GETCHARS);
    exit(1);
  }
  fname=argv[1];
  if((fd=open(fname,O_RDWR))==-1)
    err_exit(fname);
  if((fsize=lseek(fd,0,SEEK_END))==-1)
    err_exit(fname);
  if((mapped=mmap(0,fsize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0))==MAP_FAILED)
    err_exit("mmap()");
  while(fscanf(stdin,"%i",&pos)==1) {
    if(!fgets(buf,BUFSIZE,stdin))
      break;
    len=strlen(buf);
    if(len>0&&buf[len-1]=='\n') /* Zrusit newline na konci */
      buf[--len]='\0';
    s=buf;
    if(buf[0]==' ') { /* Preskocit mezeru za offsetem */
      s++;
      len--;
    }
    if(len) /* Zápis do souboru */
      write_at(fd,pos,s,len);
    else { /* Ctení ze souboru */
      read_at(fd,pos,buf,GETCHARS);
      buf[GETCHARS]='\0'; /* Zakoncit nactený retezec */
      if(puts(buf)==EOF)
	err_exit("stdout");
    }
  }
  if(ferror(stdin))
    err_exit("stdin");
  if(munmap(mapped,fsize)==-1)
    err_exit("munmap()");
  close(fd);
  exit(0);
}
