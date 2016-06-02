/* Oboustranná komunikace mezi dvema procesy pomocí dvou opacne orientovaných
   rour. Jeden proces generuje data a po blocích je odesílá druhému, který je
   zpracovává (po jednotlivých bajtech) a posílá zpet. Demonstrace deadlocku
   pri naplnení bufferu rour. Velikost bloku a pocet bloku se zadávají jako
   parametry. Odstranení deadlocku pomocí select() je v pipe_select.c. */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void parent(int wfd,int rfd,int bs,int nb)
{
    char *buf;
    int i,n1,n2;
    if(!(buf=malloc(bs))) {
	perror("malloc()");
	exit(1);
    }
    for(i=0;i<nb;i++) { /* Opakuj pro nb bloku */
	n1=bs;
	printf("(");
	fflush(stdout);
	for(n1=bs;n1>0;n1-=n2) /* Nejprve zápis celého bloku */
	    if((n2=write(wfd,buf+bs-n1,n1))==-1) {
		perror("parent write()");
		exit(1);
	    }
	printf("%d",i);
	fflush(stdout);
	for(n1=bs;n1>0;n1-=n2) /* Pak ctení celého bloku */
	    if((n2=read(rfd,buf+bs-n1,n1))==-1) {
		perror("parent read()");
		exit(1);
	    }
	printf(")");
	fflush(stdout);
    }
}

void child(int rfd,int wfd,int bs)
{
    char buf;
    for(;;) {
	switch(read(rfd,&buf,1)) {
	    case -1:
		perror("child read()");
		exit(1);
	    case 0:
		printf("CHILD: EOF\n");
		exit(0);
	    default:
		if(write(wfd,&buf,1)==-1) {
		    perror("child write()");
		    exit(1);
		}
	}
    }
}

int main(int argc,char *argv[])
{
    int p1[2],p2[2];
    int bs,nb;
    if(argc!=3) {
	fprintf(stderr,"usage: %s block_size n_blocks\n",argv[0]);
	exit(1);
    }
    bs=atoi(argv[1]);
    nb=atoi(argv[2]);
    if(bs<=0||nb<=0) {
	fprintf(stderr,"Block size and count must be at least 1\n");
	exit(1);
    }
    printf("PROCESSING %d BLOCKS OF %d BYTES\n",nb,bs);
    if(pipe(p1)==-1) {
	perror("pipe()");
	exit(1);
    }
    if(pipe(p2)==-1) {
	perror("pipe()");
	exit(1);
    }
    switch(fork()) {
	case -1:
	    perror("fork()");
	    exit(1);
	case 0:
	    close(p1[1]);
	    close(p2[0]);
	    child(p1[0],p2[1],bs);
	    break;
	default:
	    close(p1[0]);
	    close(p2[1]);
	    parent(p1[1],p2[0],bs,nb);
	    break;
    }
    exit(0);
}
