/* Interaktivní TCP server - vstup z klávesnice posílá na sít, vstup ze síte
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
  int port,nclients,sockfd,newsock,ca_sz;
  struct servent *sp;
  struct protoent *pp;
  struct sockaddr_in sa,ca;
  struct hostent *ca_ent;
  char *ca_name,*ca_addr;
  /* Zpracovat argumenty */
  if(argc!=3) {
    fprintf(stderr,"usage: %s port num_clients\n",argv[0]);
    exit(1);
  }
  if(sscanf(argv[1],"%d",&port)!=1) {
    if(!(sp=getservbyname(argv[1],"tcp"))) {
      fprintf(stderr,"%s: Unknown service\n",argv[1]);
      exit(1);
    } 
    else
      port=ntohs(sp->s_port);
  }
  if(sscanf(argv[2],"%d",&nclients)!=1||nclients<=0) {
    fprintf(stderr,"%s: Not a positive integer\n",argv[2]);
    exit(1);
  }
  /* Pripravit spojení */
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
  sa.sin_addr.s_addr=INADDR_ANY;
  if(bind(sockfd,(struct sockaddr *)&sa,sizeof(sa))==-1) {
    perror("bind()");
    exit(1);
  }
  if(listen(sockfd,nclients)==-1) {
    perror("listen()");
    exit(1);
  }
  for(;;) {
    /* Akceptovat spojení od klienta */
    fprintf(stderr,"### LISTENING TO network ON PORT %d, PROTOCOL %d FOR "
	"%d CLIENTS ###\n",port,pp->p_proto,nclients);
    ca_sz=sizeof(ca);
    if((newsock=accept(sockfd,&ca,&ca_sz))==-1) {
      perror("accept()");
      exit(1);
    }
    ca_addr=inet_ntoa(ca.sin_addr);
    if(!(ca_ent=gethostbyaddr((void *)&ca.sin_addr,
	    sizeof(ca.sin_addr),AF_INET)))
      ca_name="unknown name";
    else
      ca_name=ca_ent->h_name;
    fprintf(stderr,"### ACCEPTED CONNECTION FROM %s (%s), PORT %d ###\n",
	ca_name,ca_addr,ntohs(ca.sin_port));
    /* Komunikace s klientem */
    if(run_echoing(newsock))
      break;
  }
  /* Server má EOF na stdin, konec */
  if(close(sockfd)==-1) {
    perror("close()");
    exit(1);
  }
  exit(0);
}
