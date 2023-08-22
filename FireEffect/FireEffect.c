/*
 * About:
 *   The "Fire Effect" demo is a port of Doom PSX fire splash screen implemented as ELF application.
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * Additional information:
 *   https://fabiensanglard.net/doom_fire_psx
 *   https://github.com/EXL/Stuff/tree/master/Sandbox/SDL/doom_fire_demo
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

#define TIMER_FAST_TRIGGER_MS             (1)
#if defined(FPS_15)
#define TIMER_FAST_UPDATE_MS              (1000 / 15) /* ~15 FPS. */
#elif defined(FPS_30)
#define TIMER_FAST_UPDATE_MS              (1000 / 30) /* ~30 FPS. */
#endif
#define KEYPAD_BUTTONS                    (8)
#define BITMAP_WIDTH                      (64)
#define BITMAP_HEIGHT                     (48)
#define START_Y_COORD                     (220)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0xE398,
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

	UINT8 *p_fire;
	UINT16 y_coord;
	BOOL flag_restart_demo;

	APP_AHI_T ahi;
	APP_KEYBOARD_T keys;
	UINT32 timer_handle;
	UINT8 keyboard_volume_level;
} APP_INSTANCE_T;

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments);                                /* ElfPack 2.x entry point. */
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period);

static UINT32 CheckKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ProcessKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 key, BOOL pressed);

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static void FPS_Meter(void);

static UINT32 ATI_Driver_Start(APPLICATION_T *app);
static UINT32 ATI_Driver_Stop(APPLICATION_T *app);
static UINT32 ATI_Driver_Flush(APPLICATION_T *app);

static UINT32 GFX_Draw_Start(APPLICATION_T *app);
static UINT32 GFX_Draw_Stop(APPLICATION_T *app);
static UINT32 GFX_Draw_Step(APPLICATION_T *app);

static BOOL Fire_Demo_Is_Screen_Empty(APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "FireEffect";

static const UINT32 fire_palette[] = {
	ATI_565RGB(0x07, 0x07, 0x07), /*  0 */
	ATI_565RGB(0x1F, 0x07, 0x07), /*  1 */
	ATI_565RGB(0x2F, 0x0F, 0x07), /*  2 */
	ATI_565RGB(0x47, 0x0F, 0x07), /*  3 */
	ATI_565RGB(0x57, 0x17, 0x07), /*  4 */
	ATI_565RGB(0x67, 0x1F, 0x07), /*  5 */
	ATI_565RGB(0x77, 0x1F, 0x07), /*  6 */
	ATI_565RGB(0x8F, 0x27, 0x07), /*  7 */
	ATI_565RGB(0x9F, 0x2F, 0x07), /*  8 */
	ATI_565RGB(0xAF, 0x3F, 0x07), /*  9 */
	ATI_565RGB(0xBF, 0x47, 0x07), /* 10 */
	ATI_565RGB(0xC7, 0x47, 0x07), /* 11 */
	ATI_565RGB(0xDF, 0x4F, 0x07), /* 12 */
	ATI_565RGB(0xDF, 0x57, 0x07), /* 13 */
	ATI_565RGB(0xDF, 0x57, 0x07), /* 14 */
	ATI_565RGB(0xD7, 0x5F, 0x07), /* 15 */
	ATI_565RGB(0xD7, 0x5F, 0x07), /* 16 */
	ATI_565RGB(0xD7, 0x67, 0x0F), /* 17 */
	ATI_565RGB(0xCF, 0x6F, 0x0F), /* 18 */
	ATI_565RGB(0xCF, 0x77, 0x0F), /* 19 */
	ATI_565RGB(0xCF, 0x7F, 0x0F), /* 20 */
	ATI_565RGB(0xCF, 0x87, 0x17), /* 21 */
	ATI_565RGB(0xC7, 0x87, 0x17), /* 22 */
	ATI_565RGB(0xC7, 0x8F, 0x17), /* 23 */
	ATI_565RGB(0xC7, 0x97, 0x1F), /* 24 */
	ATI_565RGB(0xBF, 0x9F, 0x1F), /* 25 */
	ATI_565RGB(0xBF, 0x9F, 0x1F), /* 26 */
	ATI_565RGB(0xBF, 0xA7, 0x27), /* 27 */
	ATI_565RGB(0xBF, 0xA7, 0x27), /* 28 */
	ATI_565RGB(0xBF, 0xAF, 0x2F), /* 29 */
	ATI_565RGB(0xB7, 0xAF, 0x2F), /* 30 */
	ATI_565RGB(0xB7, 0xB7, 0x2F), /* 31 */
	ATI_565RGB(0xB7, 0xB7, 0x37), /* 32 */
	ATI_565RGB(0xCF, 0xCF, 0x6F), /* 33 */
	ATI_565RGB(0xDF, 0xDF, 0x9F), /* 34 */
	ATI_565RGB(0xEF, 0xEF, 0xC7), /* 35 */
	ATI_565RGB(0xFF, 0xFF, 0xFF)  /* 36 */
};

#if defined(EP2)
static ldrElf g_app_elf;
#endif

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

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	LdrStartApp(ev_code_base);

	return status;
}
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments) {
	UINT32 status;
	UINT32 ev_code_base;
	UINT32 reserve;

	if (ldrIsLoaded(g_app_name)) {
		cprint("FireEffect: Error! Application has already been loaded!\n");
		return NULL;
	}

	status = RESULT_OK;
	ev_code_base = ldrRequestEventBase();
	reserve = ev_code_base + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

	status |= APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	status |= ldrSendEvent(ev_code_base);
	g_app_elf.name = (char *) g_app_name;

	return (status == RESULT_OK) ? &g_app_elf : NULL;
}
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 0, 1, 1, 1, 0);

		app_instance->ahi.info_driver = NULL;
		app_instance->bmp_width = BITMAP_WIDTH;
		app_instance->bmp_height = BITMAP_HEIGHT;
		app_instance->p_fire = NULL;
		app_instance->timer_handle = 0;
		app_instance->keys.pressed = 0;
		app_instance->keys.released = 0;
		app_instance->flag_restart_demo = FALSE;

		DL_AudGetVolumeSetting(PHONE, &app_instance->keyboard_volume_level);
		DL_AudSetVolumeSetting(PHONE, 0);

		status |= ATI_Driver_Start((APPLICATION_T *) app_instance);

		status |= APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);

