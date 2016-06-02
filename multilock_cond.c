/* Zamykání nekolika systémových zdroju soucasne: N vláken a N zdroju,
 * pro operaci op() potrebuje vlákno I zdroje I-1, I, I+1 (mod N) k zamykání
 * se pouzívají podmínkové promenné. */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>

#define OP_TIME 1 /* Trvání op() */

int n; /* Pocet threadu */

pthread_rwlock_t outlock=PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
int *resources;

void out(char *msg,...)
{
  va_list args;
  va_start(args,msg);
  if((errno=pthread_rwlock_wrlock(&outlock))) {
    perror("pthread_rwlock_wrlock()");
    exit(1);
  }
  vprintf(msg,args);
  if((errno=pthread_rwlock_unlock(&outlock))) {
    perror("pthread_rwlock_unlock()");
    exit(1);
  }
  va_end(args);
}

void op(int i)
{
  out("%d op() started\n",i);
  sleep(OP_TIME);
  out("%d op() finished\n",i);
}

void *thread(void *arg)
{
  int i=*(int*)arg;
  out("%d getting resources [%d,%d,%d]\n",i,(i-1+n)%n,i,(i+1)%n);
  if((errno=pthread_mutex_lock(&mutex))) {
    perror("pthread_mutex_lock()");
    exit(1);
  }
  while(resources[(i-1+n)%n]==0||resources[i]==0||resources[(i+1)%n]==0)
    if((errno=pthread_cond_wait(&cond,&mutex))) {
      perror("pthread_cond_wait()");
      exit(1);
    }
  resources[(i-1+n)%n]=resources[i]=resources[(i+1)%n]=0;
  if((errno=pthread_mutex_unlock(&mutex))) {
    perror("pthread_mutex_unlock()");
    exit(1);
  }
  out("%d got resources [%d,%d,%d]\n",i,(i-1+n)%n,i,(i+1)%n);
  op(i);
  out("%d releasing resources [%d,%d,%d]\n",i,(i-1+n)%n,i,(i+1)%n);
  if((errno=pthread_mutex_lock(&mutex))) {
    perror("pthread_mutex_lock()");
    exit(1);
  }
  resources[(i-1+n)%n]=resources[i]=resources[(i+1)%n]=1;
  if((errno=pthread_cond_broadcast(&cond))) {
    perror("pthread_cond_broadcast()");
    exit(1);
  }
  if((errno=pthread_mutex_unlock(&mutex))) {
    perror("pthread_mutex_unlock()");
    exit(1);
  }
  out("%d released resources [%d,%d,%d]\n",i,(i-1+n)%n,i,(i+1)%n);
  return NULL;
}

int main(int argc,char *argv[])
{
  int i;
  pthread_t *threads;
  int *args;
  if(argc!=2||(n=atoi(argv[1]))<=2) {
    fprintf(stderr,"usage: %s n_threads\n",argv[0]);
    exit(1);
  }
  if(!(threads=malloc(n*sizeof(pthread_t)))) {
    perror("malloc()");
    exit(1);
  }
  if(!(args=malloc(n*sizeof(int)))) {
    perror("malloc()");
    exit(1);
  }
  if(!(resources=malloc(n*sizeof(int)))) {
    perror("malloc()");
    exit(1);
  }
  for(i=0;i<n;i++)
    resources[i]=1;
  for(i=0;i<n;i++) {
    args[i]=i;
    if((errno=pthread_create(threads+i,NULL,thread,(void*)(args+i)))) {
      perror("pthread_create()");
      exit(1);
    }
  }
  for(i=0;i<n;i++)
    if((errno=pthread_join(threads[i],NULL))) {
      perror("pthread_join()");
      exit(1);
    }
  exit(0);
}
