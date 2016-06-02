/* UDP server, který vrací délky prijatých datagramu */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFSZ 16

int main(int argc, char *argv[])
{
    int port, fd, sz, ca_sz;
    struct servent *sp;
    struct hostent *ha;
    char *cname, *cip, buf[BUFSZ];
    struct sockaddr_in sa, ca;
    
    /* Zpracovat argumenty */
    if(argc != 2) {
	fprintf(stderr, "usage: %s port\n", argv[0]);
	exit(1);
    }
    if(sscanf(argv[1], "%d", &port) !=1) {
	if(!(sp = getservbyname(argv[1], "tcp"))) {
	    fprintf(stderr,"%s: Unknown service\n",argv[1]);
	    exit(1);
	} else
	    port = ntohs(sp->s_port);
    }
    
    /* Pripravit socket */
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket()");
	exit(1);
	}
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = INADDR_ANY;
    if(bind(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
	perror("bind()");
	exit(1);
    }

    /* Hlavní cyklus */
    ca_sz = sizeof(ca);
    while( (sz = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *) &ca,
			  &ca_sz)) >= 0) {
	cip = inet_ntoa(ca.sin_addr);
	if(!(ha = gethostbyaddr( (void *) &ca.sin_addr, sizeof(ca.sin_addr),
				 AF_INET)))
	    cname = "unknown name";
	else
	    cname = ha->h_name;
	printf("Received %d bytes from %s[%s]:%d\n", sz, cname, cip,
	       ntohs(ca.sin_port));
	sz = snprintf(buf, sizeof(buf), "%d", sz);
	buf[sizeof(buf)-1] = '\0';
	if(sendto(fd, buf, sz, 0, (struct sockaddr *) &ca, sizeof(ca)) == -1) {
	    perror("sendto()");
	    exit(1);
	}
    }
    perror("recvfrom()");
    return 1;
}
