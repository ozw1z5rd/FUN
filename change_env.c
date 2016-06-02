/* Manipulace s promenn�mi prostred�
 * Argumenty tvaru 'jm�no=hodnota' prid� do prostred� (nebo zmen� hodnotu
 * promenn�, pokud uz existovala), ostatn� argumenty povazuje za jm�na
 * promenn�ch, kter� odstran� z prostred�.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

extern char **environ;

int main(int argc,char *argv[])
{
  int i;
  for(i=1;i<argc;i++) {
    if(strchr(argv[i],'=')) { /* Pridat promennou */
      printf("putenv(\"%s\")\n",argv[i]);
      putenv(argv[i]);
    } else {
      printf("unsetenv(\"%s\")\n",argv[i]);
      unsetenv(argv[i]); /* Funkce z BSD 4.3, kter� nen� v UNIX98 */
    }
  }
  for(i=0;environ[i];i++)
    printf("%s\n",environ[i]);
  exit(0);
}