#if defined(EP2)
		g_app_elf.app = (APPLICATION_T *) app_instance;
#endif
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	DL_AudSetVolumeSetting(PHONE, app_instance->keyboard_volume_level);

	status |= GFX_Draw_Stop(app);
	status |= SetLoopTimer(app, 0);
	status |= ATI_Driver_Stop(app);
	status |= APP_Exit(ev_st, app, 0);

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#endif

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;

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
#if !defined(FTR_V600)
			dialog = UIS_CreateNullDialog(&port);
#else
			{
				DRAWING_BUFFER_T buffer;
				GRAPHIC_POINT_T point;
				point = UIS_CanvasGetDisplaySize();
				buffer.w = point.x + 1;
				buffer.h = point.y + 1;
				buffer.buf = NULL;
				dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);
			}
#endif
			DL_KeyKjavaGetKeyState(); /* Reset Keys. */

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

static UINT32 CheckKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
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
				ProcessKeyboard(ev_st, app, key, TRUE);
			}
			if (app_instance->keys.released & key) {
				/* Key Released. */
				ProcessKeyboard(ev_st, app, key, FALSE);
			}
		}
		key >>= 1;
	}

	return RESULT_OK;
}

static UINT32 ProcessKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 key, BOOL pressed) {
	#if defined(ROT_90)
		#define KK_2 MULTIKEY_4
		#define KK_UP MULTIKEY_LEFT
		#define KK_4 MULTIKEY_8
		#define KK_LEFT MULTIKEY_DOWN
		#define KK_6 MULTIKEY_2
		#define KK_RIGHT MULTIKEY_UP
		#define KK_8 MULTIKEY_6
		#define KK_DOWN MULTIKEY_RIGHT
	#elif defined(ROT_0)
		#define KK_2 MULTIKEY_2
		#define KK_UP MULTIKEY_UP
		#define KK_4 MULTIKEY_4
		#define KK_LEFT MULTIKEY_LEFT
		#define KK_6 MULTIKEY_6
		#define KK_RIGHT MULTIKEY_RIGHT
		#define KK_8 MULTIKEY_8
		#define KK_DOWN MULTIKEY_DOWN
	#endif

	switch (key) {
		case MULTIKEY_0:
		case MULTIKEY_SOFT_LEFT:
			app->exit_status = TRUE;
			break;
		case MULTIKEY_1:
			break;
		case KK_2:
		case KK_UP:
			break;
		case MULTIKEY_3:
			break;
		case KK_4:
		case KK_LEFT:
			break;
		case MULTIKEY_5:
		case MULTIKEY_JOY_OK:
			break;
		case KK_6:
		case KK_RIGHT:
			break;
		case MULTIKEY_7:
			break;
		case KK_8:
		case KK_DOWN:
			break;
		case MULTIKEY_9:
		case MULTIKEY_SOFT_RIGHT:
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
			FPS_Meter();
			CheckKeyboard(ev_st, app);
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

static void FPS_Meter(void) {
#if defined(FPS_METER)
	UINT64 current_time;
	UINT32 delta;

	static UINT32 one = 0;
	static UINT64 last_time = 0;
	static UINT32 tick = 0;
	static UINT32 fps = 0;

	current_time = suPalTicksToMsec(suPalReadTime());
	delta = (UINT32) (current_time - last_time);
	last_time = current_time;

	tick = (tick + delta) / 2;
	if (tick != 0) {
		fps = 1000 * 10 / tick;
	}

	if (one > 30) {
		UtilLogStringData("FPS: %d.%d\n", fps / 10, fps % 10);
		PFprintf("FPS: %d.%d\n", fps / 10, fps % 10);
#if defined(EP2)
		cprintf("FPS: %d.%d\n", fps / 10, fps % 10);
#endif
		one = 0;
	}
	one++;
#endif
}

static UINT32 ATI_Driver_Start(APPLICATION_T *app) {
	UINT32 status;
	INT32 result;
	APP_INSTANCE_T *appi;
	AHIDEVICE_T ahi_device;
	AHIDISPMODE_T display_mode;

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

	status |= AhiDispModeGet(appi->ahi.context, &display_mode);

	status |= AhiDispSurfGet(appi->ahi.context, &appi->ahi.screen);
	appi->ahi.draw = DAL_GetDrawingSurface(DISPLAY_MAIN);

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

		LOG_ATI("ATI Display Mode: size=%dx%d, pixel_format=%d, frequency=%d, rotation=%d, mirror=%d\n",
			display_mode.size.x, display_mode.size.y,
			display_mode.pixel_format, display_mode.frequency, display_mode.rotation, display_mode.mirror);

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
	appi->ahi.bitmap.image = suAllocMem(appi->bmp_width * appi->bmp_height, &result);
	if (result) {
		return RESULT_FAIL;
	}
	appi->ahi.rect_bitmap.x1 = 0;
	appi->ahi.rect_bitmap.y1 = 0;
	appi->ahi.rect_bitmap.x2 = 0 + appi->bmp_width;
	appi->ahi.rect_bitmap.y2 = 0 + appi->bmp_height;

#if defined(ROT_90)
	appi->ahi.rect_draw.x1 = 0;
	appi->ahi.rect_draw.y1 = appi->bmp_height + 1;
	appi->ahi.rect_draw.x2 = 0 + appi->bmp_height;
	appi->ahi.rect_draw.y2 = appi->bmp_height + 1 + appi->bmp_width;
#elif defined(ROT_0)
	appi->ahi.rect_draw.x1 = appi->width / 2 - appi->bmp_width / 2;
	appi->ahi.rect_draw.y1 = appi->height / 2 - appi->bmp_height / 2;
	appi->ahi.rect_draw.x2 = (appi->width / 2 - appi->bmp_width / 2) + appi->bmp_width;
	appi->ahi.rect_draw.y2 = (appi->height / 2 - appi->bmp_height / 2) + appi->bmp_height;

	status |= AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);

	status |= AhiDrawBrushFgColorSet(appi->ahi.context, ATI_565RGB(0xFF, 0xFF, 0xFF));
	status |= AhiDrawBrushSet(appi->ahi.context, NULL, NULL, 0, AHIFLAG_BRUSH_SOLID);
	status |= AhiDrawRopSet(appi->ahi.context, AHIROP3(AHIROP_PATCOPY));
	status |= AhiDrawSpans(appi->ahi.context, &appi->ahi.update_params.rect, 1, 0);
#endif

	AhiDrawRopSet(appi->ahi.context, AHIROP3(AHIROP_SRCCOPY));

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
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

#if defined(ROT_0)
	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);
	AhiDrawBitmapBlt(appi->ahi.context,
		&appi->ahi.rect_draw, &appi->ahi.point_bitmap, &appi->ahi.bitmap, (void *) fire_palette, 0);
	AhiDispWaitVBlank(appi->ahi.context, 0);
#elif defined(ROT_90)
	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.draw, 0);
	AhiDrawBitmapBlt(appi->ahi.context,
		&appi->ahi.rect_bitmap, &appi->ahi.point_bitmap, &appi->ahi.bitmap, (void *) fire_palette, 0);

	AhiDrawRotateBlt(appi->ahi.context,
		&appi->ahi.rect_draw, &appi->ahi.point_bitmap, AHIROT_90, AHIMIRR_NO, 0);

	AhiDrawSurfSrcSet(appi->ahi.context, appi->ahi.draw, 0);
	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);

	AhiDispWaitVBlank(appi->ahi.context, 0);
	AhiDrawStretchBlt(appi->ahi.context, &appi->ahi.update_params.rect, &appi->ahi.rect_draw, AHIFLAG_STRETCHFAST);
