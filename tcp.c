/* Jednoduchý TCP klient/server */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <limits.h>

#ifndef __GNUC__
#define UNUSED
#else  /* ! __GNUC__ */
#define UNUSED __attribute__((unused))
#endif /* ! __GNUC__ */

#define NOT_REACHED 0

#define BACKLOG 10

#define BUFSZ 65535

char buf[BUFSZ + 1];

sigset_t sigs;
sigjmp_buf jmp_label;

#define VERBOSE(print_args) if(cmdline.verbose) verbose print_args
#define VERBDATA(print_args) if(cmdline.verbdata) verbose print_args

struct {
    unsigned int verbose:1, verbdata:1, stdin_eof:1, net_eof:1;
    unsigned int reuse_addr:1, srv:1, srv1:1, linger:1;
    unsigned int opt_a:1, opt_p:1, set_addr:1, set_port:1;
    int linger_time;
    char *sock_ip;
    char *sock_port;
    char *peer_ip;
    char *peer_port;
    struct sockaddr_in sockname;
    struct sockaddr_in peername;
} cmdline;

static void handler(int sig)
{
    if(sig == SIGQUIT)
	_exit(1);
    else
	siglongjmp(jmp_label, sig);
}

static void usage(void)
{
    fprintf(stderr,
"Operate as TCP client:\n"
"tcp [-vdhinA] [-l time] [-a local_addr] [-p local_port] "
"remote_addr remote_port\n"
"Operate as TCP server:\n"
"tcp {-s|-S} [-vdhinA] [-l time] [local_addr] [local_port]\n"
"	-s ... accept single client\n"
"	-S ... accept many clients (sequentially)\n"
"	-v ... verbose\n"
"	-d ... write size of read/sent data to stderr\n"
"	-h ... help - write this message and exit\n"
"	-i ... EOF on stdin terminates connection\n"
"	-n ... EOF from network terminates connection\n"
"	(without -i/-n, EOF must occur both on stdin and network)\n"
"	-A ... set SO_REUSEADDR on socket\n"
"	-l ... set SO_LINGER with given timeout\n"
"signals: SIGINT  ... reset network connection\n"
"	  SIGQUIT ... terminate\n"
"	  SIGUSR1 ... shutdown network connection for sending\n"
"	  SIGUSR2 ... shutdown network connection for receiving\n");
    exit(1);
}

static void verbose(const char *format, ...)
{
    va_list arg;
    static int mark = 0;

    if(!mark)
	fprintf(stderr, "** ");
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    mark = (format[strlen(format)-1] != '\n');
}

static int str2ip(const char *str, struct sockaddr_in *sock)
{
    struct hostent *he;

    if(!(he = gethostbyname(str)))
	return -1;
    assert(he->h_addrtype == AF_INET);
    assert(he->h_length == sizeof(sock->sin_addr.s_addr));
    sock->sin_family = AF_INET;
    memcpy(&sock->sin_addr.s_addr, he->h_addr_list[0], (size_t)he->h_length);
    return 0;
}

static int str2port(const char *str, struct sockaddr_in *sock)
{
    struct servent *se;
    unsigned long p;
    char *err;

    if((se = getservbyname(str, "udp"))) {
	p = se->s_port;
    } else {
	p = strtoul(str, &err, 10);
	if(!*str || *err || p > USHRT_MAX)
	    return -1;
	p = htons((unsigned short) p);
    }
    sock->sin_family = AF_INET;
    sock->sin_port = p;
    return 0;
}

