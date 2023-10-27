#include <apps.h>
#include <canvas.h>
#include <uis.h>

#include <ati.h>
#include <dal.h>
#include <filesystem.h>
#include <loader.h>
#include <mem.h>
#include <sms.h>

#if defined(FTR_L6)
#define RECTSURF_W 128
#define RECTSURF_H 160
#define x_t (UINT8)(60)
#define y_t (UINT8)(102)
#else
#define RECTSURF_W 176
#define RECTSURF_H 220
#define x_t (UINT8)(100)
#define y_t (UINT8)(157)
#endif

typedef struct
{
	APPLICATION_T apt;
} APP_ATI_T;

typedef enum
{
	HW_STATE_ANY,
	HW_STATE_MAX
} HW_STATES_T;

UINT32 Register(char *file_uri, char *param, UINT32 reserve);
UINT32 startApp(EVENT_STACK_T *ev_st, REG_ID_T reg_id, UINT32 param2);
UINT32 destroyApp(EVENT_STACK_T *ev_st, APPLICATION_T *app);
UINT32 MainStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T type);
UINT32 HandleKeypress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
UINT32 Timer(EVENT_STACK_T *ev_st, APPLICATION_T *app);
UINT32 addcall(EVENT_STACK_T *ev_st, APPLICATION_T *app);
UINT32 delcall(EVENT_STACK_T *ev_st, APPLICATION_T *app);
UINT32 paint(void);

#define a(b, c) (UINT8)((b == c) ? (1) : (0))
#define b(t1, t2, t3, t4) (UINT8)((t1 << 3) + (t2 << 2) + (t3 << 1) + t4)
#define color_t (UINT32)(ATI_565RGB(0, 0, 255))

UIS_DIALOG_T dialog = 0xFFFFFF;
SU_PORT_T su_port = 0xFFFFFF;

typedef struct
{
	UINT8 r, g, b;
} TColor;
typedef struct
{
	INT8 x, y;
} TPoint;
UINT8 ccall = 0, _count[1] = {0}, s_d = 0, cpp = 0, pred_d = 0;
INT8 d = 3;
INT8 x = 4;
TColor *color;
char *arr, *bin;
FILE file;
UINT32 r;
BOOL new = 1, exit = 0;
TPoint s[1];
AHIBITMAP_T bm, bmp;
AHIDEVCONTEXT_T dCtx;
AHISURFACE_T sDraw;
AHISURFACE_T sDisp;
AHIPOINT_T pSurf = {0, 0};
AHIRECT_T rectSurf = {0, 0, RECTSURF_W, RECTSURF_H};

void TextColorRGB(UINT8 r, UINT8 g, UINT8 b)
{
	AhiDrawFgColorSet(dCtx, ATI_565RGB(r, g, b));
}

UINT8 two2bin(UINT8 b1, UINT8 b2, UINT8 b3, UINT8 b4, UINT8 c, UINT8 n)
{
	UINT8 t1, t2, t3, t4, t5, t6, t7, t8;
	t1 = (b1 >> 4);
	t2 = b1 - (t1 << 4);
	t3 = (b2 >> 4);
	t4 = b2 - (t3 << 4);
	t5 = (b3 >> 4);
	t6 = b3 - (t5 << 4);
	t7 = (b4 >> 4);
	t8 = b4 - (t7 << 4);
	return (n == 0) ? ((b(a(t1, c), a(t2, c), a(t3, c), a(t4, c)) << 4) + b(a(t5, c), a(t6, c), a(t7, c), a(t8, c)))
					: ((b(a(t8, c), a(t7, c), a(t6, c), a(t5, c)) << 4) + b(a(t4, c), a(t3, c), a(t2, c), a(t1, c)));
}

void FreeBin()
{
	if (arr)
	{
		mfree(arr);
	}
	if (bin)
	{
		mfree(bin);
	}
	if (color)
	{
		mfree(color);
	}
}

void OpenBin(UINT8 l)
{
	file = DL_FsOpenFile(L"file://c/Elf/sheep.ani", FILE_READ_MODE, 0);
	if (_count[0] < 1)
	{
		FreeBin();
		DL_FsReadFile(_count, 1, 1, file, &r);
		color = malloc(3 * _count[0]);
		DL_FsReadFile(color, 1, 3 * _count[0], file, &r);
		DL_FsReadFile(s, 1, 2, file, &r);
		arr = malloc(s[0].y * s[0].x);
		bin = malloc(s[0].y * s[0].x / 4);
	}
	// 3 * (15 + 1) + 1 * 20 * 40
	DL_FsFSeekFile(file, 3 * (_count[0] + 1) + l * s[0].x * s[0].y, 0);
	DL_FsReadFile(arr, 1, s[0].y * s[0].x, file, &r);
	DL_FsCloseFile(file);
	bmp.stride = s[0].x >> 2;
	bmp.width = (UINT32)(s[0].x * 2);
	bmp.height = (UINT32)(s[0].y);
}

void WriteBin(UINT8 x, UINT8 y, UINT8 u)
{
	UINT16 i, i2;
	UINT8 c, _x, _y;
	AHIPOINT_T p;
	AHIRECT_T r;
	p.x = 0;
	p.y = 0;
	r.x1 = x;
	r.y1 = y;
	r.x2 = x + bmp.width;
	r.y2 = y + bmp.height;
	pred_d = u;
	AhiDrawSurfDstSet(dCtx, sDraw, 0);
	for (c = 0; c < _count[0]; c++)
	{
		for (_y = 0; _y < s[0].y; _y++)
		{
			for (_x = 0; _x < (s[0].x) >> 2; _x++)
			{
				if (u == 0)
				{
					i = _y * (s[0].x >> 2) + _x;
					bin[i] = two2bin(arr[i << 2], arr[(i << 2) + 1], arr[(i << 2) + 2], arr[(i << 2) + 3], c, 0);
				}
				else if (u == 1)
				{
					i = _y * (s[0].x >> 2) + _x;
					i2 = (_y * s[0].x >> 2) + ((s[0].x >> 2) - _x - 1);
					bin[i] = two2bin(arr[i2 << 2], arr[(i2 << 2) + 1], arr[(i2 << 2) + 2], arr[(i2 << 2) + 3], c, 1);
				}
			}
		}
		bmp.image = (void *)(bin);
		TextColorRGB(color[c].r, color[c].g, color[c].b);
		if (ATI_565RGB(color[c].r, color[c].g, color[c].b) != color_t)
		{
			AhiDrawBitmapBlt(dCtx, &r, &p, &bmp, NULL, 1);
		}
	}
}

void AHG_Init()
{
	dCtx = DAL_GetDeviceContext(0);
	sDraw = DAL_GetDrawingSurface(0);
	AhiDispSurfGet(dCtx, &sDisp);
	AhiDrawSurfDstSet(dCtx, sDisp, 0);
	AhiDrawSurfSrcSet(dCtx, sDraw, 0);
	AhiDrawClipSrcSet(dCtx, NULL);
	AhiDrawClipDstSet(dCtx, NULL);
}

void AHG_Flush()
{
	AHIUPDATEPARAMS_T update_params;
	update_params.size = sizeof(AHIUPDATEPARAMS_T);
	update_params.sync = FALSE;
	update_params.rect.x1 = 0;
	update_params.rect.y1 = 0;
	update_params.rect.x2 = 0 + RECTSURF_W;
	update_params.rect.y2 = 0 + RECTSURF_H;

	AhiDrawSurfSrcSet(dCtx, sDraw, 0);
	AhiDrawSurfDstSet(dCtx, sDisp, 0);
	AhiDrawClipSrcSet(dCtx, NULL);
	AhiDrawClipDstSet(dCtx, NULL);
	AhiDrawRopSet(dCtx, AHIROP3(AHIROP_SRCCOPY));
	AhiDrawBitBlt(dCtx, &rectSurf, &pSurf);

//	AhiDispWaitVBlank(dCtx, 0);
	AhiDispUpdate(dCtx, &update_params);
}

BOOL KeypadLock()
{
	BOOL keypad_statate;
	DL_DbFeatureGetCurrentState(*KEYPAD_STATE, &keypad_statate);
	return keypad_statate;
}
BOOL WorkingTable()
{
	UINT8 res;
	UIS_GetActiveDialogType(&res);
	return (res == 0xE) ? (true) : (false);
}
BOOL sms()
{
	UINT16 t = 0;
	MsgUtilGetUnreadMsgsInAllFolders(&t);
	return (t > 0) ? (true) : (false);
}
BOOL call()
{
	return (ccall > 0) ? (true) : (false);
}
BOOL sleep()
{
	CLK_TIME_T time;
	DL_ClkGetTime(&time);
	return (time.hour > 22 || time.hour < 6) ? (true) : (false);
}
// BOOL JavaApp() {UINT8 res; UIS_GetActiveDialogType(&res);return (res == DialogType_GameAni)?(true):(false);}

