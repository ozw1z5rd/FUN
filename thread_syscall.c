/* Test, zda ostatní thready bezí, kdyz nekteré cekají v systémovém volání */

#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define SLEEP 5

void *thread_run(void *str)
{
  while(1) {
    sleep(SLEEP);
    printf("%s\n",(char*)str);
  }
}

void *thread_pause(void *arg)
{
  sigset_t s;
  int sig;
  sigemptyset(&s);
  sigaddset(&s,SIGUSR1);
  while(1)
    if(!sigwait(&s,&sig)&&sig==SIGUSR1)
      printf("SIGUSR 1 caught\n");
}

void *thread_cat(void *arg)
{
  char buf;
  while(1)
    if(read(0,&buf,1)==1)
      write(1,&buf,1);
}

int main()
{
  sigset_t s;
  pthread_t tid;
  sigemptyset(&s);
  sigaddset(&s,SIGUSR1);
  sigprocmask(SIG_BLOCK,&s,NULL);
  if((errno=pthread_create(&tid,NULL,thread_run,"*** THREAD 1 ***"))) {
    perror("pthread_create()");
    exit(1);
  }
  pthread_detach(tid);
  if((errno=pthread_create(&tid,NULL,thread_run,"*** THREAD 2 ***"))) {
    perror("pthread_create()");
    exit(1);
  }
  pthread_detach(tid);
  if((errno=pthread_create(&tid,NULL,thread_pause,NULL))) {
    perror("pthread_create()");
    exit(1);
  }
  pthread_detach(tid);
  if((errno=pthread_create(&tid,NULL,thread_cat,NULL))) {
    perror("pthread_create()");
    exit(1);
  }
  pthread_detach(tid);
  while(1)
    pause();
  exit(0);
}
