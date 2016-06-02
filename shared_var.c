/* Ukázka konfliktu pri sdílení promenné více thready, které nad ní delají
   neatomické operace */

#include <stdio.h>
#include <pthread.h>

struct {
  int a,b;
} shared_variable={0,0};

void *update(void *arg)
{
  int i,val,a,b;
  val=*((int *)arg);
  for(i=1;1;i++) {
    a=shared_variable.a;
    b=shared_variable.b;
    if(a!=b)
      printf("Inconsistency (%d,%d) detected in thread %d, iteration %i\n",
	  a,b,val,i);
    shared_variable.a=val;
    shared_variable.b=val;
  }
  return NULL;
}

int main()
{
  pthread_t id;
  int arg1=1,arg2=2;
  pthread_create(&id,NULL,update,&arg1);
  update(&arg2);
  return 0;
}
