/* Roura mezi nekolika procesy, tak jak ji stav� Bourne shell, tj. shell je
   otcem posledn�ho procesu v kolone, d�le vzdy proces je otcem predchoz�ho
   procesu v kolone. (V ksh a bash jsou vsechny procesy kolony syny shellu,
   viz pipe_bash.c) */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <stddef.h>
#include <unistd.h>
#include <wait.h>
/* Protoze pod Linuxem prid�v� basename nav�c '/' na zac�tek, mus�me jeste
   udelat malou �pravu */
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
    case 0: /* Syn bude prov�det pr�kazy */
      for(i=argc-1;i>=1;i--) {
	if(i>1) { /* Nen� to zac�tek kolony, bude c�st z roury */
	  if(pipe(pd)==-1) {
	    perror("pipe()");
	    exit(1);
	  }
	  switch(fork()) { /* Dals� generace */
	    case -1:
	      perror("fork()");
	      exit(1);
	      break;
	    case 0: /* Syn postupuje do dals�ho kola s v�stupem do roury */
	      if(dup2(pd[1],1)==-1) {
		perror("dup2()");
		exit(1);
	      }
	      close(pd[0]);
	      close(pd[1]);
	      continue;
	  }
	  /* Otec bude c�st z roury */
	  if(dup2(pd[0],0)==-1) {
	    perror("dup2()");
	    exit(1);
	  }
	  close(pd[0]);
	  close(pd[1]);
    	} 
	/* Otec, resp. proces na zac�tku roury, spust� pr�kaz */
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
	exit(1); /* execl() se vr�til, tud�z se nepovedl */
      }
      break;
    default: /* Otec (�plne prvn� proces), bude cekat na syna (tj. na posledn� 
		proces v kolone) */
      if(wait(&status)==-1) {
	perror("wait()");
	exit(1);
      }
      /* Takto vrac� n�vratovou hodnotu shell: kdyz pr�kaz skonc� vol�n�m
         exit(), vrac� n�vratov� status, kdyz skonc� na sign�l, vrac�
	 128+c�slo sign�lu */
      if(WIFEXITED(status))
	exit(WEXITSTATUS(status));
      if(WIFSIGNALED(status))
	exit(WTERMSIG(status)+128);
      break;
  }
  exit(0);
}