void copy(UINT8 x, UINT8 y, UINT8 bk)
{
	AHIPOINT_T pt;
	AHIRECT_T rt;
	if (bk == 1)
	{
		pt.x = x;
		pt.y = y;
		rt.x1 = 0;
		rt.y1 = 0;
		rt.x2 = 40;
		rt.y2 = 40;
		AhiSurfCopy(dCtx, sDraw, &bm, &rt, &pt, NULL, 1);
	}
	else
	{
		pt.x = 0;
		pt.y = 0;
		rt.x1 = x;
		rt.y1 = y;
		rt.x2 = x + 40;
		rt.y2 = y + 40;
		AhiSurfCopy(dCtx, sDraw, &bm, &rt, &pt, NULL, 0);
	}
}

void _time()
{
	UINT8 count[12] = {1, 3, 3, 3, 3, 3, 3, 5, 3, 3, 5, 1}, start[12] = {0, 1, 1, 4, 4, 6, 9, 12, 9, 17, 20, 0},
		u[12] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
	INT8 px[12] = {0, -2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	if (!bm.image)
	{
		bm.image = (void *)malloc(40 * 40 * 2);
		copy((UINT8)(x), y_t, 1);
	}
	else
	{
		copy((UINT8)(x), y_t, 0);
	}
	if (sms())
	{
		d = 10;
	}
	else if (call())
	{
		d = 9;
	}
	else if (sleep())
	{
		d = 0;
	}
	if (new)
	{
		if (s_d == count[d] - 1)
		{
			new = false;
			s_d = 0;
		}
		else
		{
			s_d++;
			x = x + count[d] * px[d];
		}
	}
	else
	{
		if (count[d] * px[d] + x < 5)
		{
			d = 2;
		}
		else if (count[d] * px[d] + x > x_t)
		{
			d = 1;
		}
		else
		{
			d = random(20);
			if (d > 8)
			{
				while (d > 2)
				{
					d -= 5;
				}
			}
			if (d < 0)
			{
				d = 0;
			}
		}
		new = true;
	}
	if (sms())
	{
		d = 10;
	}
	else if (call())
	{
		d = 9;
	}
	else if (sleep())
	{
		d = 0;
	}
	copy((UINT8)(x), y_t, 1);
	OpenBin(start[d] + s_d);
	WriteBin((UINT8)(x), y_t, (d != 0) ? (u[d]) : (pred_d));
}

const EVENT_HANDLER_ENTRY_T any_state_handlers[] = {
	{0x8201B, addcall},        {0x0398, delcall},          {EV_KEY_PRESS, HandleKeypress},
	{EV_TIMER_EXPIRED, Timer}, {STATE_HANDLERS_END, NULL},
	};

const STATE_HANDLERS_ENTRY_T state_handling_table[] = {{HW_STATE_ANY, MainStateEnter, NULL, any_state_handlers}};

UINT32 Register(char *file_uri, char *param, UINT32 reserve)
{
	UINT32 status, e_b = reserve;
	// any_state_handlers[0].code = EV_CALLS_TERMINATED;//EV_CALLS_MISSED;
	bm.format = AHIFMT_16BPP_565;
	bm.width = 40;
	bm.height = 40;
	bm.stride = 40 * 2;
	bmp.format = AHIFMT_1BPP;
	status = APP_Register(&e_b, 1, state_handling_table, HW_STATE_MAX, (void *)startApp);
	LdrStartApp(e_b);
	return 1;
}

UINT32 startApp(EVENT_STACK_T *ev_st, REG_ID_T reg_id, UINT32 param2)
{
	char app_name[APP_NAME_LEN] = "sheep";
	UINT32 status;
	APP_ATI_T *app = NULL;
	randomize();
	AHG_Init();
	app = (APP_ATI_T *)APP_InitAppData((void *)APP_HandleEvent, sizeof(APP_ATI_T), reg_id, 0, 1, 1, 2, 0, 0);
	status = APP_Start(ev_st, &app->apt, HW_STATE_ANY, state_handling_table, destroyApp, app_name, 0);
	return RESULT_OK;
}

UINT32 destroyApp(EVENT_STACK_T *ev_st, APPLICATION_T *app)
{
	APPLICATION_T *papp = (APPLICATION_T *)app;
	if (exit)
	{
		APP_UtilStopTimer(app);
		APP_UtilUISDialogDelete(&papp->dialog);
		if (bm.image)
		{
			mfree(bm.image);
		}
		APP_Exit(ev_st, app, 0);
		LdrUnloadELF(&Lib);
	}
	return RESULT_OK;
}

UINT32 MainStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T type)
{
	APPLICATION_T *papp = (APPLICATION_T *)app;
	SU_PORT_T port = papp->port;
	if (type != ENTER_STATE_ENTER)
	{
		return RESULT_OK;
	}
	// dialog = UIS_CreateColorCanvasWithWallpaper(&port, &bufd, 0, 0);
	// dialog = UIS_CreateColorCanvas ( &port, &bufd, TRUE ); // б®§¤ с¬ Є ­ў б
	su_port = port;
	papp->port = su_port;
	papp->dialog = dialog;

	APP_UtilStartCyclicalTimer(200, 0, app);
	return RESULT_OK;
}

UINT32 HandleKeypress(EVENT_STACK_T *ev_st, APPLICATION_T *app)
{
	// EVENT_T *event = AFW_GetEv(ev_st);
	// UINT32 key = AFW_GetEv(ev_st)->data.key_pressed;
	if (AFW_GetEv(ev_st)->data.key_pressed == KEY_STAR)
	{
		cpp++;
	}
	else
	{
		cpp = 0;
	}
	if (cpp == 4)
	{
		exit = 1;
		return destroyApp(ev_st, app);
	}
	return RESULT_OK;
}

UINT32 paint(void)
{
	_time();
	AHG_Flush();
	return RESULT_OK;
}
UINT32 Timer(EVENT_STACK_T *ev_st, APPLICATION_T *app)
{
	if (WorkingTable() && !KeypadLock())
	{
		paint();
	}
	return RESULT_OK;
}
UINT32 addcall(EVENT_STACK_T *ev_st, APPLICATION_T *app)
{
	ccall = 1;
	return RESULT_OK;
}
UINT32 delcall(EVENT_STACK_T *ev_st, APPLICATION_T *app)
{
	ccall = 0;
	return RESULT_OK;
}

// UINT32 Show(EVENT_STACK_T *ev_st, void *app, BOOL show){
// return APP_ChangeRoutingStack(app,ev_st,
//(void*)(show?APP_HandleEvent:APP_HandleEventPrepost),
//(show?1:0),(show?0:1),1,(show?1:2));}

// UINT32 Tim (EVENT_STACK_T *ev_st, void *app) {
// inFocus = true;
// paint();
// return RESULT_OK; }

// UINT32 NoTim (EVENT_STACK_T *ev_st, void *app) {
// inFocus = false;
// APP_UtilChangeState(HW_STATE_ANY,ev_st,app);
// Show(ev_st,app,true);
// return RESULT_OK; }


#if 0
/*
 * About:
 *
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * Application type:
 *   Deamon + GUI + ATI + Widgets.
 */

#include <loader.h>
#include <apps.h>
#include <mem.h>
#include <uis.h>
#include <res_def.h>
#include <utilities.h>
#include <filesystem.h>
#include <dl.h>
#include <ati.h>

#include "icons/icon_ambilight_48x48.h"

#define MAX_NUMBER_LENGTH           (6)
#define MIN_NUMBER_VALUE            (10)
#define MAX_NUMBER_VALUE            (60000)
#define MAX_HEX_LENGTH              (10)
#define MAX_HEX_VALUE               (0xFFF)
#define KEY_LONG_PRESS_START_MS     (1500)
#define KEY_LONG_PRESS_STOP_MS      (2500)
#define NETWORK_MS_GAP_CONSTANT     (80)
#define AMBILIGHT_RECT_CONSTANT     (16)
#define LIGHT_SENSOR_MAX_VALUE      (0xE2)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_EDIT,
	APP_STATE_SELECT,
	APP_STATE_POPUP,
	APP_STATE_WARNING,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_DISPLAY_HIDE,
	APP_DISPLAY_SHOW
} APP_DISPLAY_T;

