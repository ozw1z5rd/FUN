/* Alokace co nejvetsího úseku pameti */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MIN 1024
#define INIT_VAL 123

int main()
{
  void *p;
  int asize=MIN,total=0;
  while(asize>=MIN) {
    printf("Allocating %d ... ",asize);
    if((p=malloc(asize))) {
      printf("initializing ... ");
      memset(p,INIT_VAL,asize);
      total+=asize;
      printf("total %d\n",total);
      asize*=2;
    } else {
      printf("failed\n");
      asize/=2;
    }
  }
  printf("All memory allocated\n");
  sleep(60);
  exit(0);
}
