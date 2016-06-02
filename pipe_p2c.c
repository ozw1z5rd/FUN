/* Roura od otce k synovi. Probl�m: otec skonc�, shell vyp�se prompt, ale pak
   se vyp�se zbytek v�stupu syna. Mozn� resen� je cekat na syna, ale to nelze
   snadno udelat, kdyz otec provede exec a spust� nejak� program, kter� na sv�
   deti necek� (napr. nejakou beznou utilitu). Proto je leps� stavet rouru
   opacne (od syna k otci, viz pipe_p2c.c). */

#include <stddef.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <ctype.h>

#define PBUFSZ 4096
#define CBUFSZ 1

void producer()
{
  char buf[PBUFSZ];
  int sz;
  while((sz=read(0,buf,PBUFSZ))>0)
    if(write(1,buf,sz)!=sz) {
      perror("producer write()");
      exit(1);
    }
  if(sz==-1) {
    perror("producer read()");
    exit(1);
  }
}

void consumer()
{
  char buf[CBUFSZ];
  int sz,i;
  while((sz=read(0,buf,CBUFSZ))>0) {
    for(i=0;i<sz;i++)
      buf[i]=toupper(buf[i]);
    if(write(1,buf,sz)!=sz) {
      perror("producer write()");
      exit(1);
    }
    sleep(1); /* Konzument (syn) pracuje rychlost� 1 znak za sekundu, aby byl
		 dobre videt efekt jeho dobehu po skoncen� otce */
  }
  if(sz==-1) {
    perror("producer read()");
    exit(1);
  }
}

int main()
{
  int pd[2];
  if(pipe(pd)==-1) {
    perror("pipe()");
    exit(1);
  }
  switch(fork()) {
    case -1:
      perror("fork()");
      exit(1);
      break;
    case 0: /* Syn (cte z roury) */
      close(0); /* close(), dup() lze ekvivalentne ps�t dup2(pd[0],0) */
      if(dup(pd[0])==-1) {
	perror("dup()");
	exit(1);
      }
      close(pd[0]);
      close(pd[1]); /* Konzument mus� zavr�t z�pisov� konec roury, protoze
		       dokud m� nejak� proces rouru otevrenou pro z�pis,
		       nenastane na n� EOF a konzument by cekal vecne i po
		       skoncen� producenta!!! */
      consumer();
      break;
    default: /* Otec (zapisuje do roury) */
      if(dup2(pd[1],1)==-1) { /* ekvivalentn� close(1);dup(pd[1]) */
	perror("dup2()");
	exit(1);
      }
      close(pd[0]);
      close(pd[1]);
      producer();
      /* Tady by mohl b�t wait(), kter�m by otec pockal, nez syn dokonc�
         zpracov�n�. Toto resen� ale nelze pouz�t, kdyz otec m�sto vol�n�
	 funkce producer() provede exec(). */
      break;
  }
  exit(0);
}