typedef enum {
	APP_TIMER_EXIT = 0xE490,
	APP_TIMER_DO_LIGHTS
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_STRING_NAME,
	APP_RESOURCE_ICON_AMBILIGHT,
	APP_RESOURCE_STRING_MODE,
	APP_RESOURCE_STRING_DELAY,
	APP_RESOURCE_STRING_COLOR,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_MODE = APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_DELAY,
	APP_MENU_ITEM_COLOR,
	APP_MENU_ITEM_START_LIGHTS,
	APP_MENU_ITEM_STOP_LIGHTS,
	APP_MENU_ITEM_RESET,
	APP_MENU_ITEM_HELP,
	APP_MENU_ITEM_ABOUT,
	APP_MENU_ITEM_EXIT,
	APP_MENU_ITEM_MAX
} APP_MENU_ITEM_T;

typedef enum {
	APP_SELECT_ITEM_FIRST,
	APP_SELECT_ITEM_AMBILIGHT = APP_SELECT_ITEM_FIRST,
	APP_SELECT_ITEM_COLOR,
	APP_SELECT_ITEM_COLOR_BLINK,
	APP_SELECT_ITEM_FLASH_25,
	APP_SELECT_ITEM_FLASH_50,
	APP_SELECT_ITEM_FLASH_100,
	APP_SELECT_ITEM_NETWORK,
	APP_SELECT_ITEM_BATTERY,
	APP_SELECT_ITEM_RAINBOW,
	APP_SELECT_ITEM_RANDOM,
	APP_SELECT_ITEM_STROBOSCOPE,
	APP_SELECT_ITEM_STROBO_COLOR,
	APP_SELECT_ITEM_BACKLIGHT,
	APP_SELECT_ITEM_MAX
} APP_SELECT_ITEM_T;

typedef enum {
	APP_POPUP_CHANGED,
	APP_POPUP_WARNING,
	APP_POPUP_STARTED,
	APP_POPUP_STOPPED,
	APP_POPUP_RESETED
} APP_POPUP_T;

typedef enum {
	APP_WARNING_FLASH,
	APP_WARNING_RESET
} APP_WARNING_T;

typedef enum {
	APP_VIEW_HELP,
	APP_VIEW_ABOUT
} APP_VIEW_T;

typedef enum {
	APP_EDIT_DELAY,
	APP_EDIT_COLOR
} APP_EDIT_T;

typedef struct {
	APP_SELECT_ITEM_T mode;
	UINT32 delay;
	UINT16 color;
} APP_OPTIONS_T;

typedef struct {
	UINT8 r;
	UINT8 g;
	UINT8 b;
} APP_COLOR_T;

typedef enum {
	RAINBOW_RED,
	RAINBOW_ORANGE,
	RAINBOW_YELLOW,
	RAINBOW_GREEN,
	RAINBOW_LIGHT_BLUE,
	RAINBOW_BLUE,
	RAINBOW_VIOLET,
	RAINBOW_WHITE
} RAINBOW_COLOR_T;

typedef enum {
	STATE_NETWORK_SIGNAL,
	STATE_BATTERY
} PHONE_STATE_T;

typedef struct {
	APPLICATION_T app;

	RESOURCE_ID resources[APP_RESOURCE_MAX];

	APP_DISPLAY_T state;
	APP_POPUP_T popup;
	APP_VIEW_T view;
	APP_EDIT_T edit;
	APP_MENU_ITEM_T menu_current_item_index;
	APP_WARNING_T warning;

	APP_OPTIONS_T options;

	UINT64 ms_key_press_start;
	UINT32 timer_handle;
	BOOL is_lights_started;

	APP_COLOR_T color_current;
	APP_COLOR_T color_pattern;
	RAINBOW_COLOR_T rainbow;
	UINT32 blink;
} APP_INSTANCE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ApplicationDisplay(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_DISPLAY_T display);

