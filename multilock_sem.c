/* Zamykání nekolika systémových zdroju soucasne: N vláken a N zdroju,
 * pro operaci op() potrebuje vlákno I zdroje I-1, I, I+1 (mod N) k zamykání
 * se pouzívají semafory, prevenci deadlocku zajistuje atomicita volání
 * semop() */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/sem.h>
#include <pthread.h>

#define OP_TIME 1 /* Trvání op() */

union semun { /* Semaforové operace */
  int val;
  struct semid_ds *buf;
  unsigned short  *array;
};

int n; /* Pocet threadu */
int sem; /* Identifikátor pole semaforu */

pthread_rwlock_t outlock=PTHREAD_RWLOCK_INITIALIZER;

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
  struct sembuf sops[3];
  out("%d getting resources [%d,%d,%d]\n",i,(i-1+n)%n,i,(i+1)%n);
  sops[0].sem_num=(i-1+n)%n;
  sops[1].sem_num=i;
  sops[2].sem_num=(i+1)%n;
  sops[0].sem_op=sops[1].sem_op=sops[2].sem_op=-1;
  sops[0].sem_flg=sops[1].sem_flg=sops[2].sem_flg=0;
  if(semop(sem,sops,3)==-1) {
    perror("semop()");
    exit(1);
  }
  out("%d got resources [%d,%d,%d]\n",i,(i-1+n)%n,i,(i+1)%n);
  op(i);
  out("%d releasing resources [%d,%d,%d]\n",i,(i-1+n)%n,i,(i+1)%n);
  sops[0].sem_op=sops[1].sem_op=sops[2].sem_op=1;
  if(semop(sem,sops,3)==-1) {
    perror("semop()");
    exit(1);
  }
  out("%d released resources [%d,%d,%d]\n",i,(i-1+n)%n,i,(i+1)%n);
  return NULL;
}

int main(int argc,char *argv[])
{
  int i;
  union semun sarg;
  pthread_t *threads;
  int *args;
  if(argc!=2||(n=atoi(argv[1]))<=2) {
    fprintf(stderr,"usage: %s n_threads\n",argv[0]);
    exit(1);
  }
  if((sem=semget(IPC_PRIVATE,n,0700))==-1) {
    perror("semget()");
    exit(1);
  }
  sarg.val=1;
  for(i=0;i<n;i++)
    if(semctl(sem,i,SETVAL,sarg)==-1) {
      perror("semctl()");
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
  if(semctl(sem,0,IPC_RMID)==-1) {
    perror("semctl()");
    exit(1);
  }
  exit(0);
}
