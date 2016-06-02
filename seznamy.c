/* CGI program pro zapisování lidí do seznamu */

/* Zobrazí seznam se zadaným názvem a poctem kolonek. Do seznamu se muze
 * kdokoliv zapsat az do vycerpání kapacity (jméno, príjmení, poznámka -
 * libovolný text, nezobrazuje se). Kazdý dostane pridelené heslo, kterým se
 * muze ze seznamu vymazat. Konfiguracní soubor $PATH_TRANSLATED.conf definuje
 * pro kazdý název seznamu jeho délku, soubor s obsahem a nadpis na rádku
 * tvaru: název:nadpis:délka:soubor
 * Obsah seznamu je ulozen v souboru s rádky tvaru:
 * jméno:príjmení:heslo:poznámka 
 */

#define _XOPEN_SOURCE 500

#if defined(__sun) && defined(__svr4__)
#undef _XOPEN_SOURCE
#endif /* Protoze Solaris definuje struct timeval, pouze kdyz není definováno
	  _XOPEN_SOURCE !?! */
#include <sys/time.h>
#if defined(__sun) && defined(__svr4__)
#define _XOPEN_SOURCE 500
#endif
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/utsname.h>
#include <ctype.h>

/*** Jednotlivá pole formuláre ***/

#define SZ_JMENO 20
#define SZ_PRIJMENI 30
#define SZ_POZNAMKA 80
#define SZ_HESLO 10

#define SZ_JMENO_STR "20"
#define SZ_PRIJMENI_STR "30"
#define SZ_POZNAMKA_STR "80"
#define SZ_POZNAMKA_INPUT_STR "40"
#define SZ_HESLO_STR "10"

typedef char jmeno_t[SZ_JMENO+1];
typedef char prijmeni_t[SZ_PRIJMENI+1];
typedef char poznamka_t[SZ_POZNAMKA+1];
typedef char heslo_t[SZ_HESLO+1];

jmeno_t jmeno="";
prijmeni_t prijmeni="";
poznamka_t poznamka="";
heslo_t heslo="";

/*** Víceúcelový buffer pro retezce (pro volné pouzití, nelze spoléhat na
 * zachování obsahu) ***/

#define CHAR_BUF_SZ 4096

char char_buf[CHAR_BUF_SZ];

/*** HTTP hlavicka ***/

void doc_header()
{
  printf("Content-type: text/html;charset=iso-8859-2\n\n");
}

/*** Prekódování 1 znaku pro HTML (náhrada <, >, &) ***/

const char *html_char(char c)
{
  static char buf[]=" ";
  switch(c) {
    case '<':
      return "&lt;";
    case '>':
      return "&gt;";
    case '&':
      return "&amp;";
    case '"':
      return "&quot;";
    default:
      buf[0]=c;
      return buf;
  }
}

/*** Vypsat text upravený pro HTML (náhrada speciálních znaku) ***/

void html_puts(const char *text,int len)
{
  for(;len>0;len--)
    fputs(html_char(*text++),stdout);
}

/*** Vypsat HTML dokument s chybovým hlásením a konec programu ***/

void err_msg(const char *msg,...)
{
  va_list val;
  static char err_buf[CHAR_BUF_SZ];
  printf("<HTML>\n<HEAD>\n<TITLE>Chyba</TITLE>\n</HEAD>\n<BODY>\n"
      "<H1>Pri zpracování nastala chyba</H1>\n<P><TT>");
  va_start(val,msg);
#if defined(__sun) && defined(__svr4__)
  vsprintf(err_buf,msg,val); /* Na Solarisu chybí vsnprintf() */
#else
  vsnprintf(err_buf,CHAR_BUF_SZ,msg,val);
#endif
  html_puts(err_buf,strlen(err_buf));
  va_end(val);
  printf("</TT></P>\n</BODY>\n</HTML>\n");
  exit(1);
}

/*** Otevrít a namapovat do pameti konfiguracní soubor, vrací deskriptor,
 * adresu mapování a délku souboru ***/

