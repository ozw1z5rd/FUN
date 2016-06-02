/* Zamyk�n� souboru */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>

#define LINE_LEN 40
#define SLEEP_CHAR 10
#define SLEEP_LINE 100

#ifdef LOCK_NOLOCK /* Z�dn� zamyk�n� */
#define LOCK
#define UNLOCK
#elif LOCK_LOCKFILE /* Zamyk�n� pomoc� lock souboru */
#define LOCK lockfile_lock();
#define UNLOCK lockfile_unlock();
#elif LOCK_FLOCK /* Zamyk�n� pomoc� flock */
#define LOCK flock_lock();
#define UNLOCK flock_unlock();
#else
#error "Locking mechanism???"
#endif

#ifdef LOCK_LOCKFILE
char *lockfile;
char *fname;
int locked=0;

/* Nab�z� se vytvoren� lockfile pomoc� vol�n� open() s pr�zkaky
   O_CREAT|O_EXCL, ale to nemus� fungovat pro soubory sd�len� pres NFS.
   Proto pouzijeme trochu slozitejs� zpusob. */

void lockfile_lock()
{
  int fd;
  struct stat buf;
  if((fd=open(fname,O_RDWR|O_CREAT,0600))==-1) {
    perror(fname);
    exit(1);
  }
  close(fd);
  for(;;) {
    if(link(fname,lockfile)==-1) {
      if(errno==EEXIST)
	continue; /* Lockfile existuje, jin� proces drz� z�mek, zkusit znovu */
      else {
	perror("link()");
	exit(1);
      }
    }
    /* Na NFS ani �spech link() nemus� zajistit spr�vnou funkci, proto jeste 
       otestujeme, zda se zv�sil pocet linku souboru fname */
    if(stat(fname,&buf)==-1) {
      perror(fname);
      exit(1);
    }
    if(buf.st_nlink==2)
      break; /* Ted drz�me z�mek */
  }
  if(unlink(fname)==-1) {
    perror(fname);
    exit(1);
  }
  locked=1;
}

void lockfile_unlock()
{
  if(unlink(lockfile)==-1) {
    perror(lockfile);
    exit(1);
  }
  locked=0;
}

void handler(int sig)
{
  unlink(fname);
  if(locked)
    unlink(lockfile);
  exit(sig);
}
#endif

#ifdef LOCK_FLOCK
void flock_lock()
{
  if(flock(1,LOCK_EX)==-1) {
    perror("flock()");
    exit(1);
  }
}

void flock_unlock()
{
  if(flock(1,LOCK_UN)==-1) {
    perror("flock()");
    exit(1);
  }
}
#endif

void do_sleep(int msec)
{
  struct timespec st,rst;
  if(msec<=0)
    return;
  st.tv_nsec=(msec%1000)*1000000;
  st.tv_sec=msec/1000;
  while(nanosleep(&st,&rst)==-1) {
    perror("nanosleep()");
    if(errno!=EINTR)
      exit(1);
    else
      st=rst;
  }
}

int main(int argc,char *argv[])
{
  char out;
  int i;
#ifdef LOCK_LOCKFILE
  if(argc!=3) {
    fprintf(stderr,"usage:%s character lockfile\n",argv[0]);
    exit(1);
  }
  lockfile=argv[2];
  if(!(fname=malloc(strlen(lockfile)+16))) {
    perror("malloc()");
    exit(1);
  }
  sprintf(fname,"%s%d",lockfile,getpid());
  signal(SIGINT,handler); /* Zajistit zrusen� z�mku pri n�siln�m ukoncen� */
  signal(SIGTERM,handler);
  signal(SIGQUIT,handler);
  signal(SIGHUP,handler);
#else
  if(argc!=2) {
    fprintf(stderr,"usage: %s character\n",argv[0]);
    exit(1);
  }
#endif
  out=argv[1][0];
  for(;;) {
    LOCK
    for(i=0;i<LINE_LEN;i++) {
      if(write(1,&out,1)==-1) {
    	perror("write()");
      	exit(1);
      }
      do_sleep(SLEEP_CHAR);
    }
    if(write(1,"\n",1)==-1) {
      perror("write()");
      exit(1);
    }
    UNLOCK
    do_sleep(SLEEP_LINE);
  }
}