static UINT32 InitResourses(RESOURCE_ID *resources);
static UINT32 FreeResourses(RESOURCE_ID *resources);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static void HandleEventMain(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id);
static UINT32 HandleEventHide(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventShow(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventMenuRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventEditData(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventEditDone(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventSelectDone(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventSelectRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_STATE_T app_state);
static UINT32 HandleEventCancel(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 SendMenuItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count);
static UINT32 SendSelectItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count);
static const WCHAR *GetTriggerOptionString(APP_SELECT_ITEM_T item);
static UINT32 ResetSettingsToDefaultValues(APPLICATION_T *app);
static UINT32 ReadFileConfig(APPLICATION_T *app, const WCHAR *file_config_path);
static UINT32 SaveFileConfig(APPLICATION_T *app, const WCHAR *file_config_path);

static BOOL IsKeyPadLocked(void);
static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period);
static UINT32 ConvertDecToHexColor(UINT32 number, WCHAR *heximal);
static UINT32 ConvertHexColorToDec(const WCHAR *heximal, UINT32 *number);

static UINT32 StartLights(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ProcessLights(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 StopLights(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 Ambilight(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 PhoneState(EVENT_STACK_T *ev_st, APPLICATION_T *app, PHONE_STATE_T state);
static UINT32 Rainbow(EVENT_STACK_T *ev_st, APPLICATION_T *app, BOOL random_colors);
static UINT32 Stroboscope(EVENT_STACK_T *ev_st, APPLICATION_T *app, BOOL flashlight, BOOL color, BOOL random);
static UINT16 GetAverageColorRGB444(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT16 start, UINT16 size);
static UINT32 Backlight(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "Ambilight";

static const WCHAR g_str_app_name[] = L"Ambilight";
static const WCHAR g_str_mode[] = L"Light Mode:";
static const WCHAR g_str_e_mode[] = L"Light Mode";
static const WCHAR g_str_mode_ambilight[] = L"Ambilight";
static const WCHAR g_str_mode_color[] = L"Color";
static const WCHAR g_str_mode_color_blink[] = L"Color Blink";
static const WCHAR g_str_mode_flash25[] = L"Flash 25%";
static const WCHAR g_str_mode_flash50[] = L"Flash 50%";
static const WCHAR g_str_mode_flash100[] = L"Flash 100%";
static const WCHAR g_str_mode_network[] = L"Network";
static const WCHAR g_str_mode_battery[] = L"Battery";
static const WCHAR g_str_mode_rainbow[] = L"Rainbow";
static const WCHAR g_str_mode_random[] = L"Random";
static const WCHAR g_str_mode_stroboscope[] = L"Stroboscope";
static const WCHAR g_str_mode_strobo_color[] = L"Strobo Color";
static const WCHAR g_str_mode_backlight[] = L"Backlight";
static const WCHAR g_str_color[] = L"RGB Color:";
static const WCHAR g_str_e_color[] = L"HEX Color";
static const WCHAR g_str_delay[] = L"Delay (in ms):";
static const WCHAR g_str_e_delay[] = L"Delay (in ms)";
static const WCHAR g_str_start_ligths[] = L"Start Lights!";
static const WCHAR g_str_stop_ligths[] = L"Stop Lights!";
static const WCHAR g_str_reset[] = L"Reset to default";
static const WCHAR g_str_help[] = L"Help...";
static const WCHAR g_str_e_help[] = L"Help";
static const WCHAR g_str_about[] = L"About...";
static const WCHAR g_str_e_about[] = L"About";
static const WCHAR g_str_exit[] = L"Exit";
static const WCHAR g_str_changed[] = L"Changed:";
static const WCHAR g_str_started[] = L"Lights service started!";
static const WCHAR g_str_stopped[] = L"Lights service stopped!";
static const WCHAR g_str_warn_activated[] = L"The \"Flash 100%\" mode is activated, don't use this mode for too long!";
static const WCHAR g_str_warn_flash[] = L"This can be dangerous to the phone's flash diode, are you sure?";
static const WCHAR g_str_reseted[] = L"All settings have been reset to default values!";
static const WCHAR g_str_warn_reset[] = L"Do you want to reset settings to default?";
static const WCHAR g_str_help_content_p1[] =
	L"Bias lighting (ambilight) daemon program for Motorola P2K phones.\n\n"
	L"Press and hold the right soft key for 1.5-2.5 seconds to bring up the main application menu.\n\n"
	L"The following modes are available for use:\n\n"
	L"Ambilight - The side LEDs use the most common color on the screen as source of light.\n\n"
	L"Color - The side LEDs use the specified HEX string with (0xRGB) format as source of light.\n\n"
	L"Color Blink - The side LEDs blink with the specified HEX string with (0xRGB) format as source of light.\n\n"
	L"Flash 25% - The flash LED lights up at 25% brightness of maximum.\n\n"
	L"Flash 50% - The flash LED lights up at 50% brightness of maximum.\n\n"
	L"Flash 100% - The flash LED lights up at maximum brightness. Danger! This mode may damage the flash LED!\n\n"
	L"Network - The side LEDs blink on network signal strength: 0..30% - red, 30..60% - yellow, 60..100% - green\n\n"
	L"Battery - The side LEDs blink depending on battery: 0..30% - red, 30..60% - yellow, 60..100% - green\n\n"
	L"Rainbow - Smooth rainbow effect on the side LEDs.\n\n"
	L"Random - Smooth color transitions to random colors.\n\n"
	L"Stroboscope - Strobe mode with side LEDs (white color) and flash LED.\n\n"
	L"Strobo Color - Strobe mode with side LEDs (random color) and flash LED.\n\n"
	L"Backlight - Smooth change in backlight brightness based on the Light Sensor.\n\n"
	L"Some modes require setting the delay and the color.\n\n"
	L"Managing of lighting services is available in the main menu of the ELF application.\n";
static const WCHAR g_str_about_content_p1[] = L"Version: 1.0";
static const WCHAR g_str_about_content_p2[] = L"\x00A9 EXL, 13-Sep-2023.";
static const WCHAR g_str_about_content_p3[] = L"https://github.com/EXL/P2kElfs/tree/master/Ambilight";
static const WCHAR g_str_about_content_p4[] = L"       "; /* HACK: gap */

static const UINT8 g_key_app_menu = KEY_SOFT_LEFT;
static const UINT8 g_key_app_exit = KEY_0;

static WCHAR g_config_file_path[FS_MAX_URI_NAME_LENGTH]; /* TODO: Can it be non-global? */

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ EV_KEY_PRESS, HandleEventKeyPress },
	{ EV_KEY_RELEASE, HandleEventKeyRelease },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	/*
	 * EXL, 01-Dec-2022:
	 *  Please comment out the `{ EV_GRANT_TOKEN, APP_HandleUITokenGranted }` construction
	 *  then change `app_instance->state` variable to `APP_DISPLAY_HIDE` value
	 *  then change `routing_stack` argument in `APP_InitAppData()` function to `APP_DISPLAY_HIDE` value
	 *  for pure daemon behavior with no GUI at start.
	 */
	/* { EV_GRANT_TOKEN, APP_HandleUITokenGranted }, */
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_REQUEST_LIST_ITEMS, HandleEventMenuRequestListItems },
	{ EV_DONE, HandleEventHide },
	{ EV_DIALOG_DONE, HandleEventHide },
	{ EV_SELECT, HandleEventSelect },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_edit_hdls[] = {
	{ EV_DATA, HandleEventEditData },
	{ EV_DONE, HandleEventEditDone },
	{ EV_DIALOG_DONE, HandleEventCancel },
	{ EV_CANCEL, HandleEventCancel },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_select_hdls[] = {
	{ EV_DONE, HandleEventSelectDone },
	{ EV_DIALOG_DONE, HandleEventCancel },
	{ EV_CANCEL, HandleEventCancel },
	{ EV_REQUEST_LIST_ITEMS, HandleEventSelectRequestListItems },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_popup_hdls[] = {
	{ EV_DONE, HandleEventCancel },
	{ EV_DIALOG_DONE, HandleEventCancel },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_warn_hdls[] = {
	{ EV_DONE, HandleEventCancel },
	{ EV_DIALOG_DONE, HandleEventCancel },
	{ EV_NO, HandleEventCancel },
	{ EV_YES, HandleEventYes },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls },
	{ APP_STATE_EDIT, HandleStateEnter, HandleStateExit, g_state_edit_hdls },
	{ APP_STATE_SELECT, HandleStateEnter, HandleStateExit, g_state_select_hdls },
	{ APP_STATE_POPUP, HandleStateEnter, HandleStateExit, g_state_popup_hdls },
	{ APP_STATE_WARNING, HandleStateEnter, HandleStateExit, g_state_warn_hdls },
	{ APP_STATE_VIEW, HandleStateEnter, HandleStateExit, g_state_popup_hdls } /* Same as popups. */
};

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	u_atou(elf_path_uri, g_config_file_path);
	g_config_file_path[u_strlen(g_config_file_path) - 3] = '\0';
	u_strcat(g_config_file_path, L"cfg");

	LdrStartApp(ev_code_base);

	return status;
}

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) HandleEventMain, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, APP_DISPLAY_HIDE, 0);

		InitResourses(app_instance->resources);
		app_instance->state = APP_DISPLAY_HIDE;
		app_instance->popup = APP_POPUP_CHANGED;
		app_instance->view = APP_VIEW_HELP;
		app_instance->edit = APP_EDIT_DELAY;
		app_instance->warning = APP_WARNING_FLASH;
		app_instance->menu_current_item_index = APP_MENU_ITEM_FIRST;
		app_instance->ms_key_press_start = 0LLU;
		app_instance->is_lights_started = FALSE;
		app_instance->color_current.r = 0xF;
		app_instance->color_current.g = 0x0;
		app_instance->color_current.b = 0x0;
		app_instance->color_pattern.r = 0xF;
		app_instance->color_pattern.g = 0x0;
		app_instance->color_pattern.b = 0x0;
		app_instance->rainbow = RAINBOW_RED;
		app_instance->blink = 0;

		ResetSettingsToDefaultValues((APPLICATION_T *) app_instance);

		if (DL_FsFFileExist(g_config_file_path)) {
			ReadFileConfig((APPLICATION_T *) app_instance, g_config_file_path);
		} else {
			SaveFileConfig((APPLICATION_T *) app_instance, g_config_file_path);
		}

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_INIT,
			g_state_table_hdls, HandleEventHide, g_app_name, 0);

		StartLights(ev_st, (APPLICATION_T *) app_instance);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	FreeResourses(app_instance->resources);

	status |= StopLights(ev_st, app);

	status |= APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_NAME], RES_TYPE_STRING,
		(void *) g_str_app_name, (u_strlen(g_str_app_name) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_MODE], RES_TYPE_STRING,
		(void *) g_str_e_mode, (u_strlen(g_str_e_mode) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_DELAY], RES_TYPE_STRING,
		(void *) g_str_e_delay, (u_strlen(g_str_e_delay) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_COLOR], RES_TYPE_STRING,
		(void *) g_str_e_color, (u_strlen(g_str_e_color) + 1) * sizeof(WCHAR));

	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_AMBILIGHT], RES_TYPE_GRAPHICS,
		(void *) ambilight_48x48_gif, sizeof(ambilight_48x48_gif));

	return status;
}

static UINT32 FreeResourses(RESOURCE_ID *resources) {
	UINT32 status;
	UINT32 i;

	status = RESULT_OK;

	for (i = 0; i < APP_RESOURCE_MAX; ++i) {
		if (resources[i]) {
			status |= DRM_ClearResource(resources[i]);
		}
	}

	return status;
}

