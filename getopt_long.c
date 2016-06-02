/* Zpracov�n� argumentu na pr�kazov� r�dce - GNU getopt_long() */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include <getopt.h>
/* Protoze pod Linuxem prid�v� basename nav�c '/' na zac�tek, mus�me jeste
   udelat malou �pravu */
#define __USE_GNU
#undef basename
#include <string.h>

#define OK 0
#define ERR 1

typedef union {
  int on_off; /* 0=volba nen�, 1=volba je */
  char *val; /* argument volby */
} opt_val;

void set_opt_val(opt_val *opt,char *val)
{
  if(!(opt->val=malloc(strlen(val)+1))) {
    perror(NULL);
    exit(ERR);
  }
  strcpy(opt->val,val);
}

int main(int argc,char *argv[])
{
  struct { /* Mozn� volby jsou -abcd, nav�c jeste pouzijeme -PEh, dlouh�
        varianty  jsou --arg_a, --arg_b, --arg_c, --arg_d, nav�c --arg_o */
    opt_val a;
    opt_val b;
    opt_val c;
    opt_val d;
    opt_val o;
  } opts={{0},{0},{0},{0},{0}};
  int opt;
  int i;
  struct option long_opts[]={
    {"arg_a",no_argument,NULL,'a'},
    {"arg_b",no_argument,NULL,'b'},
    {"arg_c",required_argument,NULL,'c'},
    {"arg_d",required_argument,NULL,'d'},
    {"opt_arg",optional_argument,NULL,'o'},
    {"help",no_argument,NULL,'h'},
    {NULL,0,NULL,0}
  };
  if(argc>1&&!strcmp(argv[1],"-P")) /* Mus�me zpracovat -P pred prvn�m vol�n�m
				       getopt() */
    putenv("POSIXLY_CORRECT=1"); /* Bez tohoto nastaven� Linux permutuje
      argumenty, napr. getopt a -b je tot�z jako getopt -b a. M�sto nastaven�
      promenn� lze d�t '+' na zac�tek 3. argumentu vol�n� getopt(). */ 
  while((opt=getopt_long(argc,argv,":abc:d:EPh",long_opts,NULL))!=-1)
    /* -c a -d maj� argument */
    /* Linuxov� manu�lov� str�nka tvrd�, ze getopt() vzdy vrac� ':', kdyz
       chyb� argument volby, ale ve skutecnosti funguje spr�vne, tj. vrac�
       ':', jen kdyz 3. argument getopt() zac�n� ':' */
    switch(opt) {
      case 'P': /* Uz jsme zpracovali */
	break;
      case 'E':
	opterr=0; /* Potlacit chybov� hl�sen� */
	break;
      case 'a':
	opts.a.on_off=1;
	break;
      case 'b':
	opts.b.on_off=1;
	break;
      case 'c':
	set_opt_val(&opts.c,optarg);
	break;
      case 'd':
	set_opt_val(&opts.d,optarg);
	break;
      case 'o':
	if(optarg)
	  set_opt_val(&opts.o,optarg);
	else
	  opts.o.on_off=1;
	break;
      case 'h':
	fprintf(stderr,
"usage: %s [-P] [-Eabh] [-c Carg] [-d Darg] [long_options] arg1 arg2 ...\n"
"long_options:\n"
"  --arg_a\n"
"  --arg_b\n"
"  --arg_c=VAL (or --arg_c VAL)\n"
"  --arg_d=VAL (or --arg_d VAL)\n"
"  --opt_arg[=VAL]\n"
"  --help\n",
	    basename(argv[0]));
	exit(OK);
	break;
      case '?':
      case ':':
	fprintf(stderr,"getopt() returned '%c'\n",opt);
	break;
      default: /* Sem bychom se nikdy nemeli dostat */
	fprintf(stderr,"getopt() returned %d\n",opt);
	break;
    }
  printf("-a: %s\n",opts.a.on_off ? "on" : "off");
  printf("-b: %s\n",opts.b.on_off ? "on" : "off");
  printf("-c: %s\n",opts.c.val ? opts.c.val : "(NULL)");
  printf("-d: %s\n",opts.d.val ? opts.d.val : "(NULL)");
  printf("-o: %s\n",
      opts.o.on_off ? (opts.o.on_off==1 ? "on" : opts.o.val) : "off");
  for(i=optind;i<argc;i++)
    printf("arg[%d]: %s\n",i-optind+1,argv[i]);
  exit(OK);
}
