/*
 * About:
 *   Port of the "Spout" game by kuni to Motorola P2K phones.
 *
 * Author:
 *   kuni, EXL
 *
 * License:
 *   MIT
 *
 * Additional information:
 *   https://exlmoto.ru/spout-droid/
 *   https://github.com/EXL/Spout
 *   http://www.susami.co.jp/kuni/junk/junk.htm
 *   https://njw.me.uk/spout/
 *
 * Application type:
 *   GUI + ATI
 */

#include <loader.h>
#include <ati.h>
#include <apps.h>
#include <dl.h>
#include <dal.h>
#include <uis.h>
#include <mem.h>
#include <time_date.h>
#include <utilities.h>

#include "Spout.h"
#include "Font.h"

/******** MOVE IT TO SDK ***/

typedef struct {
	UINT32 size;
	BOOL sync;
	AHIRECT_T rect;
} AHIUPDATEPARAMS_T;

extern UINT32 AhiDispUpdate(AHIDEVCONTEXT_T context, AHIUPDATEPARAMS_T *update_params);

/******** MOVE IT TO SDK ***/

#define TIMER_FAST_TRIGGER_MS             (1)
#define TIMER_FAST_UPDATE_MS              (1000 / 25) /* ~25 FPS. */
#define KEYPAD_BUTTONS                    (8)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0x0001,
	APP_TIMER_LOOP
} APP_TIMER_T;

typedef struct {
	AHIDRVINFO_T *info_driver;
	AHIDEVCONTEXT_T context;
	AHISURFACE_T screen;
	AHISURFACE_T draw;
	AHISURFINFO_T info_surface;
	AHIBITMAP_T bitmap;

	AHIPOINT_T point_bitmap;
	AHIRECT_T rect_bitmap;
	AHIRECT_T rect_draw;
	AHIUPDATEPARAMS_T update_params;
} APP_AHI_T;

typedef struct {
	UINT32 pressed;
	UINT32 released;
} APP_KEYBOARD_T;

typedef struct {
	APPLICATION_T app;

	BOOL is_CSTN_display;
	UINT16 width;
	UINT16 height;
	UINT16 bmp_width;
	UINT16 bmp_height;

	UINT8 *p_spout;
	UINT16 y_coord;

	APP_AHI_T ahi;
	APP_KEYBOARD_T keys;
	UINT32 timer_handle;
} APP_INSTANCE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period);

static UINT32 CheckKeyboard(APPLICATION_T *app);
static UINT32 ProcessKeyboard(APPLICATION_T *app, UINT32 key, BOOL pressed);

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 ATI_Driver_Start(APPLICATION_T *app);
static UINT32 ATI_Driver_Stop(APPLICATION_T *app);
static UINT32 ATI_Driver_Flush(APPLICATION_T *app);