static UINT32 ApplicationDisplay(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_DISPLAY_T display) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	void *hdl;
	UINT32 routing_stack;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->state != display) {
		app_instance->state = display;
		hdl = (void *) ((app_instance->state == APP_DISPLAY_SHOW) ? APP_HandleEvent : APP_HandleEventPrepost);
		routing_stack = (app_instance->state == APP_DISPLAY_SHOW);
		status = APP_ChangeRoutingStack(app, ev_st, hdl, routing_stack, 0, 1, 1);
	}

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	APP_INSTANCE_T *app_instance;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	UINT32 starting_list_item;
	RESOURCE_ID edit_title;
	WCHAR edit_buffer[MAX_HEX_LENGTH];

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;

	memclr(&content, sizeof(CONTENT_T));

	switch (app_state) {
		case APP_STATE_MAIN:
			starting_list_item = APP_MENU_ITEM_FIRST;
			dialog = UIS_CreateList(&port, 0, APP_MENU_ITEM_MAX, 0, &starting_list_item, 0, 2, NULL,
				app_instance->resources[APP_RESOURCE_STRING_NAME]);

			/* Insert cursor to proper position. */
			if (app_instance->menu_current_item_index != APP_MENU_ITEM_FIRST) {
				APP_UtilAddEvUISListChange(ev_st, app, 0, app_instance->menu_current_item_index + 1, APP_MENU_ITEM_MAX,
					FALSE, 2, NULL, NULL, NULL);
				UIS_HandleEvent(dialog, ev_st);
			}
			break;
		case APP_STATE_EDIT:
			switch (app_instance->edit) {
				case APP_EDIT_DELAY:
					edit_title = app_instance->resources[APP_RESOURCE_STRING_DELAY];
					u_ltou(app_instance->options.delay, edit_buffer);
					dialog = UIS_CreateCharacterEditor(&port, edit_buffer, 32 /* Numbers only. */, MAX_NUMBER_LENGTH,
						FALSE, NULL, edit_title);
					break;
				case APP_EDIT_COLOR:
					edit_title = app_instance->resources[APP_RESOURCE_STRING_COLOR];
					ConvertDecToHexColor(app_instance->options.color, edit_buffer);
					dialog = UIS_CreateCharacterEditor(&port, edit_buffer, 5 /* Text. */, MAX_NUMBER_LENGTH,
						FALSE, NULL, edit_title);
					break;
				default:
					break;
			}
			break;
		case APP_STATE_SELECT:
			starting_list_item = APP_MENU_ITEM_FIRST;
			dialog = UIS_CreateSelectionEditor(&port, 0, APP_SELECT_ITEM_MAX, app_instance->options.mode + 1,
				&starting_list_item, 0, NULL, app_instance->resources[APP_RESOURCE_STRING_MODE]);
			break;
		case APP_STATE_POPUP:
			switch (app_instance->popup) {
				default:
				case APP_POPUP_CHANGED:
					UIS_MakeContentFromString("MCq0NMCq1NMCq2", &content,
						g_str_changed, g_str_e_mode, GetTriggerOptionString(app_instance->options.mode));
					break;
				case APP_POPUP_WARNING:
					UIS_MakeContentFromString("MCq0", &content, g_str_warn_activated);
					break;
				case APP_POPUP_STARTED:
					UIS_MakeContentFromString("MCq0", &content, g_str_started);
					break;
				case APP_POPUP_STOPPED:
					UIS_MakeContentFromString("MCq0", &content, g_str_stopped);
					break;
				case APP_POPUP_RESETED:
					UIS_MakeContentFromString("MCq0", &content, g_str_reseted);
					break;
			}
			dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_OK);
			break;
		case APP_STATE_WARNING:
			switch (app_instance->warning) {
				default:
				case APP_WARNING_FLASH:
					UIS_MakeContentFromString("MCq0", &content, g_str_warn_flash);
					break;
				case APP_WARNING_RESET:
					UIS_MakeContentFromString("MCq0", &content, g_str_warn_reset);
					break;
			}
			dialog = UIS_CreateConfirmation(&port, &content);
			break;
		case APP_STATE_VIEW:
			switch (app_instance->view) {
				default:
				case APP_VIEW_HELP:
					UIS_MakeContentFromString("q0Nq1", &content, g_str_e_help,
						g_str_help_content_p1
					);
					break;
				case APP_VIEW_ABOUT:
					UIS_MakeContentFromString("q0NMCp1NMCq2NMCq3NMCq4NMCq5NMCq6", &content, g_str_app_name,
						app_instance->resources[APP_RESOURCE_ICON_AMBILIGHT],
						g_str_about_content_p1, g_str_about_content_p2, g_str_about_content_p3,
						g_str_about_content_p4, g_str_about_content_p4);
					break;
			}
			dialog = UIS_CreateViewer(&port, &content, NULL);
			break;
		default:
			dialog = DialogType_None;
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	switch (app_state) {
		case APP_STATE_MAIN:
			SendMenuItemsToList(ev_st, app, 1, APP_MENU_ITEM_MAX);
			break;
		case APP_STATE_SELECT:
			SendSelectItemsToList(ev_st, app, 1, APP_SELECT_ITEM_MAX);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state) {
	if (state == EXIT_STATE_EXIT) {
		DeleteDialog(app);
		return RESULT_OK;
	}
	return RESULT_FAIL;
}

static UINT32 DeleteDialog(APPLICATION_T *app) {
	if (app->dialog != DialogType_None) {
		UIS_Delete(app->dialog);
		app->dialog = DialogType_None;
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static void HandleEventMain(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id) {
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->state == APP_DISPLAY_SHOW) {
		APP_HandleEvent(ev_st, app, app_id, reg_id);
	} else {
		APP_HandleEventPrepost(ev_st, app, app_id, reg_id);
	}
}

static UINT32 HandleEventHide(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	ApplicationDisplay(ev_st, app, APP_DISPLAY_HIDE);
	return RESULT_OK;
}

static UINT32 HandleEventShow(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	if (app->state != APP_STATE_MAIN) {
		APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);
	}

	ApplicationDisplay(ev_st, app, APP_DISPLAY_SHOW);

	return RESULT_OK;
}

static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	app_instance->menu_current_item_index = event->data.index - 1;

	switch (app_instance->menu_current_item_index) {
		case APP_MENU_ITEM_MODE:
			status |= APP_UtilChangeState(APP_STATE_SELECT, ev_st, app);
			break;
		case APP_MENU_ITEM_DELAY:
			app_instance->edit = APP_EDIT_DELAY;
			status |= APP_UtilChangeState(APP_STATE_EDIT, ev_st, app);
			break;
		case APP_MENU_ITEM_COLOR:
			app_instance->edit = APP_EDIT_COLOR;
			status |= APP_UtilChangeState(APP_STATE_EDIT, ev_st, app);
			break;
		case APP_MENU_ITEM_START_LIGHTS:
			if (app_instance->options.mode == APP_SELECT_ITEM_FLASH_100) {
				app_instance->warning = APP_WARNING_FLASH;
				status |= APP_UtilChangeState(APP_STATE_WARNING, ev_st, app);
			} else {
				app_instance->popup = APP_POPUP_STARTED;
				StartLights(ev_st, app);
				status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
			}
			break;
		case APP_MENU_ITEM_STOP_LIGHTS:
			app_instance->popup = APP_POPUP_STOPPED;
			StopLights(ev_st, app);
			status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
			break;
		case APP_MENU_ITEM_RESET:
			app_instance->warning = APP_WARNING_RESET;
			status |= APP_UtilChangeState(APP_STATE_WARNING, ev_st, app);
			break;
		case APP_MENU_ITEM_HELP:
			app_instance->view = APP_VIEW_HELP;
			status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);
			break;
		case APP_MENU_ITEM_ABOUT:
			app_instance->view = APP_VIEW_ABOUT;
			status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);
			break;
		case APP_MENU_ITEM_EXIT:
			status |= APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
			break;
		default:
			break;
	}

	status |= APP_ConsumeEv(ev_st, app);

	return status;
}

static UINT32 HandleEventMenuRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	return HandleEventRequestListItems(ev_st, app, APP_STATE_MAIN);
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT8 key;

	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_app_menu || key == g_key_app_exit) {
		app_instance->ms_key_press_start = suPalTicksToMsec(suPalReadTime());
	}

	return RESULT_OK;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT8 key;
	UINT32 ms_key_release_stop;

	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_app_menu || key == g_key_app_exit) {
		/*
		 * Detect long key press between 500 ms (0.5 s) and 1500 ms (1.5 s) and ignore rest.
		 */
		ms_key_release_stop = (UINT32) (suPalTicksToMsec(suPalReadTime()) - app_instance->ms_key_press_start);
		if ((ms_key_release_stop >= KEY_LONG_PRESS_START_MS) && (ms_key_release_stop <= KEY_LONG_PRESS_STOP_MS)) {
			if (key == g_key_app_menu) {
				if (!IsKeyPadLocked()) {
					APP_ConsumeEv(ev_st, app);
					return HandleEventShow(ev_st, app);
				}
			} else if (key == g_key_app_exit) {
#ifdef EXIT_BY_KEY
				APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
#endif
			}
		}
	}

	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	if (timer_id == APP_TIMER_EXIT) {
		/* Play an exit sound using quiet speaker. */
		DL_AudPlayTone(0x00,  0xFF);

		/* Exit App! */
		return ApplicationStop(ev_st, app);
	} else if (timer_id == APP_TIMER_DO_LIGHTS) {
		ProcessLights(ev_st, app);
	}

	return RESULT_OK;
}

static UINT32 HandleEventEditData(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT32 data;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	if (event->attachment != NULL) {
		switch (app_instance->edit) {
			case APP_EDIT_DELAY:
				data = u_atol(event->attachment);
				if (data > MAX_NUMBER_VALUE) {
					data = MAX_NUMBER_VALUE;
				}
				if (data < MIN_NUMBER_VALUE) {
					data = MIN_NUMBER_VALUE;
				}
				app_instance->options.delay = data;
				break;
			case APP_EDIT_COLOR:
				ConvertHexColorToDec(event->attachment, &data);
				app_instance->options.color = data;
				break;
			default:
				break;
		}
	}

	if (app_instance->is_lights_started) {
		if (app_instance->options.mode == APP_SELECT_ITEM_FLASH_100) {
			StopLights(ev_st, app);
		} else {
			StartLights(ev_st, app);
		}
	}

	status |= SaveFileConfig(app, g_config_file_path);
	status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);

	return status;
}

