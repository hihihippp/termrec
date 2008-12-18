#include "config.h"
#define _XOPEN_SOURCE_EXTENDED
#include <ncursesw/curses.h>
#include <locale.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vt100.h>
#include "export.h"


#define WIN ((WINDOW*)vt->l_data)

static void vtcur_cursor(vt100 vt, int x, int y)
{
    wmove(WIN, y, x);
}

static void setchar(cchar_t *cc, ucs ch, int attr)
{
    wchar_t cstr[2];
    int f, b;
    int cattr;
    
    f=attr&0xff;
    if (f==0xff)
        f=7;
    else
        f&=7;
    
    b=(attr>>8)&0xff;
    if (b==0xff)
        b=0;
    else
        b&=7;
    
    cattr=0;
    if (attr&VT100_ATTR_BOLD)
        cattr|=A_BOLD;
    if (attr&VT100_ATTR_DIM)
        cattr|=A_DIM;
    if (attr&VT100_ATTR_UNDERLINE)
        cattr|=A_UNDERLINE;
    if (attr&VT100_ATTR_BLINK)
        cattr|=A_BLINK;
    if (attr&VT100_ATTR_INVERSE)
        cattr|=A_REVERSE;
    
    cstr[0]=ch;
    cstr[1]=0;
    setcchar(cc, cstr, cattr, 7-f+b*8, 0);
}

static void vtcur_char(vt100 vt, int x, int y, ucs ch, int attr)
{
    cchar_t cc;
    
    setchar(&cc, ch, attr);
    mvwadd_wch(WIN, y, x, &cc);
}

export void vtcur_dump(vt100 vt)
{
    int x,y;
    attrchar *ch;
    int sx, sy;
    
    getmaxyx(WIN, sy, sx);
    if (sx>vt->sx)
        sx=vt->sx;
    if (sy>vt->sy)
        sy=vt->sy;
    
    ch=vt->scr;
    for(y=0; y<sy; y++)
    {
        for(x=0; x<sx; x++)
        {
            vtcur_char(vt, x, y, ch->ch, ch->attr);
            ch++;
        }
        ch+=vt->sx-sx;
    }
}

static void vtcur_clear(vt100 vt, int x, int y, int len)
{
    cchar_t cc;
    
    setchar(&cc, ' ', vt->attr);
    
    while(len--)
    {
        mvwadd_wch(WIN, y, x, &cc);
        if (++x>=vt->sx)
            x=0, y++;
    }
}

static void vtcur_scroll(vt100 vt, int nl)
{
    wsetscrreg(WIN, vt->s1, vt->s2-1);
    wscrl(WIN, nl);
}


static void vtcur_flag(vt100 vt, int f, int v)
{
    switch(f)
    {
    case VT100_FLAG_CURSOR:
        curs_set(v);
        break;
    }
}

static void vtcur_resized(vt100 vt, int sx, int sy)
{
}

static void vtcur_flush(vt100 vt)
{
    vtcur_cursor(vt, vt->cx, vt->cy);
    wrefresh(WIN);
}

static void vtcur_free(vt100 vt)
{
}

export void vtcur_resize(vt100 vt, int sx, int sy)
{
}

export void vtcur_attach(vt100 vt, WINDOW *win, int dump)
{
    vt->l_data=win;
    
    vt->l_char=vtcur_char;
    vt->l_cursor=vtcur_cursor;
    vt->l_clear=vtcur_clear;
    vt->l_scroll=vtcur_scroll;
    vt->l_flag=vtcur_flag;
    vt->l_resize=vtcur_resized;
    vt->l_flush=vtcur_flush;
    vt->l_free=vtcur_free;
    
    if (dump)
    {
        vtcur_dump(vt);
        vtcur_flush(vt);
    }
}

void init_curses()
{
    int f,b;
    
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    
    for(f=0;f<8;f++)
        for(b=0;b<8;b++)
            init_pair(f+b*8, 7-f, b);
}