static UINT32 GFX_Draw_Start(APPLICATION_T *app);
static UINT32 GFX_Draw_Stop(APPLICATION_T *app);
static UINT32 GFX_Draw_Step(APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "Spout";

static const UINT32 spout_palette[] = {
	ATI_565RGB(0xFF, 0xFF, 0xFF), /*  0 */
	ATI_565RGB(0xAA, 0xAA, 0xAA), /*  1 */
	ATI_565RGB(0x55, 0x55, 0x55), /*  2 */
	ATI_565RGB(0x00, 0x00, 0x00)
};

static EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DONE, ApplicationStop },
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls }
};

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	LdrStartApp(ev_code_base);

	return status;
}

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 0, 1, 1, 1, 0);

		app_instance->ahi.info_driver = NULL;
		app_instance->p_spout = NULL;
		app_instance->bmp_width = SPOUT_WIDTH;
		app_instance->bmp_height = SPOUT_HEIGHT;
		app_instance->timer_handle = 0;
		app_instance->keys.pressed = 0;
		app_instance->keys.released = 0;

		status |= ATI_Driver_Start((APPLICATION_T *) app_instance);

		status |= APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	DeleteDialog(app);

	status |= GFX_Draw_Stop(app);
	status |= SetLoopTimer(app, 0);
	status |= APP_Exit(ev_st, app, 0);
	status |= ATI_Driver_Stop(app);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	DRAWING_BUFFER_T buffer;
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (state != ENTER_STATE_ENTER) {
		if (app->state != APP_STATE_MAIN) {
			SetLoopTimer(app, TIMER_FAST_UPDATE_MS);
			return RESULT_OK;
		}
	}

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;
	dialog = DialogType_None;

	switch (app_state) {
		case APP_STATE_MAIN:
			buffer.w = app_instance->width;
			buffer.h = app_instance->height;
			buffer.buf = NULL;

//			dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);
			dialog = UIS_CreateNullDialog(&port);

			GFX_Draw_Start(app);

			SetLoopTimer(app, TIMER_FAST_UPDATE_MS);

			break;
		default:
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	switch (app_state) {
		case APP_STATE_MAIN:
			DL_KeyKjavaGetKeyState(); /* Reset Keys. */
			GFX_Draw_Step(app);
			ATI_Driver_Flush(app);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state) {
	if (state == EXIT_STATE_EXIT) {
		if (app->state != APP_STATE_MAIN) {
			DeleteDialog(app);
		}
		SetLoopTimer(app, 0);
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
		DL_ClkStartCyclicalTimer(&iface_data, period, APP_TIMER_LOOP);
		status |= app_instance->timer_handle = iface_data.handle;
	}

	return status;
}

static UINT32 CheckKeyboard(APPLICATION_T *app) {
	UINT32 key;
	APP_INSTANCE_T *app_instance;

	key = 0x00080000;

	app_instance = (APP_INSTANCE_T *) app;
	app_instance->keys.released = app_instance->keys.pressed;
	app_instance->keys.pressed = DL_KeyKjavaGetKeyState();

	while (key) {
		if ((app_instance->keys.released & key) != (app_instance->keys.pressed & key)) {
			if (app_instance->keys.pressed & key) {
				/* Key Pressed. */
				ProcessKeyboard(app, key, TRUE);
			}
			if (app_instance->keys.released & key) {
				/* Key Released. */
				ProcessKeyboard(app, key, FALSE);
			}
		}
		key >>= 1;
	}

	return RESULT_OK;
}

typedef enum {
	KPB_UP,
	KPB_DOWN,
	KPB_LEFT,
	KPB_RIGHT,
	KPB_A,
	KPB_B,
	KPB_C,
	KPB_D,
} KEYPAD_BUTTONS_T;

static BOOL keypad[KEYPAD_BUTTONS];
static BOOL autofire = FALSE;

static UINT32 ProcessKeyboard(APPLICATION_T *app, UINT32 key, BOOL pressed) {
	switch (key) {
		case MULTIKEY_0:
		case MULTIKEY_SOFT_LEFT:
			app->exit_status = TRUE;
			break;
		case MULTIKEY_1:
			if (pressed) {
				autofire = !autofire;
			}
			break;
		case MULTIKEY_2:
		case MULTIKEY_UP:
			keypad[KPB_UP] = pressed;
			break;
		case MULTIKEY_8:
		case MULTIKEY_DOWN:
			keypad[KPB_DOWN] = pressed;
			break;
		case MULTIKEY_4:
		case MULTIKEY_LEFT:
			keypad[KPB_LEFT] = pressed;
			break;
		case MULTIKEY_6:
		case MULTIKEY_RIGHT:
			keypad[KPB_RIGHT] = pressed;
			break;
		case MULTIKEY_5:
		case MULTIKEY_JOY_OK:
			keypad[KPB_A] = pressed;
			break;
		case MULTIKEY_7:
			keypad[KPB_B] = pressed;
			break;
		case MULTIKEY_9:
			keypad[KPB_C] = pressed;
			break;
		case MULTIKEY_3:
			keypad[KPB_D] = pressed;
			break;
		default:
			break;
	}
	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	APP_ConsumeEv(ev_st, app);

	switch (timer_id) {
		case APP_TIMER_LOOP:
			CheckKeyboard(app);
			GFX_Draw_Step(app);
			ATI_Driver_Flush(app);
			break;
		case APP_TIMER_EXIT:
			/* Play an exit sound using quiet speaker. */
			DL_AudPlayTone(0x00,  0xFF);
			return ApplicationStop(ev_st, app);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 ATI_Driver_Start(APPLICATION_T *app) {
	UINT32 status;
	INT32 result;
	APP_INSTANCE_T *appi;
	AHIDEVICE_T ahi_device;

	status = RESULT_OK;
	result = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	appi->ahi.info_driver = suAllocMem(sizeof(AHIDRVINFO_T), &result);
	if (!appi->ahi.info_driver && result) {
		return RESULT_FAIL;
	}
	status |= AhiDevEnum(&ahi_device, appi->ahi.info_driver, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}
	status |= AhiDevOpen(&appi->ahi.context, ahi_device, g_app_name, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}

	status |= AhiDispSurfGet(appi->ahi.context, &appi->ahi.screen);
	appi->ahi.draw = DAL_GetDrawingSurface(DISPLAY_MAIN);
	status |= AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);
//	status |= AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.draw, 0);
	status |= AhiDrawClipDstSet(appi->ahi.context, NULL);
	status |= AhiDrawClipSrcSet(appi->ahi.context, NULL);
	status |= AhiSurfInfo(appi->ahi.context, appi->ahi.screen, &appi->ahi.info_surface);

#define LOG_ATI(format, ...) UtilLogStringData(format, ##__VA_ARGS__); PFprintf(format, ##__VA_ARGS__)
	{
		UINT32 result;
		UINT32 size;
		UINT32 align;

		LOG_ATI("ATI Driver Name: %s\n", appi->ahi.info_driver->drvName);
		LOG_ATI("ATI Driver Version: %s\n", appi->ahi.info_driver->drvVer);
		LOG_ATI("ATI S/W Revision: %d (0x%08X)\n",
			appi->ahi.info_driver->swRevision, appi->ahi.info_driver->swRevision);
		LOG_ATI("ATI Chip ID: %d (0x%08X)\n",
			appi->ahi.info_driver->chipId, appi->ahi.info_driver->chipId);
		LOG_ATI("ATI Revision ID: %d (0x%08X)\n",
			appi->ahi.info_driver->revisionId, appi->ahi.info_driver->revisionId);
		LOG_ATI("ATI CPU Bus Interface Mode: %d (0x%08X)\n",
			appi->ahi.info_driver->cpuBusInterfaceMode, appi->ahi.info_driver->cpuBusInterfaceMode);
		LOG_ATI("ATI Total Memory: %d (%d KiB)\n",
			appi->ahi.info_driver->totalMemory, appi->ahi.info_driver->totalMemory / 1024);
		LOG_ATI("ATI Internal Memory: %d (%d KiB)\n",
			appi->ahi.info_driver->internalMemSize, appi->ahi.info_driver->internalMemSize / 1024);
		LOG_ATI("ATI External Memory: %d (%d KiB)\n",
			appi->ahi.info_driver->externalMemSize, appi->ahi.info_driver->externalMemSize / 1024);
		LOG_ATI("ATI CAPS 1: %d (0x%08X)\n", appi->ahi.info_driver->caps1, appi->ahi.info_driver->caps1);
		LOG_ATI("ATI CAPS 2: %d (0x%08X)\n", appi->ahi.info_driver->caps2, appi->ahi.info_driver->caps2);

		result = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, AHIFMT_16BPP_565,
			&size, &align, AHIFLAG_INTMEMORY);
		LOG_ATI("ATI Internal Memory Largest Block: result=%d, size=%d, size=%d KiB, align=%d\n",
			result, size, size / 1024, align);

		result = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, AHIFMT_16BPP_565,
			&size, &align, AHIFLAG_EXTMEMORY);
		LOG_ATI("ATI External Memory Largest Block: result=%d, size=%d, size=%d KiB, align=%d\n",
			result, size, size / 1024, align);

		LOG_ATI("ATI Surface Info: width=%d, height=%d, pixFormat=%d, byteSize=%d, byteSize=%d KiB\n",
			appi->ahi.info_surface.width, appi->ahi.info_surface.height, appi->ahi.info_surface.pixFormat,
			appi->ahi.info_surface.byteSize, appi->ahi.info_surface.byteSize / 1024);
		LOG_ATI("ATI Surface Info: offset=%d, stride=%d, numPlanes=%d\n",
			appi->ahi.info_surface.offset, appi->ahi.info_surface.stride, appi->ahi.info_surface.numPlanes);
	}
#undef LOG_ATI

	appi->width = appi->ahi.info_surface.width;
	appi->height = appi->ahi.info_surface.height;

	appi->ahi.update_params.size = sizeof(AHIUPDATEPARAMS_T);
	appi->ahi.update_params.sync = FALSE;
	appi->ahi.update_params.rect.x1 = 0;
	appi->ahi.update_params.rect.y1 = 0;
	appi->ahi.update_params.rect.x2 = 0 + appi->width;
	appi->ahi.update_params.rect.y2 = 0 + appi->height;
	appi->ahi.point_bitmap.x = 0;
	appi->ahi.point_bitmap.y = 0;
	appi->is_CSTN_display = (appi->width < DISPLAY_WIDTH) || (appi->height < DISPLAY_HEIGHT); /* Motorola L6 */

	appi->ahi.bitmap.width = appi->bmp_width;
	appi->ahi.bitmap.height = appi->bmp_height;
	appi->ahi.bitmap.stride = appi->bmp_width; /* (width * bpp) */
	appi->ahi.bitmap.format = AHIFMT_8BPP;
	appi->ahi.bitmap.image = vbuff;
	if (result) {
		return RESULT_FAIL;
	}
	appi->ahi.rect_bitmap.x1 = 0;
	appi->ahi.rect_bitmap.y1 = 0;
	appi->ahi.rect_bitmap.x2 = 0 + appi->bmp_width;
	appi->ahi.rect_bitmap.y2 = 0 + appi->bmp_height;

	appi->ahi.rect_draw.x1 = appi->width / 2 - appi->bmp_width / 2;
	appi->ahi.rect_draw.y1 = appi->height / 2 - appi->bmp_height / 2;
	appi->ahi.rect_draw.x2 = (appi->width / 2 - appi->bmp_width / 2) + appi->bmp_width;
	appi->ahi.rect_draw.y2 = (appi->height / 2 - appi->bmp_height / 2) + appi->bmp_height;

	status |= AhiDrawBrushFgColorSet(appi->ahi.context, ATI_565RGB(0x00, 0x00, 0x00));
	status |= AhiDrawBrushSet(appi->ahi.context, NULL, NULL, 0, AHIFLAG_BRUSH_SOLID);
	status |= AhiDrawRopSet(appi->ahi.context, AHIROP3(AHIROP_PATCOPY));
	status |= AhiDrawSpans(appi->ahi.context, &appi->ahi.update_params.rect, 1, 0);

	return status;
}