static void run_echoing(int fd)
{
    int sz, szw, sel, sig;
    static int net_eof, stdin_eof;
    fd_set rfdset,efdset;
    struct linger l;
    struct sigaction siga;

    net_eof = stdin_eof = 0;
    sigemptyset(&siga.sa_mask);
    siga.sa_flags = 0;

    while(!net_eof || !stdin_eof) {
	FD_ZERO(&rfdset);
	if(!stdin_eof)
	    FD_SET(0, &rfdset);
	if(!net_eof)
	    FD_SET(fd, &rfdset);
	efdset = rfdset;
	
	if((sig = sigsetjmp(jmp_label, 1))) {
	    switch(sig) {
		case 0:
		    break;
		case SIGINT:
		    VERBOSE(("SIGINT caught, resetting connection\n"));
		    l.l_onoff = 1;
		    l.l_linger = 0;
		    if(setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l))) {
			perror("setsockopt()");
			exit(1);
		    }
		    return;
		case SIGUSR1:
		    VERBOSE(("SIGHUP1 caught, shutdown for sending\n"));
		    if(shutdown(fd, SHUT_WR)) {
			perror("shutdown()");
			exit(1);
		    }
		    break;
		case SIGUSR2:
		    VERBOSE(("SIGHUP2 caught, shutdown for receiving\n"));
		    if(shutdown(fd, SHUT_RD)) {
			perror("shutdown()");
			exit(1);
		    }
		default:
		    break;
	    }
	}

	sigprocmask(SIG_UNBLOCK, &sigs, NULL);
	sel = select(fd+1, &rfdset, NULL, &efdset, NULL);
	sigprocmask(SIG_BLOCK, &sigs, NULL);
	if(sel == -1) {
	    perror("select()");
	    exit(1);
	}
	
	if(FD_ISSET(0, &efdset))
	    VERBOSE(("Exception at stdin detected by select()\n"));
	if(FD_ISSET(fd, &efdset))
	    VERBOSE(("Exception at network detected by select()\n"));
	
	if(FD_ISSET(0, &rfdset)) {
	    if((sz = read(0, buf, BUFSZ)) == -1) {
		perror("read(stdin)");
		exit(1);
	    }
	    if(sz == 0) {
		VERBDATA(("EOF on stdin\n"));
		stdin_eof = 1;
		if(cmdline.stdin_eof)
		    return;
		else {
		    VERBOSE(("Shutdown for writing\n"));
		    if(shutdown(fd, SHUT_WR)) {
			perror("shutdown()");
			exit(1);
		    }
		}
	    } else
		VERBDATA(("Received %d bytes from stdin\n", sz));
	    if(sz > 0) {
		siga.sa_handler = SIG_IGN;
		if(sigaction(SIGPIPE, &siga, NULL)) {
		    perror("sigaction()");
		    exit(1);
		}
		szw = write(fd, buf, (size_t)sz);
		siga.sa_handler = SIG_DFL;
		if(sigaction(SIGPIPE, &siga, NULL)) {
		    perror("sigaction()");
		    exit(1);
		}
		if(szw == -1) {
		    perror("write(net)");
		    if(errno == EPIPE)
			return;
		    else
			exit(1);
		}
		VERBDATA(("Sent %d bytes to network\n", szw));
		if(szw < sz)
		    fprintf(stderr, "** Data lost during write to network\n");
	    }
	}
	
	if(FD_ISSET(fd, &rfdset)) {
	    if((sz = read(fd, buf, BUFSZ)) == -1) {
		perror("read(net)");
		if(errno == ECONNRESET)
		    return;
		else
		    exit(1);
	    }
	    if(sz == 0) {
		VERBDATA(("EOF on network\n"));
		net_eof = 1;
		if(cmdline.net_eof)
		    return;
		else {
		    VERBOSE(("Shutdown for reading\n"));
		    if(shutdown(fd, SHUT_RD)) {
			perror("shutdown()");
			exit(1);
		    } else
			continue;
		}
	    } else
		VERBDATA(("Received %d bytes from network\n", sz));
	    if((szw = write(1, buf, (size_t)sz)) == -1) {
		perror("write(stdout)");
		exit(1);
	    }
	    VERBDATA(("Sent %d bytes to stdout\n", szw));
	    if(szw < sz)
		fprintf(stderr, "** Data lost during write to stdout\n");
	}
    }
}

