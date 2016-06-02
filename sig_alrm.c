/* Casove omezený vstup pomocí signálu SIGALRM */

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define BUFSZ 4096

void handler(int sig)
{
  fprintf(stderr," !!! TIMEOUT !!! \n");
}

int main()
{
  char buf[BUFSZ];
  struct sigaction act;
  int sz=0;
  act.sa_handler=handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGALRM,&act,NULL)==-1) {
    perror("sigaction()");
    exit(1);
  }
  alarm(5);
  if((sz=read(0,buf,BUFSZ))==-1) {
    perror("read()");
    if(errno!=EINTR)
      exit(1);
  }
  if(sz>0&&write(1,buf,sz)==-1) {
    perror("write()");
    exit(1);
  }
  exit(0);
}
