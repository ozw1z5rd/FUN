/* Informace, které muze proces získat o sobe */

#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#define __USE_XOPEN_EXTENDED /* Kvuli deklaraci getsid() v Linuxu */
#include <unistd.h>

int main()
{
  char buf[PATH_MAX];
  struct group *grp;
  struct passwd *user;
  char *str;
  gid_t *gids;
  int i,n;
  printf("Process ID = %ld\n",(long)getpid());
  printf("Process group ID = %ld\n",(long)getpgrp());
  printf("Parent process ID = %ld\n",(long)getppid());
  printf("Process group ID of session leader = %ld\n",(long)getsid(0));
  if(!getcwd(buf,PATH_MAX))
    perror("getcwd()");
  else
    printf("Working directory = \"%s\"\n",buf);
  printf("User name = \"%s\"\n",getlogin());
  str= (user=getpwuid(getuid())) ? user->pw_name : "";
  printf("Real user ID = %ld (%s)\n",(long)getuid(),str);
  str= (user=getpwuid(geteuid())) ? user->pw_name : "";
  printf("Effective user ID = %ld (%s)\n",(long)geteuid(),str);
  str= (grp=getgrgid(getgid())) ? grp->gr_name : "";
  printf("Real group ID = %ld (%s)\n",(long)getgid(),str);
  str= (grp=getgrgid(getegid())) ? grp->gr_name : "";
  printf("Effective group ID = %ld (%s)\n",(long)getegid(),str);
  if((n=getgroups(0,NULL))>0) {
    if(!(gids=malloc(sizeof(gid_t)*n))) {
      perror("malloc()");
      exit(1);
    }
    getgroups(n,gids);
    printf("Supplementary groups = ");
    for(i=0;i<n;i++) {
      str= (grp=getgrgid(gids[i])) ? grp->gr_name : "";
      printf("%ld (%s)%s",(long)gids[i],str,i==n-1?"\n":", ");
    }
  }
  exit(0);
}
