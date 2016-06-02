/* Interaktivní TCP klient - vstup z klávesnice posílá na sít, vstup ze síte
   zobrazuje na terminálu */

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSZ 64

int run_echoing(int fd) /* Prepisuje stdin do fd a naopak, koncí pri EOF,
			   vrací 1 pri EOF na stdin, jinak vrací 0 */
{
  int sz,szw;
  fd_set rfdset,efdset;
  char buf[BUFSZ];
  for(;;) {
    FD_ZERO(&rfdset);
    FD_SET(0,&rfdset);
    FD_SET(fd,&rfdset);
    efdset=rfdset;
    while(select(fd+1,&rfdset,NULL,&efdset,NULL)==-1) {
      perror("select()");
      if(errno==EINTR)
	continue;
      exit(1);
    }
    if(FD_ISSET(0,&efdset))
      fprintf(stderr,"### EXCEPTION AT stdin DETECTED BY select(). ###\n");
    if(FD_ISSET(fd,&efdset))
      fprintf(stderr,"### EXCEPTION AT network DETECTED BY select(). ###\n");
    if(FD_ISSET(0,&rfdset)) {
      if((sz=read(0,buf,BUFSZ))==-1) {
	perror("read(stdin)");
	exit(1);
      }
      if(sz==0) {
	fprintf(stderr,"### EOF ON stdin. ###\n");
	return 1;
      }
      fprintf(stderr,"### RECEIVED %d BYTES FROM stdin. ###\n",sz);
      if((szw=write(fd,buf,sz))==-1) {
	perror("write(net)");
	exit(1);
      }
      fprintf(stderr,"### SENT %d BYTES TO network. ###\n",szw);
      if(szw<sz)
	fprintf(stderr,"### DATA LOST DURING WRITE TO network. ###\n");
    }
    if(FD_ISSET(fd,&rfdset)) {
      if((sz=read(fd,buf,BUFSZ))==-1) {
	perror("read(net)");
	exit(1);
      }
      if(sz==0) {
	fprintf(stderr,"### EOF ON network. ###\n");
	return 0;
      }
      fprintf(stderr,"### RECEIVED %d BYTES FROM network. ###\n",sz);
      if((szw=write(1,buf,sz))==-1) {
	perror("write(stdout)");
	exit(1);
      }
      fprintf(stderr,"### SENT %d BYTES TO stdout. ###\n",szw);
      if(szw<sz)
	fprintf(stderr,"### DATA LOST DURING WRITE TO stdout. ###\n");
    }
  }
}

int main(int argc,char *argv[])
{
  char *host;
  int port;
  struct servent *se;
  struct hostent *ha;
  struct protoent *pp;
  int sockfd;
  struct sockaddr_in sa;
  /* Zpracovat argumenty */
  if(argc!=3) {
    fprintf(stderr,"usage: %s host port\n",argv[0]);
    exit(1);
  }
  host=argv[1];
  if(sscanf(argv[2],"%d",&port)!=1) {
    if(!(se=getservbyname(argv[2],"tcp"))) {
      fprintf(stderr,"%s: Unknown service\n",argv[2]);
      exit(1);
    } 
    else
      port=ntohs(se->s_port);
  }
  /* Navázat spojení */
  if(!(ha=gethostbyname(host))) {
    fprintf(stderr,"gethostbyname(): Cannot resolve name \"%s\"\n",host);
    exit(1);
  }
  if(!(pp=getprotobyname("tcp"))) {
    fprintf(stderr,"tcp: Cannot get protocol number\n");
    exit(1);
  }
  if((sockfd=socket(AF_INET,SOCK_STREAM,pp->p_proto))==-1) {
    perror("socket()");
    exit(1);
  }
  sa.sin_family=AF_INET;
  sa.sin_port=htons(port);
  memcpy(&sa.sin_addr.s_addr,ha->h_addr_list[0],ha->h_length);
  fprintf(stderr,"### CONNECTING TO %s (%s), PORT %d, PROTOCOL %d ###\n",
      host,inet_ntoa(sa.sin_addr),port,pp->p_proto);
  if(connect(sockfd,&sa,sizeof(sa))<0)
    {
      perror("connect()");
      exit(2);
    }
  fprintf(stderr,"### CONNECTED ###\n");
  run_echoing(sockfd);
  if(close(sockfd)==-1) {
    perror("close()");
    exit(1);
  }
  exit(0);
}