static UINT32 HandleEventEditDone(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	ADD_EVENT_DATA_T *event_data;

	status = RESULT_OK;

	/*
	 * EXL, 05-Jan-2023:
	 *  I don't know how, but this trick with uninitialised pointer just works.
	 *  Structure `ADD_EVENT_DATA_T &event_data;` on stack leads to a strange problem
	 *  when editing the list with deleting one symbol, only one character appears after editing.
	 */
	status |= AFW_AddEvEvD(ev_st, EV_REQUEST_DATA, event_data);
	status |= UIS_HandleEvent(app->dialog, ev_st);

	return status;
}

static UINT32 HandleEventSelectDone(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	app_instance->options.mode = event->data.index - 1;
	app_instance->popup = APP_POPUP_CHANGED;

	if (app_instance->is_lights_started) {
		if (app_instance->options.mode == APP_SELECT_ITEM_FLASH_100) {
			StopLights(ev_st, app);
		} else {
			StartLights(ev_st, app);
		}
	}

	status |= SaveFileConfig(app, g_config_file_path);
	status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);

	return status;
}

static UINT32 HandleEventSelectRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	return HandleEventRequestListItems(ev_st, app, APP_STATE_SELECT);
}

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	switch (app_instance->warning) {
		default:
		case APP_WARNING_FLASH:
			app_instance->popup = APP_POPUP_WARNING;
			break;
		case APP_WARNING_RESET:
			app_instance->popup = APP_POPUP_RESETED;
			ResetSettingsToDefaultValues(app);
			break;
	}

	StartLights(ev_st, app);

	status |= SaveFileConfig(app, g_config_file_path);
	status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);

	return status;
}

static UINT32 HandleEventRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_STATE_T app_state) {
	UINT32 status;
	EVENT_T *event;
	UINT32 start;
	UINT32 count;

	status = RESULT_OK;

	if (!app->focused) {
		return status;
	}

	event = AFW_GetEv(ev_st);
	start = event->data.list_items_req.begin_idx;
	count = event->data.list_items_req.count;

	status |= APP_ConsumeEv(ev_st, app);

	switch (app_state) {
		default:
		case APP_STATE_MAIN:
			status |= SendMenuItemsToList(ev_st, app, start, count);
			break;
		case APP_STATE_SELECT:
			SendSelectItemsToList(ev_st, app, start, count);
			break;
	}

	return status;
}

static UINT32 HandleEventCancel(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);

	return status;
}

static UINT32 SendMenuItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count) {
	UINT32 status;
	INT32 result;
	APP_INSTANCE_T *app_instance;
	UINT32 i;
	LIST_ENTRY_T *list;
	WCHAR hex[MAX_HEX_LENGTH];

	status = RESULT_OK;
	result = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	if (count == 0) {
		return RESULT_FAIL;
	}
	list = (LIST_ENTRY_T *) suAllocMem(sizeof(LIST_ENTRY_T) * APP_MENU_ITEM_MAX, &result);
	if (result != RESULT_OK) {
		return RESULT_FAIL;
	}

	for (i = 0; i < APP_MENU_ITEM_MAX; ++i) {
		memclr(&list[i], sizeof(LIST_ENTRY_T));
		list[i].editable = FALSE;
		list[i].content.static_entry.formatting = TRUE;
	}

	ConvertDecToHexColor(app_instance->options.color, hex);

	status |= UIS_MakeContentFromString("Mq0Sq1",
		&list[APP_MENU_ITEM_MODE].content.static_entry.text,
		g_str_mode, GetTriggerOptionString(app_instance->options.mode));
	status |= UIS_MakeContentFromString("Mq0Si1",
		&list[APP_MENU_ITEM_DELAY].content.static_entry.text,
		g_str_delay, app_instance->options.delay);
	status |= UIS_MakeContentFromString("Mq0Sq1",
		&list[APP_MENU_ITEM_COLOR].content.static_entry.text,
		g_str_color, hex);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_START_LIGHTS].content.static_entry.text,
		g_str_start_ligths);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_STOP_LIGHTS].content.static_entry.text,
		g_str_stop_ligths);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_RESET].content.static_entry.text,
		g_str_reset);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_HELP].content.static_entry.text,
		g_str_help);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_ABOUT].content.static_entry.text,
		g_str_about);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_EXIT].content.static_entry.text,
		g_str_exit);

	status |= APP_UtilAddEvUISListData(ev_st, app, 0, start, APP_MENU_ITEM_MAX, FBF_LEAVE,
		sizeof(LIST_ENTRY_T) * APP_MENU_ITEM_MAX, list);
	if (status != RESULT_FAIL) {
		UIS_HandleEvent(app->dialog, ev_st);
	}

	suFreeMem(list);

	return status;
}

static UINT32 SendSelectItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count) {
	UINT32 status;
	INT32 result;
	UINT32 i;
	LIST_ENTRY_T *list;

	status = RESULT_OK;
	result = RESULT_OK;

	if (count == 0) {
		return RESULT_FAIL;
	}
	list = (LIST_ENTRY_T *) suAllocMem(sizeof(LIST_ENTRY_T) * APP_SELECT_ITEM_MAX, &result);
	if (result != RESULT_OK) {
		return RESULT_FAIL;
	}

	for (i = 0; i < APP_SELECT_ITEM_MAX; ++i) {
		memclr(&list[i], sizeof(LIST_ENTRY_T));
		list[i].editable = FALSE;
		list[i].content.static_entry.formatting = TRUE;
	}

	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_AMBILIGHT].content.static_entry.text,
		g_str_mode_ambilight);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_COLOR].content.static_entry.text,
		g_str_mode_color);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_COLOR_BLINK].content.static_entry.text,
		g_str_mode_color_blink);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_FLASH_25].content.static_entry.text,
		g_str_mode_flash25);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_FLASH_50].content.static_entry.text,
		g_str_mode_flash50);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_FLASH_100].content.static_entry.text,
		g_str_mode_flash100);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_NETWORK].content.static_entry.text,
		g_str_mode_network);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_BATTERY].content.static_entry.text,
		g_str_mode_battery);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_RAINBOW].content.static_entry.text,
		g_str_mode_rainbow);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_RANDOM].content.static_entry.text,
		g_str_mode_random);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_STROBOSCOPE].content.static_entry.text,
		g_str_mode_stroboscope);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_STROBO_COLOR].content.static_entry.text,
		g_str_mode_strobo_color);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_BACKLIGHT].content.static_entry.text,
		g_str_mode_backlight);

	status |= APP_UtilAddEvUISListData(ev_st, app, 0, start, APP_SELECT_ITEM_MAX, FBF_LEAVE,
		sizeof(LIST_ENTRY_T) * APP_SELECT_ITEM_MAX, list);
	if (status != RESULT_FAIL) {
		UIS_HandleEvent(app->dialog, ev_st);
	}

	suFreeMem(list);

	return status;
}

static const WCHAR *GetTriggerOptionString(APP_SELECT_ITEM_T item) {
	switch (item) {
		default:
		case APP_SELECT_ITEM_AMBILIGHT:
			return g_str_mode_ambilight;
		case APP_SELECT_ITEM_COLOR:
			return g_str_mode_color;
		case APP_SELECT_ITEM_COLOR_BLINK:
			return g_str_mode_color_blink;
		case APP_SELECT_ITEM_FLASH_25:
			return g_str_mode_flash25;
		case APP_SELECT_ITEM_FLASH_50:
			return g_str_mode_flash50;
		case APP_SELECT_ITEM_FLASH_100:
			return g_str_mode_flash100;
		case APP_SELECT_ITEM_NETWORK:
			return g_str_mode_network;
		case APP_SELECT_ITEM_BATTERY:
			return g_str_mode_battery;
		case APP_SELECT_ITEM_RAINBOW:
			return g_str_mode_rainbow;
		case APP_SELECT_ITEM_RANDOM:
			return g_str_mode_random;
		case APP_SELECT_ITEM_STROBOSCOPE:
			return g_str_mode_stroboscope;
		case APP_SELECT_ITEM_STROBO_COLOR:
			return g_str_mode_strobo_color;
		case APP_SELECT_ITEM_BACKLIGHT:
			return g_str_mode_backlight;
	}
}

static UINT32 ResetSettingsToDefaultValues(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	app_instance->options.mode = APP_SELECT_ITEM_RAINBOW;
	app_instance->options.delay = 250; /* 250 ms. */
	app_instance->options.color = 0x80F; /* Violet. */

	return status;
}

