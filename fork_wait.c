/* Vytvorení synovského procesu a cekání na jeho ukoncení */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/wait.h>

#define CHILDEXIT 12
#define CHILDWAIT 15

void pidfprintf(FILE *stream,const char *format,...)
{
  va_list al;
  fprintf(stream,"[%ld] ",(long int)getpid());
  va_start(al,format);
  vfprintf(stream,format,al);
  va_end(al);
}

void pidperror(const char *msg)
{
  pidfprintf(stderr,"");
  perror(msg);
  exit(1);
}

int main()
{
  pid_t child;
  int status;
  pidfprintf(stdout,"parent started\n");
  sleep(1);
  switch(child=fork()) {
    case -1: /* Chyba */
      pidperror("fork()");
      break;
    case 0: /* Syn */
      pidfprintf(stdout,"child started\n");
      printf("Remaining time: %d\n",sleep(CHILDWAIT));
      exit(CHILDEXIT);
      break;
    default: /* Otec */
      pidfprintf(stdout,"parent waiting (for %ld)\n",(long int)child);
      if((child=wait(&status))==-1)
	pidperror("wait()");
      if(WIFEXITED(status))
	pidfprintf(stdout,"exit(%d) from pid %ld\n",WEXITSTATUS(status),
	    (long int)child);
      if(WIFSIGNALED(status))
	pidfprintf(stdout,"signal %d to pid %ld\n",WTERMSIG(status),
	    (long int)child);
      break;
  }
  exit(0);
}