void open_conf(int *fd,const char **addr,int *len)
{
  char *env,*p;
  /* Z PATH_TRANSLATED získat cestu ke konfiguracnímu souboru */
  if(!(env=getenv("PATH_TRANSLATED")))
    err_msg("Missing PATH_TRANSLATED");
  if(strlen(env)>=CHAR_BUF_SZ-8) /* Budeme retezec prodluzovat o ".conf" */
    err_msg("Value of PATH_TRANSLATED too long");
  strcpy(char_buf,env);
  /* Nejprve se odríznou prípadné prípony oddelené teckami */
  if(!(p=strrchr(char_buf,'/')))
    p=char_buf;
  if(!(p=strchr(p,'.')))
    p=char_buf+strlen(char_buf);
  /* a pripojí se prípona ".conf" */
  strcpy(p,".conf");
  /* Otevrít soubor, zjistit délku a namapovat */
  if((*fd=open(char_buf,O_RDONLY))==-1)
    err_msg("%s: %s",char_buf,strerror(errno));
  if((*len=lseek(*fd,0,SEEK_END))==-1)
    err_msg("%s: %s",char_buf,strerror(errno));
  if(len==0)
    err_msg("Prázdný konfiguracní soubor \"%s\"",char_buf);
  if((*addr=(char *)mmap(0,*len,PROT_READ,MAP_SHARED,*fd,0))==MAP_FAILED)
    err_msg("mmap(\"%s\"): %s",char_buf,strerror(errno));
  if((*addr)[*len-1]!='\n')
    err_msg("Chybí konec rádku na konci konfiguracního souboru \"%s\"",
	char_buf);
}

/*** Otevrít a namapovat do pameti (kdyz je neprázdný) soubor se seznamem ***/

void open_list(const char *name,int *fd,const char **addr,int *len)
{
   if((*fd=open(name,O_RDONLY))==-1)
    err_msg("%s: %s",char_buf,strerror(errno));
  if((*len=lseek(*fd,0,SEEK_END))==-1)
    err_msg("%s: %s",char_buf,strerror(errno));
  if(*len==0) {
    addr=NULL;
    return;
  }
  if((*addr=(char *)mmap(0,*len,PROT_READ,MAP_SHARED,*fd,0))==MAP_FAILED)
    err_msg("mmap(\"%s\"): %s",char_buf,strerror(errno));
  if((*addr)[*len-1]!='\n')
    err_msg("Chybí konec rádku na konci souboru \"%s\"",
	char_buf); 
}

/*** Projít konfiguracní soubor o délce len namapovaný na adrese addr, vrátit
 * adresu rádku s definicí seznamu se jménem name ***/

const char *get_list(const char *conf_addr,int len,const char *name)
{
  int n_l=strlen(name);
  while(len>n_l) {
    if(!strncmp(name,conf_addr,n_l)&&conf_addr[n_l]==':')
      return conf_addr; /* Nalezen správný rádek */
    while(*conf_addr++!='\n') /* Tento rádek to není, jdi na zacátek dalsího */
      len--;
    len--;
  }
  return NULL;
}

/*** Precíst rádek souboru (konfiguracního nebo obsahu seznamu), naplní
 * nfields polozek (adresa, délka) oddelených dvojteckou, poslední polozka
 * pokracuje az do konce rádku (muze obsahovat dvojtecky). Vrací adresu
 * zacátku následujícího rádku souboru ***/

typedef struct { const char *p;int l;} fields_t;

const char *split_line(const char *line,fields_t *fields,int nfields)
{
  int nf;
  for(nf=0;nf<nfields;nf++) {
    fields[nf].p=line;
    fields[nf].l=0;
  }
  nf=0;
  for(;;) /* Pri otevrení souboru bylo zkontrolováno, ze na konci
	     souboru je konec rádku */
    switch(*line++) {
      case '\n':
	if(nf<nfields-1)
	  err_msg("split_line(): chybný formát souboru");
	return line;
      case ':':
	if(nf<nfields-1) {
	  nf++;
	  fields[nf].p=line; /* Tady uz line ukazuje na znak za dvojteckou */
	  fields[nf].l=0;
	  break;
	}
	/* V poslední polozce mohou být dalsí dvojtecky */
      default:
	fields[nf].l++;
	break;
    }
}

