/* Sítová komunikace s pouzitím protokolu UDP
 * Pri nezadání vzdálené adresy se pouzije zdrojová adresa prvního prijatého
 * datagramu. */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>

#define NOT_REACHED 0

#define BUFSZ 65535

char buf[BUFSZ+1];

const char *progname;

static void usage(void)
{
    fprintf(stderr,
"usage: %s [-vcin] [-a local_addr] [-p local_port] [remote_addr remote_port]\n"
"	-v ... verbose\n"
"	-c ... use connect()\n"
"	-i ... EOF on stdin terminates program\n"
"	-n ... received datagram of length 0 terminates program\n",
            progname);
    exit(-1);
}

/* Trik, kterým obejdeme nemoznost pouzít promenný pocet parametru v tradicním
 * preprocesoru (GCC preprocesor umí makra s promenným poctem parametru).
 * Argumenty pri volání makra musí být uzavreny ve dvojích závorkách, napr.
 * VERBOSE(("%s=%d\n", str, num)) expanduje na
 * if(cmdline.verbose) verbose("%s=%d\n", str, num)
 * Bez vnitrních závorek bychom dostali
 * if(cmdline.verbose) verbose "%s=%d\n", str, num */
#define VERBOSE(print_args) if(cmdline.verbose) verbose print_args

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

struct {
    unsigned int verbose:1, connect:1, stdin_eof:1, net_eof:1;
    char *sock_ip;
    char *sock_port;
    char *peer_ip;
    char *peer_port;
    struct sockaddr_in sockname;
    struct sockaddr_in peername;
} cmdline;

static int str2ip(const char *str, struct sockaddr_in *sock)
{
    struct hostent *he;

    if(!(he = gethostbyname(str)))
	return -1;
    assert(he->h_addrtype == AF_INET);
    assert(he->h_length == sizeof(sock->sin_addr.s_addr));
    sock->sin_family = AF_INET;
    memcpy(&sock->sin_addr.s_addr, he->h_addr_list[0], he->h_length);
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

void lost(long int i, long int o)
{
    assert(i >= o);
    if(i > o)
	fprintf(stderr, "%l bytes lost\n", i - o);
}

/* Preposílá se stdin->sock a sock->stdout. Zápis je blokující. Pokud bychom
 * potrebovali, aby mohl program dál bezet (napr. prenáset data ve druhém
 * smeru), zatímco se provádí write(), mohli bychom pouzít neblokující write()
 * a v select() cekat i na moznost zápisu (dokud zustávají nezapsaná data). */
void run(int sock)
{
    fd_set rfds;
    int maxfd = STDIN_FILENO>sock ? STDIN_FILENO : sock;
    ssize_t ilen, olen;
    struct sockaddr_in sender;
    socklen_t slen;
    struct hostent *he;
    
    while(1) {
	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);
	FD_SET(sock, &rfds);
	if(select(maxfd+1, &rfds, NULL, NULL, NULL) == -1) {
	    perror("select()");
	    if(errno == EINTR)
		continue;
	    else
		exit(-1);
	}
	
	if(FD_ISSET(STDIN_FILENO, &rfds)) {
	    if((ilen = read(STDIN_FILENO, buf, BUFSZ)) == -1) {
		perror("read(stdin)");
		exit(-1);
	    }
	    VERBOSE(("Read %ld bytes from stdin\n", (long int) ilen));
	    if(ilen == 0 && cmdline.stdin_eof)
		return;
	    if(cmdline.connect) {
		/* Bylo provedeno connect(), nemusíme zadat cílovou adresu */
		if((olen = send(sock, buf, ilen, 0)) == -1) {
		    perror("send()");
		    exit(-1);
		}
	    	VERBOSE(("Sent %ld bytes to network\n", (long int) olen));
    		lost(ilen, olen);
	    } else {
		/* Nebylo provedeno connect(), musíme zadat cílovou adresu */
		if((olen = sendto(sock, buf, ilen, 0, 
				  (struct sockaddr *) &cmdline.peername,
				  sizeof(cmdline.peername))) == -1) {
		    perror("sendto()");
		    exit(-1);
		}
		VERBOSE(("Sent %ld bytes to network\n", (long int) olen));
		lost(ilen, olen);
	    }
	}
	
	if(FD_ISSET(sock, &rfds)) {
	    slen = sizeof(sender);
	    /* Pokud bylo provedeno connect(), známe identitu druhé strany a
	     * mohli bychom volat jen recv() */
	    if((ilen = recvfrom(sock, buf, BUFSZ, 0,
				(struct sockaddr *) &sender,
				&slen)) == -1) {
		perror("recvfrom()");
		exit(-1);
	    }
	    assert(slen == sizeof(sender));
	    assert(sender.sin_family == AF_INET);
	    he = gethostbyaddr((const char *) &sender.sin_addr,
			       sizeof(sender.sin_addr), AF_INET);
	    VERBOSE(("Received %ld bytes from [%s/", ilen,
		     he ? he->h_name : inet_ntoa(sender.sin_addr)));
	    VERBOSE(("%s]:%u\n", inet_ntoa(sender.sin_addr),
		     ntohs(sender.sin_port)));
	    if(ilen == 0 && cmdline.net_eof)
		return;
	    if((olen = write(STDOUT_FILENO, buf, ilen)) == -1) {
		perror("write(stdout)");
		exit(-1);
	    }
	    VERBOSE(("Written %ld bytes to stdout\n", (long int) olen));
	    lost(ilen, olen);
	}
    }
}

