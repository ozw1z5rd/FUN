/* System V IPC - fronty zpráv, proces cte zprávy a píse jejich obsah na
   výstup. */

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

void rm_msq()
{
  if(msgctl(id,IPC_RMID,NULL)==-1)
    perror("Removing message queue in atexit()");
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
  if((id=msgget(key,0))==-1) {
    perror("msgget()");
    exit(1);
  }
  atexit(rm_msq);
  while((sz=msgrcv(id,&msg,BUFSZ,0,0))>=0)
    if(msg.mtype==MSG_DATA) {
      if(write(1,"[",1)==-1||write(1,msg.buf,sz)==-1||write(1,"]",1)==-1) {
	perror("write()");
	exit(1);
      }
    } else /* msg.mtype==MSG_EOF */
      exit(0);
  perror("msgrcv()");
  exit(1); 
}
