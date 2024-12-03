// vNesC.h

/* 64x64   - min */
/* 256x240 - max */
#define VNES_VIEWPORT_WIDTH  256
#define VNES_VIEWPORT_HEIGHT 240

#if defined(USE_SDL2)
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define true 1
#define false 0
#define mfree free
#elif defined(__P2K__)
#include <mem.h>
#include <typedefs.h>
#endif

#define boolean char
#define byte signed char
#define null 0

#define b_int_array1d_static_fld_length 8 //?
#define d_byte_array1d_static_fld_length 256
#define h_int_array1d_static_fld_length 276
#define c_int_array1d_static_fld_length 260

////////////////////
/*
void debsave(); /////////DEBUG
char debarea[65535], *deb=debarea;

void debsave(){
  if(deb!=debarea){
  FILE *f;
  if(deb==debarea) return;
  //f=ffopen("0:\\ZBin\\nes\\log",A_ReadWrite+A_BIN+A_Create+A_Append,P_READ+P_WRITE,&err);
  //if (f==-1) return;
  //ffwrite(f,(unsigned char*)debarea,deb-debarea,&err);
  //ffclose(f,&err);
  f=fopen("out.txt","wb");
  fwrite(debarea,deb-debarea,1,f);
  fclose(f);
  deb=debarea;
  }
}

int nnn=0;

void debug(char *s, int num, int val){
  if(deb-debarea<sizeof(debarea)-100){
  sprintf(deb,"\n%s  %d = %d",s,num,val);
  deb+=strlen(deb);
  }
  if(++nnn==100) debsave();
}
////////////////

void savefile(char *fname, void *buf, int size);
*/
void initnul();
void freeall();
void init_values();
extern void repaint();

extern int getWidth, getHeight, screen_length;
extern int A; /* Frameskip: 1-9 */
extern boolean t_boolean_static_fld; /* Optimize: 0-1 */

typedef unsigned short ushort;

extern ushort *screens;
#define sbufsize 8192
extern unsigned char sbuf[];
extern unsigned char *ebuf;

#define outputstreamwrite bytearrayoutputstreamwrite
#define inputstreamread bytearrayinputstreamread

///////////////////////////
void run_step() ;
void keyPressed(int i1) ;
void keyReleased(int i1) ;
boolean loadrom(char *s) ;
char *loadfile(char *s, int *loadfilesize) ;
///////
//void savescreen(char *fname);

#define b_int_array1d_static_fld_length 8
#define e_int_array1d_static_fld_length 32
#define i_int_array1d_static_fld_length 12
#define l_int_array1d_static_fld_length 8
#define d_byte_array1d_static_fld_length 256
#define d_int_array1d_static_fld_length 8 //???4

extern void Systemarraycopy(void *from, int foff, void *to, int toff, int size);

#if defined(SAVE_LOAD)
extern FILE *openstream(int savemode);
extern void bytearrayoutputstreamwrite(int byt);
extern int bytearrayinputstreamread();
extern void closestream();
#endif

void *MEM_Alloc_HUGE(int mem_size);
void MEM_Free_HUGE(void *ptr);
