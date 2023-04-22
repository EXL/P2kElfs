/*
Copyright (C) 2003 - Derek John Evans 

This file is part of Yeti3D Portable Engine

Yeti3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 2003 - Derek J. Evans <derek@theteahouse.com.au>
Prepared for public release: 10/24/2003 - Derek J. Evans <derek@theteahouse.com.au>
*/

/*
** Name: Yeti3D
** Desc: GBA port 
** Auth: Derek Evans
**
** Copyright (C) 2003, Derek Evans
**
** YY  YY EEEEEE TTTTTT IIIIII 33333  DDDDD
** YY  YY EE       TT     II       33 DD  DD
**  YYYY  EEEE     TT     II     333  DD  DD
**   YY   EE       TT     II       33 DD  DD
**   YY   EEEEEE   TT   IIIIII 33333  DDDDD
*/

#include "../../src/yeti.h"
#include "../../src/font.h"
#include "gba.h"

IN_EWRAM yeti_t yeti;

int min(int a, int b)
{
  return MIN(a, b);
}

int max(int a, int b)
{
  return MAX(a, b);
}

typedef struct 
{
  int Sx, l, u, v;
} edge_t;

typedef struct 
{
	u16 attribute0;
	u16 attribute1;
	u16 attribute2;
	u16 attribute3;
} OAMEntry;

edge_t EdgeLeft[200];
edge_t EdgeRight[200];

/*
** Name: yeti_putc
** Desc: Puts a given character onto the screen.
*/
void yeti_putc(int x, int c)
{
  ((OAMEntry*)0x7000000)[x].attribute2 = ((c - ' ') << 1) + 512;
}

/*
** Name: yeti_putc
** Desc: Puts a given string onto the screen.
*/
void yeti_puts(int x, char* s)
{
  while (*s) yeti_putc(x++, *s++);
}

void itos(int i, char* s)
{
//#define DIGITS(X) if(i>=X){c=0;do{i-=X;c++;}while(i >= X);*s++='0'+c;} 
#define DIGITS(X) for(c=0;i>=X;i-=X,c++);*s++='0'+c;

  int c;
  DIGITS(100000); DIGITS(10000); DIGITS(1000); DIGITS(100); DIGITS(10);
  *s++ = '0' + i;
  *s = 0;
}

void gba_init(void)
{
  int i;
  
  REG_DISPCNT_L = 0x1445;
  REG_BG2PA     = 171;
  REG_BG2PD     = 206;

  // Setup the sprites to be used for overlayed text.
  for(i = 0; i < 128; i++)
  {
    ((OAMEntry*)0x7000000)[i].attribute0 = ((i / 30) + 16) << 3 | 0x2000;
    ((OAMEntry*)0x7000000)[i].attribute1 = ((i % 30) +  0) << 3;
    ((OAMEntry*)0x7000000)[i].attribute2 = 0;
    ((OAMEntry*)0x7000000)[i].attribute3 = 0;
  }
	// Copy the rom font into sprite ram.
	memcpy((void*)0x6014000, fontData, 91 * sizeof(fontchar_t));
  // Copy our rom palette into palette ram.
  memcpy((void*)0x5000200, palette, 256 * 2);

  ((u16*)0x5000200)[255] = RGB_SET(31, 31, 0);

  REG_TM2CNT = 0x80;
  REG_TM3CNT = 0x84;
}

void gba_flip(void)
{
  int i;
  framebuffer_t* temp;

  i = ~(*(u8*)0x04000130);

  yeti.keyboard.a      = i &   1;
  yeti.keyboard.b      = i &   2;
  yeti.keyboard.select = i &   4;
  yeti.keyboard.right  = i &  16;
  yeti.keyboard.left   = i &  32;
  yeti.keyboard.up     = i &  64;
  yeti.keyboard.down   = i & 128;

  i = ~(*(u8*)0x04000131);

  yeti.keyboard.l      = i & 1;
  yeti.keyboard.r      = i & 2;

  // Flip the frame buffers.
  *(u8*)0x04000000 ^= 0x10;
  temp = yeti.viewport.back;
  yeti.viewport.back = yeti.viewport.front;
  yeti.viewport.front = temp;
}

int main(void)
{
  char s[10];
  u16 real = 0;

  gba_init();
  yeti_init(&yeti, (framebuffer_t*) 0x06000000, (framebuffer_t*) 0x0600A000, textures, palette, lua);
  game_init(&yeti);

  yeti_puts(60 + 6, "Yeti3D Engine (GBA)");
  yeti_puts(90 + 5, "(C) 2003 Derek/Torlus");
  //yeti_puts(82, "FPS:");

#define timelag() (int)((*(volatile u16*)0x400010C) - real)

  while (!yeti.keyboard.select)
  {
    game_tick(&yeti);
    game_draw(&yeti);

    for (real += YETI_VIEWPORT_INTERVAL; timelag() > YETI_VIEWPORT_INTERVAL_ERROR; real += YETI_VIEWPORT_INTERVAL)
    {
      game_tick(&yeti);
    }

    //itos(yeti.tick * 1000 / (*(volatile u16*)0x400010C), s);
    //yeti_puts(0, s);
        
    while (timelag() < -YETI_VIEWPORT_INTERVAL_ERROR);

    gba_flip();
  }
  return 0;
}
