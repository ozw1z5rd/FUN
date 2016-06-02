/* Príklad synchronizace threadu producent - konzument pomocí mutexu a
   podmínkových promenných (condition variable) */

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define RUNTIME 30

int shared; /* Tuto promennou budou thready sdílet */

struct wait_here {
pthread_cond_t cond;
pthread_mutex_t mutex;
int go;
};

struct wait_here wait_w,wait_r;

void *producer(void *par)
{
  struct timespec ts;
  int old;
  ts.tv_sec=0;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old);
  for(;;) {
    pthread_mutex_lock(&wait_w.mutex);
    while(!wait_w.go) {
      pthread_cond_wait(&wait_w.cond,&wait_w.mutex);
    }
    wait_w.go=0;
    pthread_mutex_unlock(&wait_w.mutex);
    (*((int*)par))++;
    ts.tv_nsec=mrand48();
    pthread_mutex_lock(&wait_r.mutex);
    wait_r.go=1;
    pthread_cond_signal(&wait_r.cond);
    pthread_mutex_unlock(&wait_r.mutex);
    nanosleep(&ts,NULL);
  }
  return NULL;
}

void *consumer(void *par)
{
  int old;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old);
  for(;;) {
    pthread_mutex_lock(&wait_r.mutex);
    while(!wait_r.go) {
      pthread_cond_wait(&wait_r.cond,&wait_r.mutex);
    }
    wait_r.go=0;
    pthread_mutex_unlock(&wait_r.mutex);
    printf("%d\n",*((int*)par));
    pthread_mutex_lock(&wait_w.mutex);
    wait_w.go=1;
    pthread_cond_signal(&wait_w.cond);
    pthread_mutex_unlock(&wait_w.mutex);
  }
  return NULL;
}

int main()
{
  pthread_t prod,cons;
  pthread_mutex_init(&wait_w.mutex,NULL);
  pthread_mutex_init(&wait_r.mutex,NULL);
  pthread_cond_init(&wait_w.cond,NULL);
  pthread_cond_init(&wait_r.cond,NULL);
  wait_w.go=1;
  wait_r.go=0;
  if(pthread_create(&prod,NULL,producer,&shared)) {
    fprintf(stderr,"Cannot create producer thread\n");
    exit(1);
  }
  if(pthread_create(&cons,NULL,consumer,&shared)) {
    fprintf(stderr,"Cannot create consumer thread\n");
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
