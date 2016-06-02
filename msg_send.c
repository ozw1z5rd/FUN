/* System V IPC - fronty zpráv, proces cte vstup a posílá ho jako zprávy do
   fronty. */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define ID 123
#define MSQ_KEY 1234
#define BUFSZ 64
#define MSG_DATA 1
#define MSG_EOF 2

int id;

void send_eof()
{
  long int meof=MSG_EOF;
  if(msgsnd(id,&meof,0,0)==-1)
    perror("msgsnd() in atexit()");
}

int main(int argc,char *argv[])
{
  int sz;
  key_t key=MSQ_KEY;
  struct {
    long int mtype;
    char buf[BUFSZ];
  } msg;
  if(argc==2&&(key=ftok(argv[1],ID))==-1) {
    perror("ftok()");
    exit(1);
  }
  if((id=msgget(key,IPC_CREAT|0600))==-1) {
    perror("msgget()");
    exit(1);
  }
  atexit(send_eof);
  msg.mtype=MSG_DATA;
  while((sz=read(0,msg.buf,BUFSZ))>0)
    if(msgsnd(id,&msg,sz,0)==-1) {
      perror("msgsnd()");
      exit(1);
    }
  if(sz==-1) {
    perror("read()");
    exit(1);
  }
  exit(0);
}
