/* Zjisten� jm�na, kter� m� adres�r v rodicovsk�m adres�ri. Tento k�d je
 * z�kladem pro implementaci funkce getcwd(), resp. pr�kazu pwd. */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>

int main(int argc,char **argv)
{
    struct stat this_dir,parent_dir,subdir;
    DIR *d;
    struct dirent *de;
    int no_stat=0,with_stat=0;
    char *p;
    if(stat(".",&this_dir)==-1) {
	perror("stat(\".\")");
	exit(1);
    }
    if(stat("..",&parent_dir)==-1) {
	perror("stat(\"..\")");
	exit(1);
    }
    if(this_dir.st_dev==parent_dir.st_dev&&
	this_dir.st_ino==parent_dir.st_ino) { /* Koren odkazuje na sebe */
	printf("Odkaz \"..\" ukazuje na sebe: /\n");
	exit(0);
    }
    if(!(d=opendir(".."))) {
	perror("opendir(\"..\")");
	exit(1);
    }
    errno=0;
    while((de=readdir(d))) {
	if(de->d_ino==this_dir.st_ino) { /* Toto funguje, pokud aktu�ln�
					    adres�r nen� mount-point */
	    printf("Bez pouziti stat(): %s\n",de->d_name);
	    no_stat=1;
	}
	/* Jestlize je aktu�ln� adres�r mount-point, m� jeho rodic jin� c�slo
	 * zar�zen� a nestac� porovn�vat c�slo i-uzlu aku�ln�ho adres�re s
	 * c�sly i-uzlu polozek rodicovsk�ho adres�re. Je treba pro kazdou
	 * polozku rodice zavolat stat(), abychom se dostali na stejn�
	 * zar�zen� a mohli porovn�vat c�sla i-uzlu. */
	if(!(p=malloc(4+strlen(de->d_name)))) {
	    perror("malloc()");
	    exit(1);
	}
	strcat(strcpy(p,"../"),de->d_name);
	if(lstat(p,&subdir)==-1) {
	    fprintf(stderr,"stat(\"%s\"): %s\n",p,strerror(errno));
	    exit(1);
	}
	if(this_dir.st_dev==subdir.st_dev&&this_dir.st_ino==subdir.st_ino) {
	    printf("S pouzitim stat(): %s\n",de->d_name);
	    with_stat=1;
	}
	/* Pro adres�r, kter� nen� mount-point, funguj� obe metody (bez i s
	 * vyuzit�m stat()). */
    }
    if(errno) {
	perror("readdir()");
	exit(1);
    }
    if(closedir(d)==-1) {
	perror("closedir()");
	exit(1);
    }
    if(!no_stat)
	printf("NENALEZENO bez pouziti stat()\n");
    if(!with_stat)
	printf("NENALEZENO s pouzitim stat()\n");
    exit(0);
}
