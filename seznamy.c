/* CGI program pro zapisov�n� lid� do seznamu */

/* Zobraz� seznam se zadan�m n�zvem a poctem kolonek. Do seznamu se muze
 * kdokoliv zapsat az do vycerp�n� kapacity (jm�no, pr�jmen�, pozn�mka -
 * libovoln� text, nezobrazuje se). Kazd� dostane pridelen� heslo, kter�m se
 * muze ze seznamu vymazat. Konfiguracn� soubor $PATH_TRANSLATED.conf definuje
 * pro kazd� n�zev seznamu jeho d�lku, soubor s obsahem a nadpis na r�dku
 * tvaru: n�zev:nadpis:d�lka:soubor
 * Obsah seznamu je ulozen v souboru s r�dky tvaru:
 * jm�no:pr�jmen�:heslo:pozn�mka 
 */

#define _XOPEN_SOURCE 500

#if defined(__sun) && defined(__svr4__)
#undef _XOPEN_SOURCE
#endif /* Protoze Solaris definuje struct timeval, pouze kdyz nen� definov�no
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

/*** Jednotliv� pole formul�re ***/

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

/*** V�ce�celov� buffer pro retezce (pro voln� pouzit�, nelze spol�hat na
 * zachov�n� obsahu) ***/

#define CHAR_BUF_SZ 4096

char char_buf[CHAR_BUF_SZ];

/*** HTTP hlavicka ***/

void doc_header()
{
  printf("Content-type: text/html;charset=iso-8859-2\n\n");
}

/*** Prek�dov�n� 1 znaku pro HTML (n�hrada <, >, &) ***/

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

/*** Vypsat text upraven� pro HTML (n�hrada speci�ln�ch znaku) ***/

void html_puts(const char *text,int len)
{
  for(;len>0;len--)
    fputs(html_char(*text++),stdout);
}

/*** Vypsat HTML dokument s chybov�m hl�sen�m a konec programu ***/

void err_msg(const char *msg,...)
{
  va_list val;
  static char err_buf[CHAR_BUF_SZ];
  printf("<HTML>\n<HEAD>\n<TITLE>Chyba</TITLE>\n</HEAD>\n<BODY>\n"
      "<H1>Pri zpracov�n� nastala chyba</H1>\n<P><TT>");
  va_start(val,msg);
#if defined(__sun) && defined(__svr4__)
  vsprintf(err_buf,msg,val); /* Na Solarisu chyb� vsnprintf() */
#else
  vsnprintf(err_buf,CHAR_BUF_SZ,msg,val);
#endif
  html_puts(err_buf,strlen(err_buf));
  va_end(val);
  printf("</TT></P>\n</BODY>\n</HTML>\n");
  exit(1);
}

/*** Otevr�t a namapovat do pameti konfiguracn� soubor, vrac� deskriptor,
 * adresu mapov�n� a d�lku souboru ***/

void open_conf(int *fd,const char **addr,int *len)
{
  char *env,*p;
  /* Z PATH_TRANSLATED z�skat cestu ke konfiguracn�mu souboru */
  if(!(env=getenv("PATH_TRANSLATED")))
    err_msg("Missing PATH_TRANSLATED");
  if(strlen(env)>=CHAR_BUF_SZ-8) /* Budeme retezec prodluzovat o ".conf" */
    err_msg("Value of PATH_TRANSLATED too long");
  strcpy(char_buf,env);
  /* Nejprve se odr�znou pr�padn� pr�pony oddelen� teckami */
  if(!(p=strrchr(char_buf,'/')))
    p=char_buf;
  if(!(p=strchr(p,'.')))
    p=char_buf+strlen(char_buf);
  /* a pripoj� se pr�pona ".conf" */
  strcpy(p,".conf");
  /* Otevr�t soubor, zjistit d�lku a namapovat */
  if((*fd=open(char_buf,O_RDONLY))==-1)
    err_msg("%s: %s",char_buf,strerror(errno));
  if((*len=lseek(*fd,0,SEEK_END))==-1)
    err_msg("%s: %s",char_buf,strerror(errno));
  if(len==0)
    err_msg("Pr�zdn� konfiguracn� soubor \"%s\"",char_buf);
  if((*addr=(char *)mmap(0,*len,PROT_READ,MAP_SHARED,*fd,0))==MAP_FAILED)
    err_msg("mmap(\"%s\"): %s",char_buf,strerror(errno));
  if((*addr)[*len-1]!='\n')
    err_msg("Chyb� konec r�dku na konci konfiguracn�ho souboru \"%s\"",
	char_buf);
}