int main(int argc, char *argv[])
{
    int opt, sock;
    ssize_t ilen;
    socklen_t slen;
    struct hostent *he;
    
    progname = argv[0];
    cmdline.sockname.sin_family = cmdline.peername.sin_family = AF_INET;
    
    while((opt = getopt(argc, argv, "vcina:p:")) != -1)
	switch(opt) {
	    case 'v':
		cmdline.verbose = 1;
		break;
	    case 'c':
		cmdline.connect = 1;
		break;
	    case 'i':
		cmdline.stdin_eof = 1;
		break;
	    case 'n':
		cmdline.net_eof = 1;
		break;
	    case 'a':
		cmdline.sock_ip = optarg;
    		if(str2ip(optarg, &cmdline.sockname)) {
		    fprintf(stderr, "Invalid local IP address %s\n", optarg);
		    exit(-1);
		}
		break;
	    case 'p':
		cmdline.sock_port = optarg;
		if(str2port(optarg, &cmdline.sockname)) {
		    fprintf(stderr, "Invalid local port %s\n", optarg);
		    exit(-1);
		}
		break;
	    case '?':
		usage();
		break;
	    default:
		assert(NOT_REACHED);
		break;
	}
    argc -= optind;
    argv += optind;
    if(argc != 0 && argc != 2)
	usage();
    if(argc == 2) {
	cmdline.peer_ip = argv[0];
	if(str2ip(argv[0], &cmdline.peername)) {
	    fprintf(stderr, "Invalid remote IP address %s\n", argv[0]);
	    exit(-1);
	}
	cmdline.peer_port = argv[1];
	if(str2port(argv[1], &cmdline.peername)) {
	    fprintf(stderr, "Invalid remote port %s\n", argv[1]);
	    exit(-1);
	}
    }

    if((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket()");
	exit(-1);
    }
    if(bind(sock, (struct sockaddr *) &cmdline.sockname,
	    sizeof(cmdline.sockname)) == -1) {
	perror("bind()");
	exit(-1);
    }
    slen = sizeof(cmdline.sockname);
    if(getsockname(sock, (struct sockaddr *) &cmdline.sockname, &slen) == -1) {
	perror("getsockname()");
	exit(-1);
    }
    assert(slen == sizeof(cmdline.peername));
    assert(cmdline.sockname.sin_family == AF_INET);
    if(he = gethostbyaddr((const char *) &cmdline.sockname.sin_addr,
			  sizeof(cmdline.sockname.sin_addr), AF_INET))
	cmdline.sock_ip = strdup(he->h_name);
		   
    VERBOSE(("Local socket [%s/", cmdline.sock_ip ? cmdline.sock_ip :
	     inet_ntoa(cmdline.sockname.sin_addr)));
    VERBOSE(("%s]:", inet_ntoa(cmdline.sockname.sin_addr)));
    VERBOSE(("%s/", cmdline.sock_port ? cmdline.sock_port : 
	     (sprintf(buf, "%hu", ntohs(cmdline.sockname.sin_port)), buf)));
    VERBOSE(("%hu\n", ntohs(cmdline.sockname.sin_port)));
    
    /* Není zadána vzdálená adresa, cekáme na první datagram, ale necháme ho
     * ve fronte, jen z nej zjistíme adresu */
    if(argc == 0) {
	VERBOSE(("Waiting for network data\n"));
	slen = sizeof(cmdline.peername);
	if((ilen = recvfrom(sock, buf, 1, MSG_PEEK, 
			    (struct sockaddr *) &cmdline.peername,
			    &slen)) == -1) {
	    perror("recvfrom()");
	    exit(-1);
	}
	assert(slen == sizeof(cmdline.peername));
	assert(cmdline.peername.sin_family == AF_INET);
	if(he = gethostbyaddr((const char *) &cmdline.peername.sin_addr, 
			      sizeof(cmdline.peername.sin_addr), AF_INET))
	    cmdline.peer_ip = strdup(he->h_name);
    }
    
    VERBOSE(("Remote socket [%s/", cmdline.peer_ip ? cmdline.peer_ip :
	     inet_ntoa(cmdline.peername.sin_addr)));
    VERBOSE(("%s]:", inet_ntoa(cmdline.peername.sin_addr)));
    VERBOSE(("%s/", cmdline.peer_port ? cmdline.peer_port :
	     (sprintf(buf, "%hu", ntohs(cmdline.peername.sin_port)), buf)));
    VERBOSE(("%hu, %sconnecting\n", ntohs(cmdline.peername.sin_port),
	     cmdline.connect ? "" : "not "));
    
    if(cmdline.connect &&
       connect(sock, (struct sockaddr *) &cmdline.peername,
	       sizeof(cmdline.peername)) == -1) {
	perror("connect()");
	exit(-1);
    }

    run(sock);
    
    return 0;
}	
