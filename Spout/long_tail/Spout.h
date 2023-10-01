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

//#define pceTimerGetCount SDL_GetTicks
#define srand(x) randomize()

#define PP_MODE_SINGLE 0
#define PP_MODE_REPEAT 1
#define pcePadSetTrigMode

extern int pceFontPrintf1(const char *format, int);
extern int pceFontPrintf2(const char *format, int, int);
extern int pceFontPrintf3(const char *format, int, int, int);
extern int pceFontPrintf(const char *formatted_string);
extern void pceFontSetTxColor(int color);
extern void pceFontSetBkColor(int color);
extern void pceFontSetPos(int x, int y);
extern void pceFontSetType(int type);

extern void pceLCDDispStop(void);
extern void pceLCDDispStart(void);
extern unsigned char *pceLCDSetBuffer(unsigned char *pbuff);
extern void pceLCDTrans(void);

extern void pceAppSetProcPeriod(int period);
extern void pceAppReqExit(int c);

extern int pceFileCreate(const char *fname, int mode);
extern int pceFileOpen(FILEACC *pfa, const char *fname, int mode);
extern int pceFileReadSct(FILEACC *pfa, void *ptr, int sct, int len);
extern int pceFileWriteSct(FILEACC *pfa, const void *ptr, int sct, int len);
extern int pceFileClose(FILEACC *pfa);

extern int pcePadGet(void);

extern void pceAppInit(void);
extern void pceAppExit(void);
extern void pceAppProc(int cnt);

extern unsigned char *vbuff;
extern unsigned char *vbuff2;

#endif // !SPOUT_H