/*** Precíst nadpis, délku a jméno souboru z rádku konfiguracního souboru ***/

void get_list_attr(const char *line,const char **title,int *title_l,int *len,
    const char **file,int *file_l)
{
  fields_t fields[4];
  split_line(line,fields,4);
  *title=fields[1].p;
  *title_l=fields[1].l;
  if(fields[2].l>=CHAR_BUF_SZ)
    err_msg("Chybná délka seznamu v konfiguracním souboru");
  memcpy(char_buf,fields[2].p,fields[2].l);
  char_buf[fields[2].l]='\0';
  *len=atoi(char_buf);
  *file=fields[3].p;
  *file_l=fields[3].l;
}

/*** Vypsat seznam, parametr list ukazuje na rádek konfiguracního souboru s
 * definicí seznamu ***/

void show_list(const char *list)
{
  const char *title,*file,*addr,*p;
  int title_l,len,file_l,fd,flen,cislo;
  fields_t list_line[3];
  /* Zjistit nadpis, pocet polozek a jméno souboru pro seznam */
  get_list_attr(list,&title,&title_l,&len,&file,&file_l);
  if(file_l>=CHAR_BUF_SZ)
    err_msg("Prílis dlouhé jméno souboru se seznamem");
  memcpy(char_buf,file,file_l);
  char_buf[file_l]='\0';
  /* Otevrít a namapovat soubor */
  open_list(char_buf,&fd,&addr,&flen);
  /* Vytvorit zacátek stránky */
  if(title_l>=CHAR_BUF_SZ)
    err_msg("Prílis dlouhý nadpis seznamu");
  memcpy(char_buf,title,title_l);
  char_buf[title_l]='\0';
  printf("<HTML>\n<HEAD>\n<TITLE>%s</TITLE>\n</HEAD>\n<BODY>\n",char_buf);
  printf("<H1>%s</H1>\n",char_buf);
  /* Jestlize je heslo neprázdné, byl vlozen nový záznam */
  if(heslo[0]!='\0') {
    printf("<P><B>Pridáno do seznamu</B><BR>\n"
	"<B>Jméno: </B>");
    html_puts(jmeno,strlen(jmeno));
    printf("<BR>\n<B>Príjmení: </B>");
    html_puts(prijmeni,strlen(prijmeni));
    printf("<BR>\n<B>Heslo: </B>");
    html_puts(heslo,strlen(heslo));
    printf(" <B>(zadává se pri mazání polozky seznamu)</B><BR>\n"
	"<B>Poznámka: </B>");
    html_puts(poznamka,strlen(poznamka));
    printf("</P>\n");
  } else /* Prázdné heslo, neprázdné jméno - byl zrusen záznam */
    if(jmeno[0]!='\0') {
      printf("<P><B>Smazáno ze seznamu</B><BR>\n"
	  "<B>Jméno: </B>");
      html_puts(jmeno,strlen(jmeno));
      printf("<BR>\n<B>Príjmení: </B>");
      html_puts(prijmeni,strlen(prijmeni));
      printf("</P>\n");
    }
  /* Záhlaví tabulky */
  printf("<TABLE border cellpadding=\"3\">\n"
      "<TR><TD align=\"right\"><B>C.</B></TD><TD><B>Jméno</B></TD>"
      "<TD><B>Príjmení</B></TD></TR>\n");
  /* Vypsat jednotlivé polozky seznamu */
  p=addr;
  cislo=1;
  while(p<addr+flen) {
    p=split_line(p,list_line,3);
    printf("<TR><TD align=\"right\">%d</TD><TD>",cislo);
    html_puts(list_line[0].p,list_line[0].l);
    printf("</TD><TD>");
    html_puts(list_line[1].p,list_line[1].l);
    printf("</TD></TR>\n");
    cislo++;
  }
  /* Jestlize zbývají volná místa, vypsat prázdné rádky */
  for(;cislo<=len;cislo++)
    printf("<TR><TD align=\"right\">%d</TD><TD>&nbsp;</TD>"
	"<TD>&nbsp;</TD></TR>\n",cislo);
  printf("</TABLE>\n");
  /* Vypsat formulár pro zadávání a rusení polozek seznamu */
  p=getenv("REQUEST_URI");
  printf("<FORM action=\"%s\" method=\"post\">\n<TABLE>\n",p?p:"");
  printf("<TR><TD colspan=2><B>Pridat do seznamu</B></TD></TR>\n");
  printf("<TR><TD>Jméno</TD><TD><INPUT type=\"text\" name=\"Jmeno\""
      "size=\"" SZ_JMENO_STR "\" maxlength=\"" SZ_JMENO_STR "\"></TD></TR>\n");
  printf("<TR><TD>Príjmení</TD><TD>"
      "<INPUT type=\"text\" name=\"Prijmeni\" "
      "size=\"" SZ_PRIJMENI_STR "\" maxlength=\"" SZ_PRIJMENI_STR "\">"
      "</TD></TR>\n");
  printf("<TR><TD>Poznámka</TD><TD><INPUT type=\"text\" name=\"Poznamka\" "
      "size=\"" SZ_POZNAMKA_INPUT_STR
      "\" maxlength=\"" SZ_PRIJMENI_STR "\"></TD></TR>\n");
  printf("<TR><TD><INPUT type=\"submit\" name=\"Odeslat\" value=\"Pridat\">"
      "</TD></TR>\n");
  printf("<TR><TD colspan=2><B>Smazat ze seznamu</B></TD></TR>\n");
  printf("<TR><TD>Heslo</TD><TD><INPUT type=\"text\" name=\"Heslo\" "
      "size=\"" SZ_HESLO_STR "\" maxlength=\"" SZ_HESLO_STR "\">"
      "</TD></TR>\n");
  printf("<TR><TD><INPUT type=\"submit\" name=\"Odeslat\" value=\"Smazat\">"
      "</TD></TR>\n");
  printf("</TABLE>\n</FORM>\n");
  /* Ukoncit HTML dokument a zavrít soubor se seznamem */
  printf("</BODY>\n</HTML>\n");
  if(addr)
    munmap((void*)addr,flen);
  close(fd);
}

