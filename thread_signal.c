/* Synchronizace threadu producent - konzument pomocí signálu */

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#define RUNTIME 30
#define GO_SIG SIGUSR1

int shared; /* Tuto promennou budou thready sdílet */
pthread_t prod,cons;

void *producer(void *par)
{
  struct timespec ts;
  int old,sig;
  sigset_t set;
  ts.tv_sec=0;
  
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old);
  sigemptyset(&set);
  sigaddset(&set,GO_SIG);
  for(;;) {
    (*((int*)par))++;
    ts.tv_nsec=mrand48();
    pthread_kill(cons,GO_SIG);
    nanosleep(&ts,NULL);
    sigwait(&set,&sig);
  }
  return NULL;
}

void *consumer(void *par)
{
  int old,sig;
  sigset_t set;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old);
  sigemptyset(&set);
  sigaddset(&set,GO_SIG);
  for(;;) {
    sigwait(&set,&sig);
    printf("%d\n",*((int*)par));
    pthread_kill(prod,GO_SIG);
  }
  return NULL;
}

int main()
{
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set,GO_SIG);
  sigprocmask(SIG_BLOCK,&set,NULL);
  if(pthread_create(&cons,NULL,consumer,&shared)) {
    fprintf(stderr,"Cannot create consumer thread\n");
    exit(1);
  }
  if(pthread_create(&prod,NULL,producer,&shared)) {
    fprintf(stderr,"Cannot create producer thread\n");
    exit(1);
  }
  sleep(RUNTIME);
  if(pthread_cancel(prod)) {
    fprintf(stderr,"Cannot cancel producer thread\n");
    exit(1);
  }
  if(pthread_cancel(cons)) {
    fprintf(stderr,"Cannot cancel consumer thread\n");
    exit(1);
  }
  exit(0);
}