static UINT32 ATI_Driver_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	status |= AhiDevClose(app_instance->ahi.context);
	if (app_instance->ahi.info_driver) {
		suFreeMem(app_instance->ahi.info_driver);
	}

	return status;
}

static UINT32 ATI_Driver_Flush(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

//	status |= AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.draw, 0);
	status |= AhiDrawRopSet(appi->ahi.context, AHIROP3(AHIROP_SRCCOPY));
	status |= AhiDrawBitmapBlt(appi->ahi.context,
		&appi->ahi.rect_draw, &appi->ahi.point_bitmap, &appi->ahi.bitmap, (void *) spout_palette, 0);

//	status |= AhiDrawSurfSrcSet(appi->ahi.context, appi->ahi.draw, 0);
//	status |= AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);

//	status |= AhiDispWaitVBlank(appi->ahi.context, 0);
	/* 1 - AHIFLAG_STRETCHFAST */
//	status |= AhiDrawStretchBlt(appi->ahi.context, &appi->ahi.rect_draw, &appi->ahi.rect_bitmap, 1);

	/*
	status |= AhiDrawRotateBlt(appi->ahi.context,
		&appi->ahi.rect_screen, &appi->ahi.point_bitmap, AHIROT_90, AHIMIRR_NO, 0);
	*/

	if (appi->is_CSTN_display) {
		status |= AhiDispUpdate(appi->ahi.context, &appi->ahi.update_params);
	}

	return status;
}