static UINT32 ReadFileConfig(APPLICATION_T *app, const WCHAR *file_config_path) {
	UINT32 readen;
	APP_INSTANCE_T *app_instance;
	FILE_HANDLE_T file_config;

	readen = 0;
	app_instance = (APP_INSTANCE_T *) app;

	file_config = DL_FsOpenFile(file_config_path, FILE_READ_MODE, 0);
	DL_FsReadFile(&app_instance->options, sizeof(APP_OPTIONS_T), 1, file_config, &readen);
	DL_FsCloseFile(file_config);

	return (readen == 0);
}

static UINT32 SaveFileConfig(APPLICATION_T *app, const WCHAR *file_config_path) {
	UINT32 written;
	APP_INSTANCE_T *app_instance;
	FILE_HANDLE_T file_config;

	written = 0;
	app_instance = (APP_INSTANCE_T *) app;

	file_config = DL_FsOpenFile(file_config_path, FILE_WRITE_MODE, 0);
	DL_FsWriteFile(&app_instance->options, sizeof(APP_OPTIONS_T), 1, file_config, &written);
	DL_FsCloseFile(file_config);

	return (written == 0);
}

static BOOL IsKeyPadLocked(void) {
	UINT8 keypad_state;
	DL_DbFeatureGetCurrentState(DB_FEATURE_KEYPAD_STATE, &keypad_state);
	D("keypad_state = %d\n", keypad_state);
	return (BOOL) keypad_state;
}

static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	IFACE_DATA_T iface_data;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	iface_data.port = app->port;

	if (app_instance->timer_handle != 0) {
		iface_data.handle = app_instance->timer_handle;
		status |= DL_ClkStopTimer(&iface_data);
	}

	if (period != 0) {
		DL_ClkStartCyclicalTimer(&iface_data, period, APP_TIMER_DO_LIGHTS);
		status |= app_instance->timer_handle = iface_data.handle;
	}

	return status;
}

static UINT32 ConvertDecToHexColor(UINT32 decimal, WCHAR *heximal) {
	UINT32 status;
	UINT16 hex_buf[MAX_HEX_LENGTH];
	WCHAR hex_str[MAX_HEX_LENGTH];
	UINT16 length;
	INT16 i;
	UINT32 dec;

	status = RESULT_OK;
	length = 0;
	dec = decimal;

	if (dec == 0) {
		heximal[0] = L'0';
		heximal[1] = L'0';
		heximal[2] = L'0';
		heximal[3] = 0;
		return status;
	}

	while (dec != 0) {
		hex_buf[length] = dec % 16;
		dec = dec / 16;
		length += 1;
	}

	length -= 1;

	for (i = length; i >= 0; --i) {
		if (hex_buf[i] < 10) {
			hex_str[length - i] = L'0' + hex_buf[i];
		} else {
			hex_str[length - i] = L'A' + hex_buf[i] - 10;
		}
	}

	switch (length + 1) {
		case 1:
			heximal[0] = L'0';
			heximal[1] = L'0';
			heximal[2] = hex_str[0];
			heximal[3] = 0;
			break;
		case 2:
			heximal[0] = L'0';
			heximal[1] = hex_str[0];
			heximal[2] = hex_str[1];
			heximal[3] = 0;
			break;
		case 3:
		default:
			heximal[0] = hex_str[0];
			heximal[1] = hex_str[1];
			heximal[2] = hex_str[2];
			heximal[3] = 0;
			break;
	}

	return status;
}

static UINT32 ConvertHexColorToDec(const WCHAR *heximal, UINT32 *number) {
	UINT32 status;
	UINT32 value;
	char hex_buf[MAX_HEX_LENGTH];

	status = RESULT_OK;

	u_utoa(heximal, hex_buf);

	value = strtoul(hex_buf, NULL, 16);
	if (value > MAX_HEX_VALUE) {
		value = MAX_HEX_VALUE;
	}

	D("Value: %d 0x%03X\n", value, value);
	D("Value R: %d 0x%03X\n", (value >> 8) & 0xF, (value >> 8) & 0xF);
	D("Value G: %d 0x%03X\n", (value >> 4) & 0xF, (value >> 4) & 0xF);
	D("Value B: %d 0x%03X\n", (value >> 0) & 0xF, (value >> 0) & 0xF);

	*number = value;

	return status;
}

static UINT32 StartLights(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	status |= StopLights(ev_st, app);
	app_instance->is_lights_started = TRUE;
	status |= SetLoopTimer(app, app_instance->options.delay);

	return status;
}

static UINT32 ProcessLights(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	if (!app_instance->is_lights_started) {
		HAPI_LP393X_set_tri_color_led(0, 0x000);
		HAPI_LP393X_set_tri_color_led(1, 0x000);
		return status;
	}

	switch (app_instance->options.mode) {
		case APP_SELECT_ITEM_AMBILIGHT:
			Ambilight(ev_st, app);
			break;
		case APP_SELECT_ITEM_COLOR:
			HAPI_LP393X_set_tri_color_led(0, app_instance->options.color);
			break;
		case APP_SELECT_ITEM_COLOR_BLINK:
			Stroboscope(ev_st, app, FALSE, TRUE, FALSE);
			break;
		case APP_SELECT_ITEM_FLASH_25:
			HAPI_LP393X_set_tri_color_led(1, 0x444);
			break;
		case APP_SELECT_ITEM_FLASH_50:
			HAPI_LP393X_set_tri_color_led(1, 0x888);
			break;
		case APP_SELECT_ITEM_FLASH_100:
			HAPI_LP393X_set_tri_color_led(1, 0xFFF);
			break;
		case APP_SELECT_ITEM_NETWORK:
			PhoneState(ev_st, app, FALSE);
			break;
		case APP_SELECT_ITEM_BATTERY:
			PhoneState(ev_st, app, TRUE);
			break;
		case APP_SELECT_ITEM_RAINBOW:
			Rainbow(ev_st, app, FALSE);
			break;
		case APP_SELECT_ITEM_RANDOM:
			Rainbow(ev_st, app, TRUE);
			break;
		case APP_SELECT_ITEM_STROBOSCOPE:
			Stroboscope(ev_st, app, TRUE, FALSE, FALSE);
			break;
		case APP_SELECT_ITEM_STROBO_COLOR:
			Stroboscope(ev_st, app, TRUE, TRUE, TRUE);
			break;
		case APP_SELECT_ITEM_BACKLIGHT:
			Backlight(ev_st, app);
			break;
		default:
			break;
	}

	return status;
}

static UINT32 StopLights(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	app_instance->is_lights_started = FALSE;
	app_instance->blink = 0;

	status |= SetLoopTimer(app, 0);
	status |= APP_UtilStopTimer(app);

	HAPI_LP393X_set_tri_color_led(0, 0x000);
	HAPI_LP393X_set_tri_color_led(1, 0x000);

	return status;
}

static UINT32 Ambilight(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	INT32 result;
	AHIDRVINFO_T *ahi_driver_info;
	AHIDEVICE_T ahi_device;
	AHIDEVCONTEXT_T ahi_device_context;
	AHISURFACE_T ahi_surface;
	AHISURFINFO_T ahi_surface_info;
	AHIBITMAP_T ahi_bitmap;
	AHIRECT_T ahi_rect;
	AHIPOINT_T ahi_point;
	UINT16 rgb444;

	status = RESULT_OK;
	result = RESULT_OK;

	if (IsKeyPadLocked()) {
		HAPI_LP393X_set_tri_color_led(0, 0x000);
		return status;
	}

	ahi_driver_info = suAllocMem(sizeof(AHIDRVINFO_T), &result);
	if (!ahi_driver_info && result) {
		return RESULT_FAIL;
	}
	status |= AhiDevEnum(&ahi_device, ahi_driver_info, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}
	status |= AhiDevOpen(&ahi_device_context, ahi_device, "Ambilight", 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}

	status |= AhiDispSurfGet(ahi_device_context, &ahi_surface);
	status |= AhiDrawSurfDstSet(ahi_device_context, ahi_surface, 0);
	status |= AhiDrawClipDstSet(ahi_device_context, NULL);
	status |= AhiDrawClipSrcSet(ahi_device_context, NULL);

	status |= AhiSurfInfo(ahi_device_context, ahi_surface, &ahi_surface_info);

	ahi_bitmap.width = ahi_surface_info.width;
	ahi_bitmap.height = ahi_surface_info.height;
	ahi_bitmap.stride =
		ahi_surface_info.width * (ahi_surface_info.byteSize / (ahi_surface_info.width * ahi_surface_info.height));
	ahi_bitmap.format = AHIFMT_16BPP_565;
	ahi_bitmap.image  = (void *) display_source_buffer;

	ahi_rect.x1 = 0;
	ahi_rect.y1 = 0;
	ahi_rect.x2 = 0 + ahi_surface_info.width;
	ahi_rect.y2 = 0 + 1; /* 1-pixel line. */

	ahi_point.x = 0;
	ahi_point.y = 0;

	status |= AhiSurfCopy(ahi_device_context, ahi_surface, &ahi_bitmap, &ahi_rect, &ahi_point, 0, AHIFLAG_COPYFROM);

	rgb444 = GetAverageColorRGB444(
		ev_st,
		app,
		(AMBILIGHT_RECT_CONSTANT * 2),
		ahi_surface_info.width - (AMBILIGHT_RECT_CONSTANT * 2) * 2
	);

	HAPI_LP393X_set_tri_color_led(0, rgb444);

	status |= AhiDevClose(ahi_device_context);
	if (ahi_driver_info) {
		suFreeMem(ahi_driver_info);
	}

	return status;
}