/*** Prevod jedné hexadekadické císlice na císlo ***/

int hex2int(char hex)
{
  return hex>='0'&&hex<='9' ? hex-'0' : toupper(hex)-'A'+10;
}

/*** Precíst polozku name (name musí koncit znakem '=') z formuláre nacteného
 * v char_buf; max. len znaku ulozí do val; kdyz colon!=0, prekóduje znaky :
 * (oddelovace polozek) na ? ***/

void get_form_field(const char *name,char *val,int len,int colon)
{
  char *p;
  /* Najít polozku */
  if(!(p=strstr(char_buf,name))) {
    memcpy(char_buf,name,strlen(name));
    char_buf[strlen(name)-1]='\0';
    err_msg("Ve formulári chybí polozka \"%s\"",name);
  }
  /* Precíst hodnotu */
  p+=strlen(name);
  while(*p!='&'&&*p!='\0'&&len>0) {
    if(*p=='%'&&p[1]!='\0'&&p[2]!='\0') {
      *val=16*hex2int(p[1])+hex2int(p[2]);
      p+=3;
    } else
      *val=*p++;
    /* Prekódovat znaky 'S', 's', 'T', 't', 'Z', 'z' z Win1250 do ISO-8859-2 */
    switch(*val) {
      case (char)0x8a:
	*val='S';
	break;
      case (char)0x8d:
	*val='T';
	break;
      case (char)0x8e:
	*val='Z';
	break;
      case (char)0x9a:
	*val='s';
	break;
      case (char)0x9d:
	*val='t';
	break;
      case (char)0x9e:
	*val='z';
	break;
    }
    if(colon&&*val==':')
      *val='?';
    val++;
    len--;
  }
}

/*** Vygenerovat heslo (o délce max. HESLO_SZ) pro polozku daného císla ***/

void gen_heslo(char *heslo,int cislo)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  sprintf(heslo,"%d%d",cislo%100,(int)(tv.tv_sec%10000));
}

/*** Metoda GET - vypsat seznam se jménem $QUERY_STRING ***/

