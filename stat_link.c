/* Získání informací o souboru, procházení symlinku */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h> /* Linux: dev_t -> ( major, minor ) */
#include <unistd.h>
#include <limits.h>

/* Zpracuje výstup lstat(), stat(), vrací -1 po chybe, jinak 0 */

int info_buf(const char *lpath,struct stat *buf)
{
  struct stat lbuf;
  char path[PATH_MAX+1];
  int n;
  if(S_ISREG(buf->st_mode))
    printf("FILE %ld bytes\n",(long int)buf->st_size);
  else
    if(S_ISDIR(buf->st_mode))
      printf("DIRECTORY %ld bytes\n",(long int)buf->st_size);
    else
      if(S_ISCHR(buf->st_mode))
	printf("CHARACTER DEVICE major=%d, minor=%d\n",
	    (int)major(buf->st_rdev),(int)minor(buf->st_rdev));
      else
	if(S_ISBLK(buf->st_mode))
	  printf("BLOCK DEVICE major=%d, minor=%d\n",(int)major(buf->st_rdev),
	      (int)minor(buf->st_rdev));
	else
	  if(S_ISFIFO(buf->st_mode))
	    printf("FIFO\n");
	  else
	    if(S_ISSOCK(buf->st_mode))
	      printf("SOCKET\n");
	    else
	      if(S_ISLNK(buf->st_mode)) {
		printf("SYMLINK to ");
		if((n=readlink(lpath,path,PATH_MAX))==-1) {
		  printf("\n");
		  perror(lpath);
		  return -1;
		} else /* readlink() nezapisuje '\0' na konec retezce !!! */
		  path[n]='\0';
		printf("%s ",path);
		if(stat(lpath,&lbuf)==-1) {
		  printf("\n");
		  perror(lpath);
		  return -1;
		}
		if(info_buf(path,&lbuf)==-1)
		  return -1;
	      }
	      else
		printf("UNKNOWN\n");
  return 0;
}

int main(int argc,char *argv[])
{
  int i;
  struct stat buf;
  char *arg;
  if(argc<2) {
    fprintf(stderr,"usage: stat_link file ...\n");
    exit(1);
  }
  for(i=1;i<argc;i++) {
    arg=argv[i];
    if(lstat(arg,&buf)==-1) {
      perror(arg);
      continue;
    }
    printf("%s ",arg);
    info_buf(arg,&buf);
  }
  exit(0);
}