int main(int argc,char *argv[])
{
    struct sigaction siga;
    struct linger l;
    int opt, fd, sockfd;
    socklen_t alen;
    struct hostent *he;
    
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGUSR1);
    sigaddset(&sigs, SIGUSR2);
    sigprocmask(SIG_BLOCK, &sigs, NULL);
    siga.sa_handler = handler;
    siga.sa_mask = sigs;
    siga.sa_flags = 0;
    sigaction(SIGINT, &siga, NULL);
    sigaction(SIGQUIT, &siga, NULL);
    sigaction(SIGUSR1, &siga, NULL);
    sigaction(SIGUSR2, &siga, NULL);
    
    cmdline.sockname.sin_family = cmdline.peername.sin_family = AF_INET;
    while((opt = getopt(argc, argv, "sSvdhinAPl:a:p:")) != -1)
	switch(opt) { 
	    case 's':
		cmdline.srv1 = 1;
		/* nobreak */
	    case 'S':
		cmdline.srv = 1;
		break;
	    case 'v':
		cmdline.verbose = 1;
		break;
	    case 'd':
		cmdline.verbdata = 1;
		break;
	    case 'i':
		cmdline.stdin_eof = 1;
		break;
	    case 'n':
		cmdline.net_eof = 1;
		break;
	    case 'A':
		cmdline.reuse_addr = 1;
		break;
	    case 'l':
		cmdline.linger = 1;
		cmdline.linger_time = atoi(optarg);
		break;
	    case 'a':
		cmdline.sock_ip = optarg;
		cmdline.opt_a = 1;
		if(str2ip(optarg, &cmdline.sockname)) {
		    fprintf(stderr, "Invalid local IP address %s\n", optarg);
		    exit(1);
		}
		break;
	    case 'p':
		cmdline.sock_port = optarg;
		cmdline.opt_p = 1;
		if(str2port(optarg, &cmdline.sockname)) {
		    fprintf(stderr, "Invalid local port %s\n", optarg);
		    exit(1);
		}
		break;
	    case 'h':
	    case '?':
		usage();
		break;
	    default:
		assert(NOT_REACHED);
		break;
	}
    argc -= optind;
    argv += optind;
    if(argc > 2) 
	usage();
    if(argc > 0) {
	cmdline.set_addr = 1;
	cmdline.peer_ip = argv[0];
	if(str2ip(cmdline.peer_ip, &cmdline.peername)) {
	    fprintf(stderr, "Invalid %s IP address %s\n",
		    cmdline.srv ? "local" : "remote", cmdline.peer_ip);
	    exit(1);
	}
	if(argc > 1) {
	    cmdline.set_port = 1;
	    cmdline.peer_port = argv[1];
	    if(str2port(cmdline.peer_port, &cmdline.peername)) {
		fprintf(stderr, "Invalid %s port %s\n",
			cmdline.srv ? "local" : "remote", cmdline.peer_port);
		exit(1);
	    }
	}
    }
    if(cmdline.srv) {
	if(cmdline.opt_a) {
	    fprintf(stderr, "Option -a not valid in server mode\n");
	    exit(1);
	}
	if(cmdline.opt_p) {
	    fprintf(stderr, "Option -p not valid in server mode\n");
	    exit(1);
	}
	cmdline.sock_ip = cmdline.peer_ip;
	cmdline.sock_port = cmdline.peer_port;
	cmdline.sockname = cmdline.peername;
    } else
	if(!cmdline.set_addr || !cmdline.set_port) {
	    fprintf(stderr, "Remote address and port needed\n");
	    exit(1);
	}
    
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket()");
	exit(1);
    }
    opt = 1;
    if(cmdline.reuse_addr &&
       setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
	perror("setsockopt(SO_REUSEADDR)");
	exit(1);
    }
    l.l_onoff = 1;
    l.l_linger = cmdline.linger_time;
    if(cmdline.linger &&
       setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &l, sizeof(l))) {
	perror("setsockopt(SO_LINGER)");
	exit(1);
    }
    if(bind(sockfd, (struct sockaddr *)&cmdline.sockname,
	    sizeof(cmdline.sockname))) {
	perror("bind()");
	exit(1);
    }
    alen = sizeof(cmdline.sockname);
    if(getsockname(sockfd, (struct sockaddr *)&cmdline.sockname, &alen)) {
	perror("getsockname()");
	exit(1);
    }
    if((he = gethostbyaddr((const char *)&cmdline.sockname, (int)alen, 
			   AF_INET))) {
	if(!(cmdline.sock_ip = strdup(he->h_name))) {
	    perror("strdup()");
	    exit(1);
	}
    } else {
	if(!(cmdline.sock_ip = strdup(inet_ntoa(cmdline.sockname.sin_addr)))) {
	    perror("strdup()");
	    exit(1);
	}
    }
    VERBOSE(("Local address is %s (%s) port %hd\n",
	     cmdline.sock_ip, inet_ntoa(cmdline.sockname.sin_addr),
	     ntohs(cmdline.sockname.sin_port)));
				
    if(cmdline.srv) {
	if(listen(sockfd, BACKLOG)) {
	    perror("listen()");
	    exit(1);
	}
	for(;;) {
	    VERBOSE(("Waiting for connection\n"));
	    alen = sizeof(cmdline.peername);
	    if((fd = accept(sockfd, (struct sockaddr *)&cmdline.peername,
			    &alen)) == -1) {
		perror("accept()");
		exit(1);
	    }
	    if((he = gethostbyaddr((const char *)&cmdline.peername, (int)alen,
				   AF_INET)))
		cmdline.peer_ip = he->h_name;
	    else
		cmdline.peer_ip = inet_ntoa(cmdline.peername.sin_addr);
	    VERBOSE(("Connection from %s (%s) port %hd\n",
		     cmdline.peer_ip, inet_ntoa(cmdline.peername.sin_addr),
		     ntohs(cmdline.peername.sin_port)));
	    if(cmdline.srv1 && close(sockfd)) {
		perror("close()");
		exit(1);
	    }
	    run_echoing(fd);
	    VERBOSE(("Closing connection\n"));
	    if(close(fd)==-1) {
		perror("close()");
		exit(1);
	    }
	    VERBOSE(("Closed\n"));
	    if(cmdline.srv1)
		break;
	}
    } else {
	VERBOSE(("Connecting to %s (%s), port %s (%hd)\n",
		 cmdline.peer_ip, inet_ntoa(cmdline.peername.sin_addr),
		 cmdline.peer_port, ntohs(cmdline.peername.sin_port)));
	if(connect(sockfd, (struct sockaddr *)&cmdline.peername,
		   sizeof(cmdline.peername)) < 0) {
	    perror("connect()");
	    exit(2);
	}
	VERBOSE(("Connected\n"));
	run_echoing(sockfd);
	VERBOSE(("Closing connection\n"));
	if(close(sockfd)==-1) {
	    perror("close()");
	    exit(1);
	}
	VERBOSE(("Closed\n"));
    }
    
    exit(0);
}
