/* V�pis promenn�ch prostred� */

#include <stdio.h>

extern char **environ;

int main(int argc,char *argv[])
{
  int i;
  for(i=0;environ[i];i++)
    printf("%s\n",environ[i]);
  exit(0);
}
