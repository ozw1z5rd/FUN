/* System V IPC - sdílená pamet a semafory, proces cte sdílenou pamet a píse 
   na výstup. K synchronizaci se pouzívají semafory. */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define ID 123
#define KEY 1234
#define BUFSZ 64

int shm_id=-1,sem_id=-1;
int init_ok=0;

struct shmem {
  int sz;
  char buf[BUFSZ];
};

struct shmem *pshmem=NULL;

void rm_ipc()
{
  if(shm_id!=-1&&(shmdt(pshmem)==-1||shmctl(shm_id,IPC_RMID,NULL)==-1))
    perror("Removing shared memory in atexit()");
  if(sem_id!=-1&&semctl(sem_id,0,IPC_RMID)==-1)
    perror("Removing semaphores in atexit()");
}

int main(int argc,char *argv[])
{
  key_t key=KEY;
  struct sembuf sops;
  atexit(rm_ipc);
  if(argc==2&&(key=ftok(argv[1],ID))==-1) {
    perror("ftok()");
    exit(1);
  }
  if((shm_id=shmget(key,0,0))==-1) {
    perror("shmget()");
    exit(1);
  }
  if((pshmem=(struct shmem *)shmat(shm_id,NULL,0))==(void *)-1) {
    perror("shmat()");
    exit(1);
  }
  if((sem_id=semget(key,2,0))==-1) {
    perror("semget()");
    exit(1);
  }
  for(;;) {
    sops.sem_num=0;
    sops.sem_op=-1;
    sops.sem_flg=0;
    if(semop(sem_id,&sops,1)==-1) {
      perror("semop()");
      exit(1);
    }
    if(pshmem->sz==0)
      exit(0);
    if(write(1,"[",1)==-1||write(1,pshmem->buf,pshmem->sz)==-1||
	write(1,"]",1)==-1) {
      perror("write()");
      exit(1);
    }
    sops.sem_num=1;
    sops.sem_op=1;
    sops.sem_flg=0;
    if(semop(sem_id,&sops,1)==-1) {
      perror("semop()");
      exit(1);
    }
  }
  exit(0);
}
