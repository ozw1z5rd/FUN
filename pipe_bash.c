/* Roura mezi nekolika procesy, tak jak ji staví Bash, tj. shell je
   otcem vsech procesu v kolone, ceká na skoncení vsech a pak vrátí návratovou
   hodnotu posledního. (Pro resení podle Bourne shellu viz pipe_sh.c) */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <stddef.h>
#include <unistd.h>
#include <wait.h>
/* Protoze pod Linuxem pridává basename navíc '/' na zacátek, musíme jeste
   udelat malou úpravu */
#define __USE_GNU
#undef basename
#include <string.h>
#include <errno.h>

int main(int argc,char *argv[])
{
  int i,status,ipd[2],opd[2],exitval;
  pid_t pid,pidw;
  char *sh,*path;
  if(argc<2) {
    fprintf(stderr,"usage: pipe_bash cmd1 ...\n"
	"equivalent to shell command cmd1|cmd2|cmd3|...\n");
    exit(1);
  }
  if(!(sh=getenv("SHELL"))) {
    fprintf(stderr,"Environment variable SHELL not found\n");
    exit(1);
  }
  if(!(path=malloc(strlen(sh)+1))) {
    perror("malloc()");
    exit(1);
  }
  strcpy(path,sh);
  for(i=1;i<argc;i++) {
    if(i>1) { /* Roura pro výstup predchozího procesu je vstupní roura tohoto
	         procesu */
      ipd[0]=opd[0];
      ipd[1]=opd[1];
    }
    if(i<argc-1) /* Vytvorit rouru pro výstup */
      if(pipe(opd)==-1) {
	perror("pipe()");
	exit(1);
      }
    switch(pid=fork()) {
      case -1:
	perror("fork()");
	exit(1);
	break;
      case 0: /* Syn spustí program */
	if(i>1) { /* Není první, bude císt z roury */
	  if(dup2(ipd[0],0)==-1) {
	    perror("dup2()");
	    exit(1);
	  }
	  close(ipd[0]);
	  close(ipd[1]);
	}
	if(i<argc-1) { /* Není poslední, bude zapisovat do roury */
	  if(dup2(opd[1],1)==-1) {
	    perror("dup2()");
	    exit(1);
	  }
	  close(opd[0]);
	  close(opd[1]);
	}
	execl(sh,basename(path),"-c",argv[i],NULL);
	exit(1); /* execl() se vrátil, tudíz se nepovedl */
	break;
      default: /* Otec zavre nepotrebnou rouru */
	close(ipd[0]);
	close(ipd[1]);
	break;
    }
  }
  while((pidw=wait(&status))!=-1||errno!=ECHILD) /* Pockat na vsechny deti */
    if(pidw==-1)
      perror("wait()");
    else
      if(pid==pidw) /* Toto byl proces na konci roury */
	exitval=status;
      /* Takto vrací návratovou hodnotu shell: kdyz príkaz skoncí voláním
         exit(), vrací návratový status, kdyz skoncí na signál, vrací
	 128+císlo signálu */
  if(WIFEXITED(exitval))
    exit(WEXITSTATUS(exitval));
  if(WIFSIGNALED(exitval))
    exit(WTERMSIG(exitval)+128);
  exit(0);
} 