void handler_get()
{
  const char *conf_addr,*list,*env;
  int conf_fd,conf_len;
  /* Otevrít konfiguracní soubor a podle jména v QUERY_STRING najít rádek s
   * definicí seznamu, zobrazit seznam a zavrít konfiguracní soubor */
  open_conf(&conf_fd,&conf_addr,&conf_len);
  if(!(env=getenv("QUERY_STRING")))
    err_msg("Missing QUERY_STRING");
  if(!(list=get_list(conf_addr,conf_len,env)))
    err_msg("Seznam \"%s\" není definován",env);
  show_list(list);
  munmap((void*)conf_addr,conf_len);
  close(conf_fd);
}

/*** Metoda POST - zpracovat formulár, pridat nebo ubrat polozku seznamu ***/

void handler_post()
{
  char *len_s,*pbuf;
  int len,sz,pridat;
  const char *conf_addr,*list,*env;
  int conf_fd,conf_len;
  const char *list_title,*list_file;
  int list_title_l,list_len,list_file_l;
  int old_fd,old_len,new_fd;
  const char *old_addr,*line,*new_line;
  struct utsname host;
  char new_name[CHAR_BUF_SZ];
  int cislo,dupl=0,deleted=0;
  fields_t list_line[4];
  /* Nacíst data z formuláre */
  if(!(len_s=getenv("CONTENT_LENGTH")))
    err_msg("Missing CONTENT_LENGTH");
  len=atoi(len_s);
  if(len>=CHAR_BUF_SZ) /* Potrebujeme místo na záverecný znak '\0' */
    err_msg("CONTENT_LENGTH too large");
  if(len<=0)
    err_msg("Invalid CONTENT_LENGTH");
  pbuf=char_buf;
  while(len>0) {
    if((sz=read(0,pbuf,len))==-1)
      err_msg("Form data: %s",strerror(errno));
    if(sz==0)
      err_msg("Form data: EOF");
    pbuf+=sz;
    len-=sz;
  }
  *pbuf='\0'; /* Ukoncit prectená data */
  /* Zpracovat data z formuláre */
  if(strstr(char_buf,"&Odeslat=P")) /* Tady radsi porovnání koncí pred 'r'kvuli
				       mozným problémum s prekódováním */
    pridat=1;
  else
    if(strstr(char_buf,"&Odeslat=Smazat"))
      pridat=0;
    else
      err_msg("Pridat/smazat polozku seznamu???");
  if(pridat) { /* Pridávání - precíst jméno, príjmení a poznámku */
    get_form_field("Jmeno=",jmeno,SZ_JMENO,1);
    get_form_field("Prijmeni=",prijmeni,SZ_PRIJMENI,1);
    get_form_field("Poznamka=",poznamka,SZ_POZNAMKA,0);
  } else /* Mazání - precíst heslo */
    get_form_field("Heslo=",heslo,SZ_HESLO,1);
  /* Otevrít konfiguracní soubor a podle jména v QUERY_STRING najít rádek s
   * definicí seznamu */
  open_conf(&conf_fd,&conf_addr,&conf_len);
  if(!(env=getenv("QUERY_STRING")))
    err_msg("Missing QUERY_STRING");
  if(!(list=get_list(conf_addr,conf_len,env)))
    err_msg("Seznam \"%s\" není definován",env);
  get_list_attr(list,&list_title,&list_title_l,&list_len,&list_file,
      &list_file_l);
  /* Otevrít a namapovat soubor se seznamem, vytvorit nový soubor pro upravený
   * seznam */
  if(uname(&host)==-1)
    err_msg("uname(): %s\n",strerror(errno));
  if(list_file_l>=CHAR_BUF_SZ+16+strlen(host.nodename))
    err_msg("Prílis dlouhé jméno souboru se seznamem");
  memcpy(new_name,list_file,list_file_l);
  char_buf[list_file_l]='\0';
  open_list(new_name,&old_fd,&old_addr,&old_len);
  sprintf(new_name+strlen(new_name),".%s.%d",host.nodename,(int)getpid());
  if((new_fd=open(new_name,O_WRONLY|O_CREAT|O_EXCL,0600))==-1)
    err_msg("%s: %s",new_name,strerror(errno));
  /* Projít seznam a vytvorit upravenou verzi */
  for(line=old_addr,cislo=1;line<old_addr+old_len;cislo++) {
    new_line=split_line(line,list_line,4);
    if(pridat&&strlen(jmeno)==list_line[0].l&&
	!memcmp(jmeno,list_line[0].p,list_line[0].l)&&
	strlen(prijmeni)==list_line[1].l&&
	!memcmp(prijmeni,list_line[1].p,list_line[1].l))
      dupl=1; /* Zkontrolovat, ze pridávaný záznam jeste není v seznamu */
    if(pridat||strlen(heslo)!=list_line[2].l||
	memcmp(heslo,list_line[2].p,list_line[2].l)) {
      /* Kdyz se pridává nebo nesouhlasí heslo pri mazání, zkopírovat rádek */
      if(write(new_fd,line,new_line-line)==-1) {
	close(new_fd);
	unlink(new_name);
	err_msg("stdout: %s",strerror(errno));
      }
    } else
      if(!pridat) { /* Smazat tento záznam a poznamenat, co bylo smazáno */
	deleted=1;
	memcpy(jmeno,list_line[0].p,list_line[0].l);
	jmeno[list_line[0].l]='\0';
	memcpy(prijmeni,list_line[1].p,list_line[1].l);
	prijmeni[list_line[1].l]='\0';
      }
    line=new_line;
  }
  if(pridat&&cislo<=list_len&&!dupl) { /* Pripsat nový záznam na konec */
    gen_heslo(heslo,cislo);
    if(write(new_fd,jmeno,strlen(jmeno))==-1||write(new_fd,":",1)==-1||
	write(new_fd,prijmeni,strlen(prijmeni))==-1||write(new_fd,":",1)==-1||
	write(new_fd,heslo,strlen(heslo))==-1||write(new_fd,":",1)==-1||
	write(new_fd,poznamka,strlen(poznamka))==-1||write(new_fd,"\n",1)==-1)
      {
	close(new_fd);
	unlink(new_name);
	err_msg("stdout: %s",strerror(errno));
      }
  }
  /* Zavrít starý i nový seznam, prejmenovat nový na starý */
  close(new_fd);
  if(old_addr)
    munmap((void*)old_addr,old_len);
  close(old_fd);
  memcpy(char_buf,list_file,list_file_l); /* Pri vytvárení new_name jsme uz 
			           overili, ze se retezec do char_buf vejde */
  char_buf[list_file_l]='\0';
  if(rename(new_name,char_buf)==-1) {
    unlink(new_name);
    err_msg("rename(\"%s\",\"%s\": %s",new_name,char_buf,strerror(errno));
  }
  /* Zpracovat prípadné chyby pri pridávání/mazání v seznamu */
  if(pridat&&dupl)
    err_msg("\"%s %s\" uz je v seznamu",jmeno,prijmeni);
  if(pridat&&cislo>list_len)
    err_msg("Seznam je zaplnen");
  if(!pridat&&!deleted)
    err_msg("Chybné heslo \"%s\"",heslo);
  /* Upravit promenné pro následné zobrazení seznamu */
  if(!pridat)
    heslo[0]='\0'; /* show_list() pozná podle prázdného hesla a neprázdného
		      jména, ze byl smazán záznam */
  /* Zobrazit upravený seznam */
  if(!pridat)
    heslo[0]='\0'; /* Tímto show_list() rozlisí pridání a smazání záznamu */
  show_list(list);
  /* Zavrít konfiguracní soubor */
  munmap((void*)conf_addr,conf_len);
  close(conf_fd);
}

/*** Hlavní program ***/

int main(int argc,char *argv[])
{
  char *method;
  doc_header(); /* Vygenerovat HTTP hlavicku */
  if(!(method=getenv("REQUEST_METHOD"))) /* Mozné HTTP metody */
    err_msg("Missing REQUEST_METHOD");
  if(!strcmp(method,"GET")) /* Metoda GET - vypsat seznam se jménem
			       $QUERY_STRING */
    handler_get();
  else
    if(!strcmp(method,"POST")) /* Metoda POST - zpracovat formulár */
      handler_post();
    else
      err_msg("Unknown REQUEST_METHOD=\"%s\"",method);
  exit(0);
}