/*** Otevr�t a namapovat do pameti (kdyz je nepr�zdn�) soubor se seznamem ***/

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
    err_msg("Chyb� konec r�dku na konci souboru \"%s\"",
	char_buf); 
}

/*** Proj�t konfiguracn� soubor o d�lce len namapovan� na adrese addr, vr�tit
 * adresu r�dku s definic� seznamu se jm�nem name ***/

const char *get_list(const char *conf_addr,int len,const char *name)
{
  int n_l=strlen(name);
  while(len>n_l) {
    if(!strncmp(name,conf_addr,n_l)&&conf_addr[n_l]==':')
      return conf_addr; /* Nalezen spr�vn� r�dek */
    while(*conf_addr++!='\n') /* Tento r�dek to nen�, jdi na zac�tek dals�ho */
      len--;
    len--;
  }
  return NULL;
}

/*** Prec�st r�dek souboru (konfiguracn�ho nebo obsahu seznamu), napln�
 * nfields polozek (adresa, d�lka) oddelen�ch dvojteckou, posledn� polozka
 * pokracuje az do konce r�dku (muze obsahovat dvojtecky). Vrac� adresu
 * zac�tku n�sleduj�c�ho r�dku souboru ***/

typedef struct { const char *p;int l;} fields_t;

const char *split_line(const char *line,fields_t *fields,int nfields)
{
  int nf;
  for(nf=0;nf<nfields;nf++) {
    fields[nf].p=line;
    fields[nf].l=0;
  }
  nf=0;
  for(;;) /* Pri otevren� souboru bylo zkontrolov�no, ze na konci
	     souboru je konec r�dku */
    switch(*line++) {
      case '\n':
	if(nf<nfields-1)
	  err_msg("split_line(): chybn� form�t souboru");
	return line;
      case ':':
	if(nf<nfields-1) {
	  nf++;
	  fields[nf].p=line; /* Tady uz line ukazuje na znak za dvojteckou */
	  fields[nf].l=0;
	  break;
	}
	/* V posledn� polozce mohou b�t dals� dvojtecky */
      default:
	fields[nf].l++;
	break;
    }
}

/*** Prec�st nadpis, d�lku a jm�no souboru z r�dku konfiguracn�ho souboru ***/

void get_list_attr(const char *line,const char **title,int *title_l,int *len,
    const char **file,int *file_l)
{
  fields_t fields[4];
  split_line(line,fields,4);
  *title=fields[1].p;
  *title_l=fields[1].l;
  if(fields[2].l>=CHAR_BUF_SZ)
    err_msg("Chybn� d�lka seznamu v konfiguracn�m souboru");
  memcpy(char_buf,fields[2].p,fields[2].l);
  char_buf[fields[2].l]='\0';
  *len=atoi(char_buf);
  *file=fields[3].p;
  *file_l=fields[3].l;
}

/*** Vypsat seznam, parametr list ukazuje na r�dek konfiguracn�ho souboru s
 * definic� seznamu ***/

