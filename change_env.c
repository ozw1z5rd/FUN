/* Manipulace s promennými prostredí
 * Argumenty tvaru 'jméno=hodnota' pridá do prostredí (nebo zmení hodnotu
 * promenné, pokud uz existovala), ostatní argumenty povazuje za jména
 * promenných, které odstraní z prostredí.
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
      unsetenv(argv[i]); /* Funkce z BSD 4.3, která není v UNIX98 */
    }
  }
  for(i=0;environ[i];i++)
    printf("%s\n",environ[i]);
  exit(0);
}
