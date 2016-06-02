/* Triviální príklad pouzití knihovny curses - v levém horním rohu je oktalový
   kód stisknuté klávesy a souradnice kurzoru, sipky posouvají kurzor,
   tabulátor následovaný sipkou zmení smer psaní, BACKSPACE a DELETE smaze
   obrazovku, ENTER ukoncí program. */

#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <assert.h>
#include <ctype.h>
#include <locale.h>

#define TRY(x) (assert((x)!=ERR))
#define TRYP(x) (assert((x)!=NULL))

void cleanup_curses()
{
  endwin();
}

int cur_x()
{
  int x,y;
  TRY(getyx(stdscr,y,x));
  return x;
}

int cur_y()
{
  int x,y;
  TRY(getyx(stdscr,y,x));
  return y;
}

void advance_cur(int y,int x)
{
  int ox,oy;
  TRY(getyx(stdscr,oy,ox));
  x+=ox;
  y+=oy;
  if(x>=0&&x<COLS&&y>=1&&y<LINES)
    TRY(move(y,x));
}

int main(int argc,char *argv[])
{
  int dir_x=1,dir_y=0;
  int c_x,c_y;
  int set_dir=0;
  int in_char;
  setlocale(LC_ALL,""); /* Toto je tu kvuli pouzití funkce isprint() */
  atexit(cleanup_curses);
  TRYP(initscr());
  TRY(cbreak());
  TRY(noecho());
  TRY(intrflush(stdscr,FALSE));
  TRY(keypad(stdscr,TRUE));
  TRY(mvprintw(0,0,"[0???] (y=1) (x=0)"));
  TRY(move(1,0));
  TRY(doupdate());
  for(;;) {
    TRY(in_char=getch());
    c_x=cur_x();
    c_y=cur_y();
    switch(in_char) {
      case KEY_ENTER:
      case '\n':
      case '\r':
	TRY(refresh());
	exit(0);
	break;
      case '\t':
	set_dir=1;
	break;
      case KEY_BACKSPACE:
      case 0177:
	TRY(clear());
	TRY(move(c_y,c_x));
	break;
      case KEY_UP:
	if(set_dir) {
	  dir_x=0;
	  dir_y=-1;
	  set_dir=0;
	}
	else
	  advance_cur(-1,0);
	break;
      case KEY_DOWN:
	if(set_dir) {
	  dir_x=0;
	  dir_y=1;
	  set_dir=0;
	}
	else
	  advance_cur(1,0);
	break;
      case KEY_LEFT:
	if(set_dir) {
	  dir_x=-1;
	  dir_y=0;
	  set_dir=0;
	}
	else
	  advance_cur(0,-1);
	break;
      case KEY_RIGHT:
	if(set_dir) {
	  dir_x=1;
	  dir_y=0;
	  set_dir=0;
	}
	else
	  advance_cur(0,1);
	break;
      default:
	if(isprint(in_char)) {
	  TRY(addch(in_char));
	  TRY(move(c_y,c_x));
	  advance_cur(dir_y,dir_x);
	}
	break;
    }
    c_x=cur_x();
    c_y=cur_y();
    TRY(mvprintw(0,0,"[0%03o] (y=%d) (x=%d)",in_char,c_y,c_x));
    TRY(move(c_y,c_x));
    TRY(refresh());
  }
}
