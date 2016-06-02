/* Program pro interaktivní experimenty s alokací pameti */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>

#define INIT_VAL 123

struct mem_block {
  int size;
  void *data;
  struct mem_block *next;
};

struct mem_block *mem=NULL;

int main()
{
  char buf[1024];
  int sz;
  struct mem_block *p;
  for(;;) {
    printf("Size (>0 alloc, <0 alloc&init, =0 init all): ");
    if(!gets(buf))
      break;
    sz=atoi(buf);
    if(sz) {
      printf("Allocating %d bytes of memory...",abs(sz));
      fflush(stdout);
      if(!(p=malloc(sizeof(struct mem_block)))) {
	perror("malloc(mem_block)");
	exit(1);
      }
      p->size=abs(sz);
      p->next=mem;
      mem=p;
      if(!(p->data=malloc(abs(sz)))) {
	perror("malloc(data)");
	exit(1);
      }
      printf(" OK\n");
      if(sz<0) {
	printf("Initializing %d bytes of memory...",abs(sz));
	fflush(stdout);
	memset(p->data,INIT_VAL,abs(sz));
	printf(" OK\n");
      }
    } else {
      p=mem;
      while(p) {
	printf("Initializing %d bytes of memory...",p->size);
	fflush(stdout);
	memset(p->data,INIT_VAL,p->size);
	printf(" OK\n");
	p=p->next;
      }
    }
  }
  exit(0);
}
