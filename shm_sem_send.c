/* System V IPC - sdílená pamet a semafory, proces cte vstup a predává ho 
   druhému procesu pres sdílenou pamet. K synchronizaci se pouzívají
   semafory. */

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

void send_eof()
{
  struct sembuf sops;
  if(!init_ok) {
    if(shm_id!=-1&&(shmdt(pshmem)==-1||shmctl(shm_id,IPC_RMID,NULL)==-1))
      perror("Removing shared memory in atexit()");
    if(sem_id!=-1&&semctl(sem_id,0,IPC_RMID)==-1)
      perror("Removing semaphores in atexit()");
  } else {
    sops.sem_num=1;
    sops.sem_op=-1;
    sops.sem_flg=0;
    if(semop(sem_id,&sops,1)==-1)
      perror("semop() in atexit()");
    pshmem->sz=0;
    sops.sem_num=0;
    sops.sem_op=1;
    sops.sem_flg=0;
    if(semop(sem_id,&sops,1)==-1)
      perror("semop() in atexit()");
  }
}

int main(int argc,char *argv[])
{
  key_t key=KEY;
  union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
  } sem_arg;
  struct sembuf sops;
  if(argc==2&&(key=ftok(argv[1],ID))==-1) {
    perror("ftok()");
    exit(1);
  }
  if((shm_id=shmget(key,sizeof(struct shmem),IPC_CREAT|0600))==-1) {
    perror("shmget()");
    exit(1);
  }
  atexit(send_eof);
  if((pshmem=(struct shmem *)shmat(shm_id,NULL,0))==(void *)-1) {
    perror("shmat()");
    exit(1);
  }
  if((sem_id=semget(key,2,IPC_CREAT|0600))==-1) {
    perror("semget()");
    exit(1);
  }
  sem_arg.val=0;
  if(semctl(sem_id,0,SETVAL,sem_arg)==-1) {
    perror("semctl()");
    exit(1);
  }
  sem_arg.val=1;
  if(semctl(sem_id,1,SETVAL,sem_arg)==-1) {
    perror("semctl()");
    exit(1);
  }
  init_ok=1;
  for(;;) {
    sops.sem_num=1;
    sops.sem_op=-1;
    sops.sem_flg=0;
    if(semop(sem_id,&sops,1)==-1) {
      perror("semop()");
      exit(1);
    }
    if((pshmem->sz=read(0,pshmem->buf,BUFSZ))<=0)
      break;
    sops.sem_num=0;
    sops.sem_op=1;
    sops.sem_flg=0;
    if(semop(sem_id,&sops,1)==-1) {
      perror("semop()");
      exit(1);
    }
  }
  if(pshmem->sz==-1) {
    perror("read()");
    exit(1);
  }
  sops.sem_num=1;
  sops.sem_op=1;
  sops.sem_flg=0;
  if(semop(sem_id,&sops,1)==-1) {
    perror("semop()");
    exit(1);
  }
  exit(0);
}
