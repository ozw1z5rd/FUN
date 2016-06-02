/* Osetrení signálu, ukázka mozných problému pri spustení handleru signálu
   pred ukoncením predchozího handleru (nastává, kdyz je poslední argument 0,
   tj. uvnitr handleru nejsou signály blokované): 
   1) Kdyz prijde SIGALRM na míste (1), handler_alrm() sice vynuluje
   counter_int, ale na rádku za (1) se hodnota obnoví, takze nekteré signály
   se zapocítají vícekrát.
   2) Kdyz prijde SIGINT na míste (2), na následujícím rádku se ztratí hodnota
   prictená do counter_int a nekteré signály se tedy nezapocítají.
   3) Kdyz prijde SIGQUIT na míste (3), counter_int se zapocítá do výsledné
   hodnoty counter dvakrát, protoze ho jeste funkce handler_alrm()
   nevynulovala. */

#define _XOPEN_SOURCE 500 /* Pro Linux, aby se deklarovalo sigrelse()
			     v signal.h */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <setjmp.h>

#define SLEEP_QUIT 2000

int counter_int,counter_no_reset,counter;
sigjmp_buf j_buf;
int sleep_int,sleep_alrm,period_alrm,block;

void do_sleep(int msec)
{
  struct timespec st,rst;
  if(msec<=0)
    return;
  st.tv_nsec=(msec%1000)*1000000;
  st.tv_sec=msec/1000;
  while(nanosleep(&st,&rst)==-1) {
    perror("nanosleep()");
    if(errno!=EINTR)
      exit(1);
    else
      st=rst;
  }
}

void handler_int(int sig)
{
  int aux;
  printf("SIGINT caught, counter_int=%d, counter=%d, counter_no_reset=%d\n",
      counter_int,counter,counter_no_reset);
  counter_no_reset++;
  aux=counter_int;
  aux++;
  do_sleep(sleep_int); /* (1) */
  counter_int=aux;
  printf("SIGINT handled, counter_int=%d, counter=%d, counter_no_reset=%d\n",
      counter_int,counter,counter_no_reset);
}

void handler_alrm(int sig)
{
  printf("SIGALRM caught, counter_int=%d, counter=%d, counter_no_reset=%d\n",
      counter_int,counter,counter_no_reset);
  counter+=counter_int;
  do_sleep(sleep_alrm); /* (2) (3) */
  counter_int=0;
  printf("SIGALRM handled, counter_int=%d, counter=%d, counter_no_reset=%d\n",
      counter_int,counter,counter_no_reset);
  alarm(period_alrm);
}

void handler_quit_quit(int sig)
{
  printf("EXIT\n");
  exit(0);
}

void handler_quit(int sig)
{
  struct sigaction act;
  counter+=counter_int;
  counter_int=0;
  printf("SIGQUIT caught, counter_int=%d, counter=%d, counter_no_reset=%d\n",
      counter_int,counter,counter_no_reset);
  printf("Send SIGQUIT again to exit, or wait for restart\n");
/*  do_sleep(SLEEP_QUIT);
  printf("Setting SIGQUIT to exit\n"); */
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask,SIGINT);
  sigaddset(&act.sa_mask,SIGALRM);
  act.sa_flags=SA_RESTART;
  act.sa_handler=handler_quit_quit;
  sigaction(SIGQUIT,&act,NULL);
  sigrelse(SIGQUIT);
  do_sleep(SLEEP_QUIT);
  printf("LONGJMP\n");
  siglongjmp(j_buf,1);
}

int main(int argc,char *argv[])
{
  struct sigaction act;
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask,SIGINT);
  sigaddset(&mask,SIGALRM);
  sigaddset(&mask,SIGQUIT);
  sigprocmask(SIG_BLOCK,&mask,NULL);
  if(argc!=5) {
    fprintf(stderr,
"usage: sigaction sleep_int(ms) sleep_alrm(s) period_alrm(ms) {0|1}\n");
    exit(1);
  }
  sleep_int=atoi(argv[1]);
  sleep_alrm=atoi(argv[2]);
  period_alrm=atoi(argv[3]);
  block=atoi(argv[4]);
  if(sigsetjmp(j_buf,1))
    printf("RESTART\n");
  else
    printf("BEGIN\n");
  counter=counter_int=counter_no_reset=0;
  sigemptyset(&act.sa_mask);
  if(block) {
    sigaddset(&act.sa_mask,SIGINT);
    sigaddset(&act.sa_mask,SIGALRM);
    sigaddset(&act.sa_mask,SIGQUIT);
  }
  act.sa_flags=SA_RESTART;
  act.sa_handler=handler_int;
  sigaction(SIGINT,&act,NULL);
  act.sa_handler=handler_alrm;
  sigaction(SIGALRM,&act,NULL);
  act.sa_handler=handler_quit;
  /* act.sa_flags|=SA_NODEFER; */ /* Tohle by zpusobilo, ze rychle opakovaný
				     SIGQUIT by byl zpracován dríve, nez se
				     zmení handler (tj. starým handlerem) a
				     program by neskoncil */
  sigaddset(&act.sa_mask,SIGINT);
  sigaddset(&act.sa_mask,SIGALRM);
  sigaction(SIGQUIT,&act,NULL);
  alarm(period_alrm);
  sigrelse(SIGINT);
  sigrelse(SIGALRM);
  sigrelse(SIGQUIT);
  for(;;)
    pause();
  exit(1);
}
