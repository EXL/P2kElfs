#ifndef SPOUT_H
#define SPOUT_H

#include <utilities.h>

#define SPOUT_WIDTH (128)
#define SPOUT_HEIGHT (88)

#define PAD_RI 0x01
#define PAD_LF 0x02
#define PAD_DN 0x04
#define PAD_UP 0x08
#define PAD_B  0x10
#define PAD_A  0x20
#define PAD_D  0x40
#define PAD_C  0x80

#define TRG_RI 0x0100
#define TRG_LF 0x0200
#define TRG_DN 0x0400
#define TRG_UP 0x0800
#define TRG_B  0x1000
#define TRG_A  0x2000
#define TRG_D  0x4000
#define TRG_C  0x8000

#define pcesprintf sprintf

#define CPU_SPEED_NORMAL 0
#define pceCPUSetSpeed

#define FILEACC int
#define FOMD_RD 0
#define FOMD_WR 1

#define pceTimerGetCount SDL_GetTicks

#define srand(x) randomize()

#define PP_MODE_SINGLE 0
#define PP_MODE_REPEAT 1
#define pcePadSetTrigMode

int pceFontPrintf(const char *fmt, ... );
void pceFontSetTxColor(int color);
void pceFontSetBkColor(int color);
void pceFontSetPos(int x, int y);
void pceFontSetType(int type);

extern void pceLCDDispStop(void);
extern void pceLCDDispStart(void);
unsigned char *pceLCDSetBuffer(unsigned char *pbuff);
extern void pceLCDTrans(void);

void pceAppSetProcPeriod(int period);
void pceAppReqExit(int c);

int pceFileCreate(const char *fname, int mode);
int pceFileOpen(FILEACC *pfa, const char *fname, int mode);
int pceFileReadSct(FILEACC *pfa, void *ptr, int sct, int len);
int pceFileWriteSct(FILEACC *pfa, const void *ptr, int sct, int len);
int pceFileClose(FILEACC *pfa);

extern int pcePadGet(void);

extern void pceAppInit(void);
extern void pceAppExit(void);
extern void pceAppProc(int cnt);

extern unsigned char vbuff[];
extern unsigned char vbuff2[];

#endif // !SPOUT_H
