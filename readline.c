/* Pouzití knihovny GNU readline - editace rádku, doplnování slov, historie */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#define PROMPT ">> "

const char *complete[]={"This","document","describes","the","GNU","Readline",
  "Library","utility","which","consistency","of","user","interface","across",
  "discrete","programs","that","need","to","provide","a","command","line",
  "interface"};

char *completion(char *text,int state)
{
  static int idx;
  int sz=sizeof(complete)/sizeof(char*);
  char *p;
  if(state==0)
    idx=0;
  while(idx<sz&&(strlen(text)>strlen(complete[idx])||
      memcmp(text,complete[idx],strlen(text))))
    idx++;
  if(idx>=sz)
    return NULL;
  else {
    if(!(p=malloc(strlen(complete[idx])+1))) {
      perror("malloc()");
      exit(1);
    }
    strcpy(p,complete[idx++]);
   }
  return p;
}

void process_line(char *line)
{
  add_history(line);
  puts(line);
}

int main(void)
{
  char *prompt=PROMPT;
  char *line;
  using_history();
  rl_completion_entry_function=(Function *)completion;
  for(;;) {
    if(!(line=readline(prompt))||line[0]=='\0')
      break;
    process_line(line);
    free(line);
  }
  exit(0);
}