void show_list(const char *list)
{
  const char *title,*file,*addr,*p;
  int title_l,len,file_l,fd,flen,cislo;
  fields_t list_line[3];
  /* Zjistit nadpis, pocet polozek a jm�no souboru pro seznam */
  get_list_attr(list,&title,&title_l,&len,&file,&file_l);
  if(file_l>=CHAR_BUF_SZ)
    err_msg("Pr�lis dlouh� jm�no souboru se seznamem");
  memcpy(char_buf,file,file_l);
  char_buf[file_l]='\0';
  /* Otevr�t a namapovat soubor */
  open_list(char_buf,&fd,&addr,&flen);
  /* Vytvorit zac�tek str�nky */
  if(title_l>=CHAR_BUF_SZ)
    err_msg("Pr�lis dlouh� nadpis seznamu");
  memcpy(char_buf,title,title_l);
  char_buf[title_l]='\0';
  printf("<HTML>\n<HEAD>\n<TITLE>%s</TITLE>\n</HEAD>\n<BODY>\n",char_buf);
  printf("<H1>%s</H1>\n",char_buf);
  /* Jestlize je heslo nepr�zdn�, byl vlozen nov� z�znam */
  if(heslo[0]!='\0') {
    printf("<P><B>Prid�no do seznamu</B><BR>\n"
	"<B>Jm�no: </B>");
    html_puts(jmeno,strlen(jmeno));
    printf("<BR>\n<B>Pr�jmen�: </B>");
    html_puts(prijmeni,strlen(prijmeni));
    printf("<BR>\n<B>Heslo: </B>");
    html_puts(heslo,strlen(heslo));
    printf(" <B>(zad�v� se pri maz�n� polozky seznamu)</B><BR>\n"
	"<B>Pozn�mka: </B>");
    html_puts(poznamka,strlen(poznamka));
    printf("</P>\n");
  } else /* Pr�zdn� heslo, nepr�zdn� jm�no - byl zrusen z�znam */
    if(jmeno[0]!='\0') {
      printf("<P><B>Smaz�no ze seznamu</B><BR>\n"
	  "<B>Jm�no: </B>");
      html_puts(jmeno,strlen(jmeno));
      printf("<BR>\n<B>Pr�jmen�: </B>");
      html_puts(prijmeni,strlen(prijmeni));
      printf("</P>\n");
    }
  /* Z�hlav� tabulky */
  printf("<TABLE border cellpadding=\"3\">\n"
      "<TR><TD align=\"right\"><B>C.</B></TD><TD><B>Jm�no</B></TD>"
      "<TD><B>Pr�jmen�</B></TD></TR>\n");
  /* Vypsat jednotliv� polozky seznamu */
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
  /* Jestlize zb�vaj� voln� m�sta, vypsat pr�zdn� r�dky */
  for(;cislo<=len;cislo++)
    printf("<TR><TD align=\"right\">%d</TD><TD>&nbsp;</TD>"
	"<TD>&nbsp;</TD></TR>\n",cislo);
  printf("</TABLE>\n");
  /* Vypsat formul�r pro zad�v�n� a rusen� polozek seznamu */
  p=getenv("REQUEST_URI");
  printf("<FORM action=\"%s\" method=\"post\">\n<TABLE>\n",p?p:"");
  printf("<TR><TD colspan=2><B>Pridat do seznamu</B></TD></TR>\n");
  printf("<TR><TD>Jm�no</TD><TD><INPUT type=\"text\" name=\"Jmeno\""
      "size=\"" SZ_JMENO_STR "\" maxlength=\"" SZ_JMENO_STR "\"></TD></TR>\n");
  printf("<TR><TD>Pr�jmen�</TD><TD>"
      "<INPUT type=\"text\" name=\"Prijmeni\" "
      "size=\"" SZ_PRIJMENI_STR "\" maxlength=\"" SZ_PRIJMENI_STR "\">"
      "</TD></TR>\n");
  printf("<TR><TD>Pozn�mka</TD><TD><INPUT type=\"text\" name=\"Poznamka\" "
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
  /* Ukoncit HTML dokument a zavr�t soubor se seznamem */
  printf("</BODY>\n</HTML>\n");
  if(addr)
    munmap((void*)addr,flen);
  close(fd);
}

/*** Prevod jedn� hexadekadick� c�slice na c�slo ***/

int hex2int(char hex)
{
  return hex>='0'&&hex<='9' ? hex-'0' : toupper(hex)-'A'+10;
}

/*** Prec�st polozku name (name mus� koncit znakem '=') z formul�re nacten�ho
 * v char_buf; max. len znaku uloz� do val; kdyz colon!=0, prek�duje znaky :
 * (oddelovace polozek) na ? ***/

void get_form_field(const char *name,char *val,int len,int colon)
{
  char *p;
  /* Naj�t polozku */
  if(!(p=strstr(char_buf,name))) {
    memcpy(char_buf,name,strlen(name));
    char_buf[strlen(name)-1]='\0';
    err_msg("Ve formul�ri chyb� polozka \"%s\"",name);
  }
  /* Prec�st hodnotu */
  p+=strlen(name);
  while(*p!='&'&&*p!='\0'&&len>0) {
    if(*p=='%'&&p[1]!='\0'&&p[2]!='\0') {
      *val=16*hex2int(p[1])+hex2int(p[2]);
      p+=3;
    } else
      *val=*p++;
    /* Prek�dovat znaky 'S', 's', 'T', 't', 'Z', 'z' z Win1250 do ISO-8859-2 */
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

/*** Vygenerovat heslo (o d�lce max. HESLO_SZ) pro polozku dan�ho c�sla ***/

void gen_heslo(char *heslo,int cislo)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  sprintf(heslo,"%d%d",cislo%100,(int)(tv.tv_sec%10000));
}

/*** Metoda GET - vypsat seznam se jm�nem $QUERY_STRING ***/

void handler_get()
{
  const char *conf_addr,*list,*env;
  int conf_fd,conf_len;
  /* Otevr�t konfiguracn� soubor a podle jm�na v QUERY_STRING naj�t r�dek s
   * definic� seznamu, zobrazit seznam a zavr�t konfiguracn� soubor */
  open_conf(&conf_fd,&conf_addr,&conf_len);
  if(!(env=getenv("QUERY_STRING")))
    err_msg("Missing QUERY_STRING");
  if(!(list=get_list(conf_addr,conf_len,env)))
    err_msg("Seznam \"%s\" nen� definov�n",env);
  show_list(list);
  munmap((void*)conf_addr,conf_len);
  close(conf_fd);
}

/*** Metoda POST - zpracovat formul�r, pridat nebo ubrat polozku seznamu ***/

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
  /* Nac�st data z formul�re */
  if(!(len_s=getenv("CONTENT_LENGTH")))
    err_msg("Missing CONTENT_LENGTH");
  len=atoi(len_s);
  if(len>=CHAR_BUF_SZ) /* Potrebujeme m�sto na z�verecn� znak '\0' */
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
  *pbuf='\0'; /* Ukoncit precten� data */
  /* Zpracovat data z formul�re */
  if(strstr(char_buf,"&Odeslat=P")) /* Tady radsi porovn�n� konc� pred 'r'kvuli
				       mozn�m probl�mum s prek�dov�n�m */
    pridat=1;
  else
    if(strstr(char_buf,"&Odeslat=Smazat"))
      pridat=0;
    else
      err_msg("Pridat/smazat polozku seznamu???");
  if(pridat) { /* Prid�v�n� - prec�st jm�no, pr�jmen� a pozn�mku */
    get_form_field("Jmeno=",jmeno,SZ_JMENO,1);
    get_form_field("Prijmeni=",prijmeni,SZ_PRIJMENI,1);
    get_form_field("Poznamka=",poznamka,SZ_POZNAMKA,0);
  } else /* Maz�n� - prec�st heslo */
    get_form_field("Heslo=",heslo,SZ_HESLO,1);
  /* Otevr�t konfiguracn� soubor a podle jm�na v QUERY_STRING naj�t r�dek s
   * definic� seznamu */
  open_conf(&conf_fd,&conf_addr,&conf_len);
  if(!(env=getenv("QUERY_STRING")))
    err_msg("Missing QUERY_STRING");
  if(!(list=get_list(conf_addr,conf_len,env)))
    err_msg("Seznam \"%s\" nen� definov�n",env);
  get_list_attr(list,&list_title,&list_title_l,&list_len,&list_file,
      &list_file_l);
  /* Otevr�t a namapovat soubor se seznamem, vytvorit nov� soubor pro upraven�
   * seznam */
  if(uname(&host)==-1)
    err_msg("uname(): %s\n",strerror(errno));
  if(list_file_l>=CHAR_BUF_SZ+16+strlen(host.nodename))
    err_msg("Pr�lis dlouh� jm�no souboru se seznamem");
  memcpy(new_name,list_file,list_file_l);
  char_buf[list_file_l]='\0';
  open_list(new_name,&old_fd,&old_addr,&old_len);
  sprintf(new_name+strlen(new_name),".%s.%d",host.nodename,(int)getpid());
  if((new_fd=open(new_name,O_WRONLY|O_CREAT|O_EXCL,0600))==-1)
    err_msg("%s: %s",new_name,strerror(errno));
  /* Proj�t seznam a vytvorit upravenou verzi */
  for(line=old_addr,cislo=1;line<old_addr+old_len;cislo++) {
    new_line=split_line(line,list_line,4);
    if(pridat&&strlen(jmeno)==list_line[0].l&&
	!memcmp(jmeno,list_line[0].p,list_line[0].l)&&
	strlen(prijmeni)==list_line[1].l&&
	!memcmp(prijmeni,list_line[1].p,list_line[1].l))
      dupl=1; /* Zkontrolovat, ze prid�van� z�znam jeste nen� v seznamu */
    if(pridat||strlen(heslo)!=list_line[2].l||
	memcmp(heslo,list_line[2].p,list_line[2].l)) {
      /* Kdyz se prid�v� nebo nesouhlas� heslo pri maz�n�, zkop�rovat r�dek */
      if(write(new_fd,line,new_line-line)==-1) {
	close(new_fd);
	unlink(new_name);
	err_msg("stdout: %s",strerror(errno));
      }
    } else
      if(!pridat) { /* Smazat tento z�znam a poznamenat, co bylo smaz�no */
	deleted=1;
	memcpy(jmeno,list_line[0].p,list_line[0].l);
	jmeno[list_line[0].l]='\0';
	memcpy(prijmeni,list_line[1].p,list_line[1].l);
	prijmeni[list_line[1].l]='\0';
      }
    line=new_line;
  }
  if(pridat&&cislo<=list_len&&!dupl) { /* Pripsat nov� z�znam na konec */
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
  /* Zavr�t star� i nov� seznam, prejmenovat nov� na star� */
  close(new_fd);
  if(old_addr)
    munmap((void*)old_addr,old_len);
  close(old_fd);
  memcpy(char_buf,list_file,list_file_l); /* Pri vytv�ren� new_name jsme uz 
			           overili, ze se retezec do char_buf vejde */
  char_buf[list_file_l]='\0';
  if(rename(new_name,char_buf)==-1) {
    unlink(new_name);
    err_msg("rename(\"%s\",\"%s\": %s",new_name,char_buf,strerror(errno));
  }
  /* Zpracovat pr�padn� chyby pri prid�v�n�/maz�n� v seznamu */
  if(pridat&&dupl)
    err_msg("\"%s %s\" uz je v seznamu",jmeno,prijmeni);
  if(pridat&&cislo>list_len)
    err_msg("Seznam je zaplnen");
  if(!pridat&&!deleted)
    err_msg("Chybn� heslo \"%s\"",heslo);
  /* Upravit promenn� pro n�sledn� zobrazen� seznamu */
  if(!pridat)
    heslo[0]='\0'; /* show_list() pozn� podle pr�zdn�ho hesla a nepr�zdn�ho
		      jm�na, ze byl smaz�n z�znam */
  /* Zobrazit upraven� seznam */
  if(!pridat)
    heslo[0]='\0'; /* T�mto show_list() rozlis� prid�n� a smaz�n� z�znamu */
  show_list(list);
  /* Zavr�t konfiguracn� soubor */
  munmap((void*)conf_addr,conf_len);
  close(conf_fd);
}

/*** Hlavn� program ***/

int main(int argc,char *argv[])
{
  char *method;
  doc_header(); /* Vygenerovat HTTP hlavicku */
  if(!(method=getenv("REQUEST_METHOD"))) /* Mozn� HTTP metody */
    err_msg("Missing REQUEST_METHOD");
  if(!strcmp(method,"GET")) /* Metoda GET - vypsat seznam se jm�nem
			       $QUERY_STRING */
    handler_get();
  else
    if(!strcmp(method,"POST")) /* Metoda POST - zpracovat formul�r */
      handler_post();
    else
      err_msg("Unknown REQUEST_METHOD=\"%s\"",method);
  exit(0);
}
