/* Natazení dynamické sdílené knihovny za behu programu */

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

int main(int argc,char *argv[])
{
  char *err,*libname,*fun_name;
  void *handle;
  int n;
  int (*fun)(int);
  if(argc!=4) {
    fprintf(stderr,"usage: load_so number library function\n");
    exit(1);
  }
  n=atoi(argv[1]);
  libname=argv[2];
  fun_name=argv[3];
  printf("Loading library %s\n",libname);
  if(!(handle=dlopen(libname,RTLD_NOW))) {
    fprintf(stderr,"%s\n",dlerror());
    exit(1);
  }
  printf("Obtaining address of symbol %s\n",fun_name);
  fun=dlsym(handle,fun_name);
  if((err=dlerror())) {
    fprintf(stderr,"%s\n",err);
    exit(1);
  }
  printf("Calling %p\n",fun);
  printf("%s(%d)=%d\n",fun_name,n,fun(n));
  printf("Closing library %s (%p)\n",libname,handle);
  dlclose(handle);
  exit(0);
}