static UINT32 PhoneState(EVENT_STACK_T *ev_st, APPLICATION_T *app, PHONE_STATE_T state) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	UINT8 percent;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	app_instance->blink += 1;
	if (app_instance->blink > NETWORK_MS_GAP_CONSTANT) {
		switch (state) {
			case STATE_NETWORK_SIGNAL: {
					SIGNAL_STRENGTH_T signal_strength;
					DL_SigRegQuerySignalStrength(&signal_strength);
					percent = signal_strength.percent;
				}
				break;
			case STATE_BATTERY:
				percent = DL_PwrGetActiveBatteryPercent();
				break;
			default:
				break;
		}

		D("state=%d, percent=%d\n", state, percent);

		if (percent < 5) {
			HAPI_LP393X_set_tri_color_led(0, 0xF00); /* Red. */
		} else if (percent < 20) {
			HAPI_LP393X_set_tri_color_led(0, 0xF50); /* Orange. */
		} else if (percent < 60) {
			HAPI_LP393X_set_tri_color_led(0, 0xFF0); /* Yellow. */
		} else {
			HAPI_LP393X_set_tri_color_led(0, 0x0F0); /* Green. */
		}
		if (app_instance->blink > NETWORK_MS_GAP_CONSTANT + 2) {
			app_instance->blink = 0;
			HAPI_LP393X_set_tri_color_led(0, 0x000);
		}
	}

	return status;
}

static UINT32 Rainbow(EVENT_STACK_T *ev_st, APPLICATION_T *app, BOOL random_colors) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	if (IsKeyPadLocked()) {
		HAPI_LP393X_set_tri_color_led(0, 0x000);
		return status;
	}

	if (
		app_instance->color_current.r != app_instance->color_pattern.r ||
		app_instance->color_current.g != app_instance->color_pattern.g ||
		app_instance->color_current.b != app_instance->color_pattern.b
	) {
		if (app_instance->color_current.r < app_instance->color_pattern.r) {
			app_instance->color_current.r += 1;
		}
		if (app_instance->color_current.r > app_instance->color_pattern.r) {
			app_instance->color_current.r -= 1;
		}
		if (app_instance->color_current.g < app_instance->color_pattern.g) {
			app_instance->color_current.g += 1;
		}
		if (app_instance->color_current.g > app_instance->color_pattern.g) {
			app_instance->color_current.g -= 1;
		}
		if (app_instance->color_current.b < app_instance->color_pattern.b) {
			app_instance->color_current.b += 1;
		}
		if (app_instance->color_current.b > app_instance->color_pattern.b) {
			app_instance->color_current.b -= 1;
		}
	} else {
		if (random_colors) {
			app_instance->color_pattern.r = rand() % 16;
			app_instance->color_pattern.g = rand() % 16;
			app_instance->color_pattern.b = rand() % 16;
		} else {
			switch (app_instance->rainbow) {
				case RAINBOW_RED:
					app_instance->rainbow = RAINBOW_ORANGE;
					app_instance->color_pattern.r = 0xF;
					app_instance->color_pattern.g = 0x5;
					app_instance->color_pattern.b = 0x0;
					break;
				case RAINBOW_ORANGE:
					app_instance->rainbow = RAINBOW_YELLOW;
					app_instance->color_pattern.r = 0xF;
					app_instance->color_pattern.g = 0xF;
					app_instance->color_pattern.b = 0x0;
					break;
				case RAINBOW_YELLOW:
					app_instance->rainbow = RAINBOW_GREEN;
					app_instance->color_pattern.r = 0x0;
					app_instance->color_pattern.g = 0xF;
					app_instance->color_pattern.b = 0x0;
					break;
				case RAINBOW_GREEN:
					app_instance->rainbow = RAINBOW_LIGHT_BLUE;
					app_instance->color_pattern.r = 0x3;
					app_instance->color_pattern.g = 0xC;
					app_instance->color_pattern.b = 0xF;
					break;
				case RAINBOW_LIGHT_BLUE:
					app_instance->rainbow = RAINBOW_BLUE;
					app_instance->color_pattern.r = 0x0;
					app_instance->color_pattern.g = 0x0;
					app_instance->color_pattern.b = 0xF;
					break;
				case RAINBOW_BLUE:
					app_instance->rainbow = RAINBOW_VIOLET;
					app_instance->color_pattern.r = 0x8;
					app_instance->color_pattern.g = 0x0;
					app_instance->color_pattern.b = 0xF;
					break;
				case RAINBOW_VIOLET:
					app_instance->rainbow = RAINBOW_WHITE;
					app_instance->color_pattern.r = 0xF;
					app_instance->color_pattern.g = 0xF;
					app_instance->color_pattern.b = 0xF;
					break;
				case RAINBOW_WHITE:
					app_instance->rainbow = RAINBOW_RED;
					app_instance->color_pattern.r = 0xF;
					app_instance->color_pattern.g = 0x0;
					app_instance->color_pattern.b = 0x0;
					break;
				default:
					break;
			}
		}
	}

	HAPI_LP393X_set_tri_color_led(
		0,
		(app_instance->color_current.r << 8 | app_instance->color_current.g << 4 | app_instance->color_current.b)
	);

	return status;
}

static UINT32 Stroboscope(EVENT_STACK_T *ev_st, APPLICATION_T *app, BOOL flashlight, BOOL color, BOOL random) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	app_instance->blink = !app_instance->blink;

	if (app_instance->blink) {
		if (color) {
			if (random) {
				HAPI_LP393X_set_tri_color_led(0, ((rand() % 16) << 8 | (rand() % 16) << 4 | (rand() % 16)));
			} else {
				HAPI_LP393X_set_tri_color_led(0, app_instance->options.color);
			}
		} else {
			HAPI_LP393X_set_tri_color_led(0, 0xFFF);
		}
		if (flashlight) {
			HAPI_LP393X_set_tri_color_led(1, 0x888);
		}
	} else {
		HAPI_LP393X_set_tri_color_led(0, 0x000);
		if (flashlight) {
			HAPI_LP393X_set_tri_color_led(1, 0x000);
		}
	}

	return status;
}

static UINT16 GetAverageColorRGB444(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT16 start, UINT16 size) {
	UINT16 rgb565;
	UINT16 rgb444;
	UINT32 r;
	UINT32 g;
	UINT32 b;
	UINT16 i;

	r = 0;
	g = 0;
	b = 0;

	for (i = start; i < start + size; ++i) {
		rgb565 = ((UINT16 *) display_source_buffer)[i];

		D("Color RGB565: 0x%04X\n", rgb565);

		r += ((rgb565 & 0xF800) >> 11) << 3;
		g += ((rgb565 & 0x07E0) >>  5) << 2;
		b += ((rgb565 & 0x001F) <<  3) << 0;
	}

	r /= size;
	g /= size;
	b /= size;

	D("Average RGB565: 0x%02X 0x%02X 0x%02X\n", r, g, b);

	rgb444 = (((r & 0xF0) >> 4) << 8) | (((g & 0xF0) >> 4) << 4) | (((b & 0xF0) >> 4) << 0);

	D("Color RGB444: 0x%03X\n", rgb444);

	return rgb444;
}

static UINT32 Backlight(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	UINT8 light;
	UINT8 brightness;

	status = RESULT_OK;

	if (IsKeyPadLocked()) {
		return status;
	}

	light = HAPI_ATOD_convert_ambient_light_sensor();
	brightness = (light * 100) / LIGHT_SENSOR_MAX_VALUE;
	D("light=%d (0x%02X), brightness=%d\n", light, light, brightness);

	HAPI_LP393X_disp_backlight_intensity(100 - brightness);

	return status;
}
#endif
