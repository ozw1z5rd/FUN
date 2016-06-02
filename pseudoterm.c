/* Vytvorení pseudoterminálu */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>

const char pts_master[]="/dev/ptmx";

#define BUFSZ 64

int run_echoing(int fd) /* Prepisuje stdin do fd a naopak, koncí pri EOF,
			   vrací 1 pri EOF na stdin, jinak vrací 0 */
{
  int sz,szw;
  fd_set rfdset,efdset;
  char buf[BUFSZ];
  for(;;) {
    FD_ZERO(&rfdset);
    FD_SET(0,&rfdset);
    FD_SET(fd,&rfdset);
    efdset=rfdset;
    while(select(fd+1,&rfdset,NULL,&efdset,NULL)==-1) {
      perror("select()");
      if(errno==EINTR)
	continue;
      exit(1);
    }
    if(FD_ISSET(0,&efdset))
      fprintf(stderr,"### EXCEPTION AT stdin DETECTED BY select(). ###\n");
    if(FD_ISSET(fd,&efdset))
      fprintf(stderr,
	      "### EXCEPTION AT pseudo-terminal DETECTED BY select(). ###\n");
    if(FD_ISSET(0,&rfdset)) {
      if((sz=read(0,buf,BUFSZ))==-1) {
	perror("read(stdin)");
	exit(1);
      }
      if(sz==0) {
	fprintf(stderr,"### EOF ON stdin. ###\n");
	return 1;
      }
      fprintf(stderr,"### RECEIVED %d BYTES FROM stdin. ###\n",sz);
      if((szw=write(fd,buf,sz))==-1) {
	perror("write(pts)");
	exit(1);
      }
      fprintf(stderr,"### SENT %d BYTES TO pseudo-terminal. ###\n",szw);
      if(szw<sz)
	fprintf(stderr,"### DATA LOST DURING WRITE TO pseudo-terminal. ###\n");
    }
    if(FD_ISSET(fd,&rfdset)) {
      if((sz=read(fd,buf,BUFSZ))==-1) {
	perror("read(pts)");
	if(errno!=EIO)
  	  exit(1);
	else
	  continue;
      }
      if(sz==0) {
	fprintf(stderr,"### EOF ON pseudo-terminal. ###\n");
	return 0;
      }
      fprintf(stderr,"### RECEIVED %d BYTES FROM pseudo-terminal. ###\n",sz);
      if((szw=write(1,buf,sz))==-1) {
	perror("write(stdout)");
	exit(1);
      }
      fprintf(stderr,"### SENT %d BYTES TO stdout. ###\n",szw);
      if(szw<sz)
	fprintf(stderr,"### DATA LOST DURING WRITE TO stdout. ###\n");
    }
  }
}

int main()
{
  int mfd;
  char *sname;
  if((mfd=open(pts_master,O_RDWR|O_NOCTTY,0))==-1) {
    perror(pts_master);
    exit(1);
  }
  if(grantpt(mfd)==-1) {
    perror("grantpt()");
    exit(1);
  }
  if(unlockpt(mfd)==-1) {
    perror("unlockpt()");
    exit(1);
  }
  if(!(sname=ptsname(mfd))) {
    perror("ptsname()");
    exit(1);
  }
  fprintf(stderr,"### SLAVE DEVICE IS %s ###\n",sname);
  while(!run_echoing(mfd))
    ;
  exit(0);
}
