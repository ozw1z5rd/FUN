/* Roura mezi nekolika procesy, tak jak ji staví Bourne shell, tj. shell je
   otcem posledního procesu v kolone, dále vzdy proces je otcem predchozího
   procesu v kolone. (V ksh a bash jsou vsechny procesy kolony syny shellu,
   viz pipe_bash.c) */

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

int main(int argc,char *argv[])
{
  int i,status,pd[2];
  char *sh,*path;
  if(argc<2) {
    fprintf(stderr,"usage: pipe_sh cmd1 ...\n"
	"equivalent to shell command cmd1|cmd2|cmd3|...\n");
    exit(1);
  }
  switch(fork()) {
    case -1:
      perror("fork()");
      exit(1);
    case 0: /* Syn bude provádet príkazy */
      for(i=argc-1;i>=1;i--) {
	if(i>1) { /* Není to zacátek kolony, bude císt z roury */
	  if(pipe(pd)==-1) {
	    perror("pipe()");
	    exit(1);
	  }
	  switch(fork()) { /* Dalsí generace */
	    case -1:
	      perror("fork()");
	      exit(1);
	      break;
	    case 0: /* Syn postupuje do dalsího kola s výstupem do roury */
	      if(dup2(pd[1],1)==-1) {
		perror("dup2()");
		exit(1);
	      }
	      close(pd[0]);
	      close(pd[1]);
	      continue;
	  }
	  /* Otec bude císt z roury */
	  if(dup2(pd[0],0)==-1) {
	    perror("dup2()");
	    exit(1);
	  }
	  close(pd[0]);
	  close(pd[1]);
    	} 
	/* Otec, resp. proces na zacátku roury, spustí príkaz */
	if(!(sh=getenv("SHELL"))) {
	  fprintf(stderr,"Environment variable SHELL not found\n");
	  exit(1);
	}
	if(!(path=malloc(strlen(sh)+1))) {
	  perror("malloc()");
	  exit(1);
	}
	strcpy(path,sh);
	execl(sh,basename(path),"-c",argv[i],NULL);
	exit(1); /* execl() se vrátil, tudíz se nepovedl */
      }
      break;
    default: /* Otec (úplne první proces), bude cekat na syna (tj. na poslední 
		proces v kolone) */
      if(wait(&status)==-1) {
	perror("wait()");
	exit(1);
      }
      /* Takto vrací návratovou hodnotu shell: kdyz príkaz skoncí voláním
         exit(), vrací návratový status, kdyz skoncí na signál, vrací
	 128+císlo signálu */
      if(WIFEXITED(status))
	exit(WEXITSTATUS(status));
      if(WIFSIGNALED(status))
	exit(WTERMSIG(status)+128);
      break;
  }
  exit(0);
}
