/* S�len� cyklus, kazdou iteraci vykon�v� jin� proces, pouzit� viz pipe_sh.c
   (idea prevzata od J. Pavelky) */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define NCHILD 10
#define PARENT_SLEEP 5
#define PARENT_DECR 1
#define CHILD_SLEEP 1

int main()
{
  int i,interval=PARENT_SLEEP;
  pid_t child;
  for(i=0;i<NCHILD;i++)
    switch(child=fork()) {
      case -1:
	perror("fork()");
	exit(1);
	break;
      case 0: /* Syn postupuje do dals� iterace */
	sleep(CHILD_SLEEP);
	interval-=PARENT_DECR; /* Az se stane otcem, bude cekat krats� dobu */
	break;
      default: /* Otec vyp�se r�dek a skonc� */
	printf("pid=%ld, parent_pid=%ld, child_pid=%ld, i=%d\n",
	    (long int)getpid(),(long int)getppid(),(long int)child,i);
	if(interval>0) /* Pokud stihne rodic skoncit dr�ve nez d�te,
		    	  adoptivn�m rodicem se stane init (pid==1) */
	  sleep(PARENT_SLEEP);
	exit(0);
	break;
    }
  /* Sem se dostane jen posledn� syn */
  printf("AFTER FOR: pid=%ld, parent_pid=%ld, i=%d\n",(long int)getpid(),
      (long int)getppid(),i);
  exit(0);
}
