/* Vytvorení nového procesu, spustení programu a cekání na jeho ukoncení */

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc,char *argv[])
{
  int pid,status,i;
  if(argc<2) {
    fprintf(stderr,"Usage: fork_exec program [arg ...]\n");
    exit(1);
  }
  if((pid=fork())==-1) {
    perror("fork()");
    exit(1);
  }
  if(pid>0) { /* Otec */
    if(wait(&status)==-1) {
      perror("wait()");
      exit(1);
    }
    if(WIFEXITED(status)) {
      fprintf(stderr,"Child finished with exit code %d\n",
	  WEXITSTATUS(status));
      exit(WEXITSTATUS(status));
    }
    if(WIFSIGNALED(status)) {
      fprintf(stderr,"Child terminated by signal %d\n",WTERMSIG(status));
      exit(1);
    }
    /* Sem by se program nikdy nemel dostat */
    fprintf(stderr,"Strange status %d returned from wait()\n",status); 
    exit(1);
  } else { /* Syn */
    fprintf(stderr,"Process %ld started\n",(long int)getpid());
    fprintf(stderr,"Running: %s",argv[1]);
    for(i=2;i<argc;i++)
      fprintf(stderr," %s",argv[i]);
    fprintf(stderr,"\n");
    execv(argv[1],argv+1);
    perror("execv()"); /* Úspesný exec() se nikdy nevrátí */
    fprintf(stderr,"Trying execvp()\n");
    execvp(argv[1],argv+1);
    perror("execvp()");
    exit(1);
  }
}
