/* Oboustranná komunikace mezi dvema procesy pomocí dvou opacne orientovaných
   rour. Jeden proces generuje data a po blocích je odesílá druhému, který je
   zpracovává (po jednotlivých bajtech) a posílá zpet. Moznost deadlocku pri
   naplnení bufferu rour je eliminována pouzitím select() a neblokujících
   operací read() a write(). Velikost bloku a pocet bloku se zadávají jako
   parametry. */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#define max(a,b) ( ((a)>(b)) ? (a) : (b) )

void parent(int wfd,int rfd,int bs,int nb)
{
    char *bufw,*bufr;
    int i,n1r,n2r,n1w,n2w,rfl,wfl;
    fd_set rs,ws;
    if(!(bufw=malloc(bs))||!(bufr=malloc(bs))) {
	perror("malloc()");
	exit(1);
    }
    /* Nastavit neblokující ctení a zápis */
    if((wfl=fcntl(wfd,F_GETFL))==-1||(rfl=fcntl(rfd,F_GETFL))==-1||
	fcntl(wfd,F_SETFL,wfl|O_NONBLOCK)==-1||
	fcntl(rfd,F_SETFL,rfl|O_NONBLOCK)==-1) {
	perror("parent fcntl()");
	exit(1);
    }
    for(i=0;i<nb;i++) { /* Opakuj pro nb bloku */
	printf("(");
	fflush(stdout);
	n1r=bs;
	n1w=bs;
	while(n1r>0||n1w>0) {
	    FD_ZERO(&rs);
	    FD_ZERO(&ws);
	    FD_SET(rfd,&rs);
	    FD_SET(wfd,&ws);
	    if(select(max(rfd,wfd)+1,&rs,&ws,NULL,NULL)==-1) {
		perror("parent select()");
		exit(1);
	    }
	    if(n1w>0&&FD_ISSET(wfd,&ws)) { /* Mozno zapisovat */
		if((n2w=write(wfd,bufw+bs-n1w,n1w))==-1) {
		    perror("parent write()");
		    exit(1);
		}
		n1w-=n2w;
	    }
	    if(n1r>0&&FD_ISSET(rfd,&rs)) { /* Mozno císt */
		if((n2r=read(rfd,bufr+bs-n1r,n1r))==-1) {
		    perror("parent read()");
		    exit(1);
		}
		n1r-=n2r;
	    }
	}
	printf("%d)",i);
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
