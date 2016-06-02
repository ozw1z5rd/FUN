/* Triviální príklad pouzití POSIXových threadu */

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define CNT 100

void *thread_body(void *str)
{
  static int res;
  struct timespec ts;
  int i;
  printf("Thread \"%s\" started\n",(char *)str);
  ts.tv_sec=0;
  ts.tv_nsec=10000000;
  for(i=0;i<CNT;i++) {
    fputs((char *)str,stdout);
    nanosleep(&ts,NULL);
  }
  res=123;
  return &res;
}

int main()
{
  pthread_t ta,tb,tc,td;
  void *res;
  setbuf(stdout,NULL);
  if((errno=pthread_create(&ta,NULL,thread_body,"a"))) {
    perror("pthread_create()");
    exit(1);
  }
  if((errno=pthread_create(&tb,NULL,thread_body,"b"))) {
    perror("pthread_create()");
    exit(1);
  }
  if((errno=pthread_create(&tc,NULL,thread_body,"c"))) {
    perror("pthread_create()");
    exit(1);
  }
  if((errno=pthread_create(&td,NULL,thread_body,"d"))) {
    perror("pthread_create()");
    exit(1);
  }
  if((errno=pthread_join(ta,&res))) {
    perror("ptread_join()");
    exit(1);
  }
  printf("(Thread A returned %d)",*((int*)res));
  if((errno=pthread_join(tb,&res))) {
    perror("ptread_join()");
    exit(1);
  }
  printf("(Thread B returned %d)",*((int*)res));
  if((errno=pthread_join(tc,&res))) {
    perror("ptread_join()");
    exit(1);
  }
  printf("(Thread C returned %d)",*((int*)res));
  if((errno=pthread_join(td,&res))) {
    perror("ptread_join()");
    exit(1);
  }
  printf("(Thread D returned %d)",*((int*)res));
  exit(0);
}
