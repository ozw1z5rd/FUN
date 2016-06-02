/* UDP klient, posílá stdin serveru a vypisuje odpovedi */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define BUF_SZ 1024

int main(int argc, char *argv[])
{
    char *sname, buf[BUF_SZ];
    struct hostent *ha;
    struct servent *sp;
    struct sockaddr_in sa;
    int port, fd, sz;
    
    /* Zpracovat argumenty */
    if(argc != 3) {
	fprintf(stderr, "usage: %s server_address port\n", argv[0]);
	exit(1);
    }
    sname = argv[1];
    if(!(ha = gethostbyname(sname))) {
	fprintf(stderr, "Cannot resolve server address\n");
	exit(1);
    }
    if(sscanf(argv[2], "%d", &port) !=1) {
	if(!(sp = getservbyname(argv[2], "tcp"))) {
    	    fprintf(stderr,"%s: Unknown service\n",argv[2]);
    	    exit(1);
	} else
	    port = ntohs(sp->s_port);
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    memcpy(&sa.sin_addr, ha->h_addr_list[0], sizeof(sa.sin_addr));
    
    /* Pripravit soket */
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket()");
	exit(1);
    }

    /* Hlavní cyklus */
    while(fgets(buf, sizeof(buf), stdin)) {
	sz = strlen(buf);
	if(buf[sz-1] == '\n')
	    buf[--sz] = '\0';
	if(sendto(fd, buf, sz, 0, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
	    perror("sendto()");
	    exit(1);
	}
	if((sz = recv(fd, buf, sizeof(buf-1), 0)) == -1) {
	    perror("recv()");
	    exit(1);
	}
	buf[sz] = '\0';
	printf("Received \"%s\"\n", buf);
    }
    if(ferror(stdin)) {
	perror("fgets()");
	return 1;
    } else
	return 0;
}