#endif

	if (appi->is_CSTN_display) {
		AhiDispUpdate(appi->ahi.context, &appi->ahi.update_params);
	}

	return RESULT_OK;
}

static UINT32 GFX_Draw_Start(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	appi->y_coord = START_Y_COORD;
	appi->p_fire = (UINT8 *) appi->ahi.bitmap.image;

	/* Fill all screen to RGB(0x07, 0x07, 0x07) except last line. */
	memset(appi->p_fire, 0, appi->bmp_width * (appi->bmp_height - 1));

	/* Fill last line to RGB(0xFF, 0xFF, 0xFF) except last line. */
	memset((UINT8 *) (appi->p_fire + (appi->bmp_height - 1) * appi->bmp_width), 36, appi->bmp_width);

	return RESULT_OK;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	if (appi->p_fire) {
		suFreeMem(appi->p_fire);
		appi->p_fire = NULL;
	}

	return RESULT_OK;
}

static UINT32 GFX_Draw_Step(APPLICATION_T *app) {
	UINT16 x;
	UINT16 y;
	UINT16 start;
	UINT16 stop;
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	if (appi->flag_restart_demo) {
		GFX_Draw_Start(app);
		appi->flag_restart_demo = FALSE;
	}

	for (x = 0; x < appi->bmp_width; ++x) {
		for (y = 1; y < appi->bmp_height; ++y) {
			const UINT8 pixel = appi->p_fire[y * appi->bmp_width + x];
			if (pixel == 0) {
				appi->p_fire[(y * appi->bmp_width + x) - appi->bmp_width] = 0;
			} else {
				const UINT8 rand_idx = rand() % 4;
				const UINT16 destination = (y * appi->bmp_width + x) - rand_idx + 1;
				appi->p_fire[destination - appi->bmp_width] = pixel - (rand_idx & 1);
			}
		}
	}

	start = appi->bmp_height - 1;
	stop = appi->bmp_height - 8;

	if (appi->y_coord != appi->bmp_height / 4) {
		appi->y_coord -= 2;
	} else {
		for(y = start; y > stop; --y) {
			for(x = 0; x < appi->bmp_width; ++x) {
				if (appi->p_fire[y * appi->bmp_width + x] > 0) {
					appi->p_fire[y * appi->bmp_width + x] -= ((rand() % 2) & 3);
				}
			}
		}
		appi->flag_restart_demo = Fire_Demo_Is_Screen_Empty(app);
	}

	return RESULT_OK;
}

static BOOL Fire_Demo_Is_Screen_Empty(APPLICATION_T *app) {
	UINT16 i;
	UINT16 stop;
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	stop = appi->bmp_width * appi->bmp_height;
	for (i = 0; i < stop; ++i) {
		if (appi->p_fire[i]) {
			return FALSE;
		}
	}

	return TRUE;
}
