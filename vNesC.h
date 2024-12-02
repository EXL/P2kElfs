// vNesC.h

#if defined(USE_SDL2)
#include <string.h>
#include <stdio.h>

#define mfree free
#endif

#define boolean char
#define byte signed char
#define true 1
#define false 0
#define null 0

#define b_int_array1d_static_fld_length 8 //?
#define d_byte_array1d_static_fld_length 256
#define h_int_array1d_static_fld_length 276
#define c_int_array1d_static_fld_length 260

extern char romname[256];
extern volatile int running;
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
void init_values();
extern volatile int running;
extern void repaint();
void serviceRepaints(){}

int main_a_int_fld, main_c_int_fld;
boolean main_b_boolean_fld;
int *main_a_int_array1d_fld=0;//????

typedef unsigned short ushort;

void Systemgc(){}
int getWidth=132, getHeight=176, screen_length=132*176;

extern ushort *screens;

//#define  getWidth 256
//#define getHeight 240
//#define screen_length getWidth*getHeight//???

//long millis=0;
long SystemcurrentTimeMillis(){ return 0;}
void Systemarraycopy(void *from, int foff, void *to, int toff, int size){
  memmove((char*)to+toff,(char*)from+foff, size);}

//#pragma warning (disable:4554)
//#pragma warning (disable:4101)


#define sbufsize 8192
unsigned char sbuf[sbufsize], *ebuf=sbuf;

int stream, streammode;

void flush(){
  fwrite(sbuf,ebuf-sbuf, 1, stream);
  ebuf=sbuf;
}

#define outputstreamwrite bytearrayoutputstreamwrite
#define inputstreamread bytearrayinputstreamread

void bytearrayoutputstreamwrite(int byt){
  if(ebuf>=sbuf+sbufsize) flush();
  *ebuf++=byt;
}

int bytearrayinputstreamread(){
  if(ebuf>=sbuf+sbufsize){
    fread(sbuf,sbufsize, 1, stream);
    ebuf=sbuf;
  }
  return *ebuf++;
}

FILE* openstream(int savemode){
  if(streammode==savemode){ //save
    if((stream=fopen(romname, "wb"))==NULL) return NULL;
    ebuf=sbuf;
  }else{
    ebuf=sbuf+sbufsize;
    if((stream=fopen(romname, "rb"))==NULL) return NULL;
  }return stream;
}

void closestream(){
  if(streammode) flush();
  fclose(stream);
}




///////////////////////////
static void k() ;
static void f() ;
static void l() ;
void run_step() ;
static void xxf(int i1, int j1) ;
    static void xxd(int i1, int j1) ;
  void keyPressed(int i1) ;
	void keyReleased(int i1) ;
static void j() ;
static void xd(int i1) ;
void h() ;
static boolean bbool() ;
 static void m() ;
static int xxb(int i1, int j1) ;
boolean loadrom(char *s) ;
static void d() ;
void c0() ;
void avoid1int(int i1) ;

   static void g() ;
   int loadROM(char *s) ;
    static void avoid() ;
    static void b() ;
    static boolean abool() ;
     int aint1int(int i1) ;
   void i() ;
    static void e(int i1, int j1) ;
    static void xxxxb(int i1, int j1, int k1, int l1) ;
    static void avoid4int(int i1, int j1, int k1, int l1) ;
    static void c(int i1, int j1) ;
    static void avoid8int(int i1, int j1, int k1, int l1, int i2, int j2, int k2, int l2) ;
    static void bvoid2int(int i1, int j1) ;
    static int aint2int(int i1, int j1) ;
    //static void b(InputStream inputstream) throws IOException
    static int bint1int(int i1) ;
    static void avoid2int(int i1, int j1) ;


///////

void savescreen(char *fname);


#define b_int_array1d_static_fld_length 8
#define e_int_array1d_static_fld_length 32
#define i_int_array1d_static_fld_length 12
#define l_int_array1d_static_fld_length 8
#define d_byte_array1d_static_fld_length 256
#define d_int_array1d_static_fld_length 8 //???4


 static void c_out();
 static void b_out();
 static void b_in();
 static void c_in();