static UINT32 GFX_Draw_Start(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	pceAppInit();

	return status;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	pceAppExit();

	return status;
}

static UINT32 GFX_Draw_Step(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	pceAppProc(0);

	return status;
}

void pceLCDDispStop(void) { }

void pceLCDDispStart(void) { }

void pceLCDTrans(void) {
#if 0 /* Use direct videobuffer drawing without these copying convertions. */
	int x, y;
	unsigned char *vbi, *bi;

	bi = app_p->ahi.bitmap.image;
	for(y = 0; y < SDL_HEIGHT; y ++) {
		vbi = vBuffer + (y / ZOOM) * 128;
		for(x = 0; x < SDL_WIDTH; x ++) {
			*bi ++ = *(vbi + x / ZOOM);
		}
		bi += app_p->ahi.bitmap.stride - SDL_WIDTH;
	}
#endif
}

int pcePadGet(void) {
	static int pad = 0;
	int i = 0, op = pad & 0x00ff;
	int p[] = { PAD_UP, PAD_DN, PAD_LF, PAD_RI, PAD_A, PAD_B, PAD_C, PAD_D, -1 };

	pad = 0;

	do {
		if(keypad[i]) {
			pad |= p[i];
		}
		i++;
	} while(p[i] >= 0);

	if (autofire) {
		pad |= PAD_A;
	}

	pad |= (pad & (~op)) << 8;
	return pad;
}

int interval = 0;

void pceAppSetProcPeriod(int period) {
	interval = period;
}

int exec = 1;

void pceAppReqExit(int c) {
	exec = 0;
}

unsigned char *pceLCDSetBuffer(unsigned char *pbuff) {
	return NULL;
}

int font_posX = 0, font_posY = 0, font_width = 4, font_height = 6;
unsigned char font_fgcolor = 3, font_bgcolor = 0, font_bgclear = 0;
const char *font_adr = (char *) FONT6;

void pceFontSetType(int type) {
	const int width[] = {5, 8, 4};
	const int height[] = {10, 16, 6};
	const char* adr[] = { (char *) FONT6, (char *) FONT16, (char *) FONT6 };

	type &= 3;
	font_width = width[type];
	font_height = height[type];
	font_adr = adr[type];
}

void pceFontSetTxColor(int color)
{
	font_fgcolor = (unsigned char)color;
}

void pceFontSetBkColor(int color)
{
	if(color >= 0) {
		font_bgcolor = (unsigned char)color;
		font_bgclear = 0;
	} else {
		font_bgclear = 1;
	}
}

void pceFontSetPos(int x, int y)
{
	font_posX = x;
	font_posY = y;
}

int pceFontPrintf(const char *fmt, ...)
{
	unsigned char *adr = vbuff + font_posX + font_posY * 128;
	unsigned char *pC;
	char c[1024];
	va_list argp;

	va_start(argp, fmt);
	vsprintf(c, fmt, argp);
	va_end(argp);

	pC = (unsigned char *) c;
	while(*pC) {
		int i, x, y;
		const unsigned char *sAdr;
		if(*pC >= 0x20 && *pC < 0x80) {
			i = *pC - 0x20;
		} else {
			i = 0;
		}
		sAdr = (unsigned char *) (font_adr + (i & 15) + (i >> 4) * 16 * 16);
		for(y = 0; y < font_height; y ++) {
			unsigned char c = *sAdr;
			for(x = 0; x < font_width; x ++) {
				if(c & 0x80) {
					*adr = font_fgcolor;
				} else if(font_bgclear == 0) {
					*adr = font_bgcolor;
				}
				adr ++;
				c <<= 1;
			}
			adr += 128 - font_width;
			sAdr += 16;
		}
		adr -= 128 * font_height - font_width;
		pC ++;
	}
	return 0;
}

int pceFileCreate(const char *fname, int mode) {
	return 0;
}

int pceFileOpen(FILEACC *pfa, const char *fname, int mode)
{
	return RESULT_OK;
#if 0
	if(mode == FOMD_RD) {
		*pfa = open(fname, O_RDONLY | O_BINARY);
	} else if(mode == FOMD_WR) {
		*pfa = open(fname, O_CREAT | O_RDWR | O_BINARY | O_TRUNC, S_IREAD | S_IWRITE);
	}

	if(*pfa >= 0) {
		return 0;
	} else {
		return 1;
	}
#endif
}

int pceFileReadSct(FILEACC *pfa, void *ptr, int sct, int len)
{
//	return read(*pfa, ptr, len);
	return 0;
}

int pceFileWriteSct(FILEACC *pfa, const void *ptr, int sct, int len)
{
//	return write(*pfa, ptr, len);
	return 0;
}

int pceFileClose(FILEACC *pfa)
{
//	close(*pfa);
	return 0;
}
