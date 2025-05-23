/*
 * About:
 *   The "Yeti3D Old" is a port of old 3D engine demo from GBA to Motorola P2K ELF application.
 *
 * Author:
 *   Derek J. Evans, EXL
 *
 * License:
 *   GPLv2
 *
 * Additional information:
 *   https://www.gbadev.org/demos.php?showinfo=568
 *
 * Application type:
 *   GUI + ATI, Nvidia
 */

#include "yeti.h"

#include <loader.h>
#include <apps.h>
#include <dl.h>
#include <dl_keypad.h>
#include <dal.h>
#include <filesystem.h>
#include <uis.h>
#include <canvas.h>
#include <mem.h>
#include <time_date.h>
#include <utilities.h>
#if defined(EP1) || defined(EP2)
#include <ati.h>
#elif defined(EM1) || defined(EM2)
#include <nvidia.h>
#endif

#define TIMER_FAST_TRIGGER_MS             (1)
#if defined(FPS_15)
#define TIMER_FAST_UPDATE_MS              (1000 / 15) /* ~15 FPS. */
#elif defined(FPS_30)
#define TIMER_FAST_UPDATE_MS              (1000 / 30) /* ~30 FPS. */
#endif
#define KEYPAD_BUTTONS                    (8)

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

#if defined(EP1) || defined(EP2)
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
#elif defined(EM1) || defined(EM2)
typedef struct {
	GF_HANDLE gxHandle;
	GXRECT fb0_rect;
	UINT8 *fb0;
} APP_GFSDK_T;
#endif

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

	UINT8 *p_bitmap;

#if defined(EP1) || defined(EP2)
	APP_AHI_T ahi;
#elif defined(EM1) || defined(EM2)
	APP_GFSDK_T gfsdk;
#endif
	APP_KEYBOARD_T keys;
	UINT32 timer_handle;
	UINT8 keyboard_volume_level;
} APP_INSTANCE_T;

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments);                                /* ElfPack 2.x entry point. */
#elif defined(EM1)
int _main(ElfLoaderApp ela);                                                 /* ElfPack 1.x M*CORE entry point. */
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments);                             /* ElfPack 2.x M*CORE entry point. */
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

#if defined(EP1) || defined(EP2)
static UINT32 ATI_Driver_Start(APPLICATION_T *app);
static UINT32 ATI_Driver_Stop(APPLICATION_T *app);
static UINT32 ATI_Driver_Flush(APPLICATION_T *app);
#elif defined(EM1) || defined(EM2)
static UINT32 Nvidia_Driver_Start(APPLICATION_T *app);
static UINT32 Nvidia_Driver_Stop(APPLICATION_T *app);
static UINT32 Nvidia_Driver_Flush(APPLICATION_T *app);
#endif

static UINT32 GFX_Draw_Start(APPLICATION_T *app);
static UINT32 GFX_Draw_Stop(APPLICATION_T *app);
static UINT32 GFX_Draw_Step(APPLICATION_T *app);

static UINT32 InitResourses(void);
static void FreeResourses(void);

static const char g_app_name[APP_NAME_LEN] = "FireEffect";

#if defined(EP2)
static ldrElf g_app_elf;
#elif defined(EM1)
static ElfLoaderApp g_app_elf = { 0 };
#elif defined(EM2)
static ldrElf *g_app_elf = NULL;
#endif

typedef enum {
	E_KEY_UP,
	E_KEY_DOWN,
	E_KEY_LEFT,
	E_KEY_RIGHT,
	E_KEY_MAX
} GAME_KEY_T;
static BOOL g_keyboard[E_KEY_MAX] = { FALSE };

static WCHAR g_res_file_path[FS_MAX_URI_NAME_LENGTH];

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

	u_atou(elf_path_uri, g_res_file_path);

	LdrStartApp(ev_code_base);

	return status;
}
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments) {
	UINT32 status;
	UINT32 ev_code_base;
	UINT32 reserve;

	if (ldrIsLoaded(g_app_name)) {
		cprint("Yeti3D-Old: Error! Application has already been loaded!\n");
		return NULL;
	}

	status = RESULT_OK;
	ev_code_base = ldrRequestEventBase();
	reserve = ev_code_base + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

	status |= APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	u_strcpy(g_res_file_path, uri);

	status |= ldrSendEvent(ev_code_base);
	g_app_elf.name = (char *) g_app_name;

	return (status == RESULT_OK) ? &g_app_elf : NULL;
}
#elif defined(EM1)
int _main(ElfLoaderApp ela) {
	UINT32 status;

	status = RESULT_OK;

	memcpy((void *) &g_app_elf, (void *) &ela, sizeof(ElfLoaderApp));

	status = APP_Register(&g_app_elf.evcode, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	u_strcpy(g_res_file_path, uri);

	LoaderShowApp(&g_app_elf);

	return RESULT_FAIL;
}
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments) {
	UINT32 status;
	UINT32 reserve;
	WCHAR *ptr;

	status = RESULT_OK;
	g_app_elf = elf;
	g_app_elf->name = (char *) g_app_name;

	if (ldrIsLoaded(g_app_elf->name)) {
		PFprintf("%s: Application already loaded.\n", g_app_elf->name);
		return RESULT_FAIL;
	}

	reserve = g_app_elf->evbase + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

	status |= APP_Register(&g_app_elf->evbase, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	if (status == RESULT_OK) {
		PFprintf("%s: Application has been registered successfully.\n", g_app_elf->name);

		ptr = NULL;
		u_strcpy(g_res_file_path, L"file:/");
		ptr = g_res_file_path + u_strlen(g_res_file_path);
		DL_FsGetURIFromID(&g_app_elf->id, ptr);

		status |= ldrSendEvent(g_app_elf->evbase);
	} else {
		PFprintf("%s: Cannot register application.\n", g_app_elf->name);
	}

	return status;
}
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 0, 1, 1, 1, 0);

#if defined(EP1) || defined(EP2)
		app_instance->ahi.info_driver = NULL;
#elif defined(EM1) || defined(EM2)
		app_instance->gfsdk.gxHandle = (GF_HANDLE) &GxHandle;
		app_instance->gfsdk.fb0 = NULL;
#endif
		app_instance->bmp_width = VIEWPORT_WIDTH;
		app_instance->bmp_height = VIEWPORT_HEIGHT;
		app_instance->p_bitmap = NULL;
		app_instance->timer_handle = 0;
		app_instance->keys.pressed = 0;
		app_instance->keys.released = 0;

		DL_AudGetVolumeSetting(PHONE, &app_instance->keyboard_volume_level);
		DL_AudSetVolumeSetting(PHONE, 0);

#if defined(EP1) || defined(EP2)
		status |= ATI_Driver_Start((APPLICATION_T *) app_instance);
#elif defined(EM1) || defined(EM2)
		status |= Nvidia_Driver_Start((APPLICATION_T *) app_instance);
#endif

		status |= APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);

#if defined(EP2)
		g_app_elf.app = (APPLICATION_T *) app_instance;
#elif defined(EM2)
		g_app_elf->app = &app_instance->app;
#endif
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	APP_ConsumeEv(ev_st, app);

	DeleteDialog(app);

	DL_AudSetVolumeSetting(PHONE, app_instance->keyboard_volume_level);

	status |= GFX_Draw_Stop(app);
	status |= SetLoopTimer(app, 0);
#if defined(EP1) || defined(EP2)
	status |= ATI_Driver_Stop(app);
#elif defined(EM1) || defined(EM2)
	status |= Nvidia_Driver_Stop(app);
#endif
	status |= APP_Exit(ev_st, app, 0);

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#elif defined(EM1)
	LoaderEndApp(&g_app_elf);
#elif defined(EM2)
	ldrUnloadElf(g_app_elf);
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
#if defined(FTR_V600) || defined(FTR_V635)
			{
				DRAWING_BUFFER_T buffer;
				GRAPHIC_POINT_T point;
				point = UIS_CanvasGetDisplaySize();
				buffer.w = point.x + 1;
				buffer.h = point.y + 1;
				buffer.buf = NULL;
				dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);
			}
#else
			dialog = UIS_CreateNullDialog(&port);
#endif
			DL_KeyKjavaGetKeyState(); /* Reset Keys. */

			if (state == ENTER_STATE_ENTER) {
				GFX_Draw_Start(app);
			}

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
	#define KK_2 MULTIKEY_6
	#define KK_UP MULTIKEY_RIGHT
	#define KK_4 MULTIKEY_2
	#define KK_LEFT MULTIKEY_UP
	#define KK_6 MULTIKEY_8
	#define KK_RIGHT MULTIKEY_DOWN
	#define KK_8 MULTIKEY_4
	#define KK_DOWN MULTIKEY_LEFT

	switch (key) {
		case MULTIKEY_0:
		case MULTIKEY_SOFT_LEFT:
			app->exit_status = TRUE;
			break;
		case MULTIKEY_1:
			break;
		case KK_2:
		case KK_UP:
			g_keyboard[E_KEY_UP] = pressed;
			break;
		case MULTIKEY_3:
			break;
		case KK_4:
		case KK_LEFT:
			g_keyboard[E_KEY_LEFT] = pressed;
			break;
		case MULTIKEY_5:
		case MULTIKEY_JOY_OK:
			break;
		case KK_6:
		case KK_RIGHT:
			g_keyboard[E_KEY_RIGHT] = pressed;
			break;
		case MULTIKEY_7:
			break;
		case KK_8:
		case KK_DOWN:
			g_keyboard[E_KEY_DOWN] = pressed;
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
#if defined(EP1) || defined(EP2)
			ATI_Driver_Flush(app);
#elif defined(EM1) || defined(EM2)
			Nvidia_Driver_Flush(app);
#endif
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

#if defined(EP1) || defined(EP2)
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

	{
		UINT32 result;
		UINT32 size;
		UINT32 align;

		LOG("ATI Driver Name: %s\n", appi->ahi.info_driver->drvName);
		LOG("ATI Driver Version: %s\n", appi->ahi.info_driver->drvVer);
		LOG("ATI S/W Revision: %d (0x%08X)\n",
			appi->ahi.info_driver->swRevision, appi->ahi.info_driver->swRevision);
		LOG("ATI Chip ID: %d (0x%08X)\n",
			appi->ahi.info_driver->chipId, appi->ahi.info_driver->chipId);
		LOG("ATI Revision ID: %d (0x%08X)\n",
			appi->ahi.info_driver->revisionId, appi->ahi.info_driver->revisionId);
		LOG("ATI CPU Bus Interface Mode: %d (0x%08X)\n",
			appi->ahi.info_driver->cpuBusInterfaceMode, appi->ahi.info_driver->cpuBusInterfaceMode);
		LOG("ATI Total Memory: %d (%d KiB)\n",
			appi->ahi.info_driver->totalMemory, appi->ahi.info_driver->totalMemory / 1024);
		LOG("ATI Internal Memory: %d (%d KiB)\n",
			appi->ahi.info_driver->internalMemSize, appi->ahi.info_driver->internalMemSize / 1024);
		LOG("ATI External Memory: %d (%d KiB)\n",
			appi->ahi.info_driver->externalMemSize, appi->ahi.info_driver->externalMemSize / 1024);
		LOG("ATI CAPS 1: %d (0x%08X)\n", appi->ahi.info_driver->caps1, appi->ahi.info_driver->caps1);
		LOG("ATI CAPS 2: %d (0x%08X)\n", appi->ahi.info_driver->caps2, appi->ahi.info_driver->caps2);

		result = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, AHIFMT_16BPP_565,
			&size, &align, AHIFLAG_INTMEMORY);
		LOG("ATI Internal Memory Largest Block: result=%d, size=%d, size=%d KiB, align=%d\n",
			result, size, size / 1024, align);

		result = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, AHIFMT_16BPP_565,
			&size, &align, AHIFLAG_EXTMEMORY);
		LOG("ATI External Memory Largest Block: result=%d, size=%d, size=%d KiB, align=%d\n",
			result, size, size / 1024, align);

		LOG("ATI Display Mode: size=%dx%d, pixel_format=%d, frequency=%d, rotation=%d, mirror=%d\n",
			display_mode.size.x, display_mode.size.y,
			display_mode.pixel_format, display_mode.frequency, display_mode.rotation, display_mode.mirror);

		LOG("ATI Surface Info: width=%d, height=%d, pixFormat=%d, byteSize=%d, byteSize=%d KiB\n",
			appi->ahi.info_surface.width, appi->ahi.info_surface.height, appi->ahi.info_surface.pixFormat,
			appi->ahi.info_surface.byteSize, appi->ahi.info_surface.byteSize / 1024);
		LOG("ATI Surface Info: offset=%d, stride=%d, numPlanes=%d\n",
			appi->ahi.info_surface.offset, appi->ahi.info_surface.stride, appi->ahi.info_surface.numPlanes);
	}

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
	appi->ahi.bitmap.stride = appi->bmp_width * 2; /* (width * bpp) */
	appi->ahi.bitmap.format = AHIFMT_16BPP_565;
#if defined(UIS_HEAP)
	appi->ahi.bitmap.image = uisAllocateMemory(appi->bmp_width * appi->bmp_height * 2, &result);
#else
	appi->ahi.bitmap.image = suAllocMem(appi->bmp_width * appi->bmp_height * 2, &result);
#endif
	if (result != RESULT_OK) {
		LOG("%s\n", "Error: Cannot allocate screen buffer memory.");
		return RESULT_FAIL;
	}
	appi->ahi.rect_bitmap.x1 = 0;
	appi->ahi.rect_bitmap.y1 = 0;
	appi->ahi.rect_bitmap.x2 = 0 + appi->bmp_width;
	appi->ahi.rect_bitmap.y2 = 0 + appi->bmp_height;

#if defined(ROT_90)
	appi->ahi.rect_draw.x1 = 0;
	appi->ahi.rect_draw.y1 = 0;
	appi->ahi.rect_draw.x2 = 0 + appi->bmp_width;
	appi->ahi.rect_draw.y2 = 0 + appi->bmp_height;
#elif defined(ROT_0)
	appi->ahi.rect_draw.x1 = appi->width / 2 - appi->bmp_width / 2;
	appi->ahi.rect_draw.y1 = appi->height / 2 - appi->bmp_height / 2;
	appi->ahi.rect_draw.x2 = (appi->width / 2 - appi->bmp_width / 2) + appi->bmp_width;
	appi->ahi.rect_draw.y2 = (appi->height / 2 - appi->bmp_height / 2) + appi->bmp_height;

	status |= AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);

	status |= AhiDrawBrushFgColorSet(appi->ahi.context, ATI_565RGB(0x88, 0x88, 0x88));
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

	if (app_instance->p_bitmap) {
		LOG("%s\n", "Free: ATI Bitmap memory.");
#if defined(UIS_HEAP)
		uisFreeMemory(app_instance->p_bitmap);
#else
		suFreeMem(app_instance->p_bitmap);
#endif
		app_instance->p_bitmap = NULL;
	}

	status |= AhiDevClose(app_instance->ahi.context);
	if (app_instance->ahi.info_driver) {
		LOG("%s\n", "Free: ATI Driver Info memory.");
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
		&appi->ahi.rect_draw, &appi->ahi.point_bitmap, &appi->ahi.bitmap, NULL, 0);
	AhiDispWaitVBlank(appi->ahi.context, 0);
#elif defined(ROT_90)
	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.draw, 0);
	AhiDrawBitmapBlt(appi->ahi.context,
		&appi->ahi.rect_bitmap, &appi->ahi.point_bitmap, &appi->ahi.bitmap, NULL, 0);

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
#elif defined(EM1) || defined(EM2)
static UINT32 Nvidia_Driver_Start(APPLICATION_T *app) {
	INT32 result;
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	GRAPHIC_POINT_T point;

	result = RESULT_OK;
	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	app_instance->gfsdk.fb0_rect.x = 0;
	app_instance->gfsdk.fb0_rect.y = 0;

	UIS_CanvasGetDisplaySize(&point);
	app_instance->gfsdk.fb0_rect.w = point.x + 1;
	app_instance->gfsdk.fb0_rect.h = point.y + 1;
	app_instance->gfsdk.fb0 = uisAllocateMemory(
		app_instance->gfsdk.fb0_rect.w * app_instance->gfsdk.fb0_rect.h * 2, &result
	);
	if (result != RESULT_OK) {
		LOG("Cannot allocate '%d' bytes of memory for fill screen!\n",
			app_instance->gfsdk.fb0_rect.w * app_instance->gfsdk.fb0_rect.h * 2);
		status = RESULT_FAIL;
	}
	memclr(app_instance->gfsdk.fb0, app_instance->gfsdk.fb0_rect.w * app_instance->gfsdk.fb0_rect.h * 2);
	Nvidia_Driver_Flush(app);
	uisFreeMemory(app_instance->gfsdk.fb0);

	app_instance->gfsdk.fb0_rect.w = VIEWPORT_WIDTH;
	app_instance->gfsdk.fb0_rect.h = VIEWPORT_HEIGHT;

	app_instance->gfsdk.fb0 = uisAllocateMemory(VIEWPORT_WIDTH * VIEWPORT_HEIGHT * 2, &result);
	if (result != RESULT_OK) {
		LOG("Cannot allocate '%d' bytes of memory for screen!\n", VIEWPORT_WIDTH * VIEWPORT_HEIGHT * 2);
		status = RESULT_FAIL;
	}

	return status;
}

static UINT32 Nvidia_Driver_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	uisFreeMemory(app_instance->gfsdk.fb0);

	return status;
}

static UINT32 Nvidia_Driver_Flush(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	GFGxCopyColorBitmap(
		app_instance->gfsdk.gxHandle,
		app_instance->gfsdk.fb0_rect.x, app_instance->gfsdk.fb0_rect.y,
		app_instance->gfsdk.fb0_rect.w, app_instance->gfsdk.fb0_rect.h,
		app_instance->gfsdk.fb0_rect.x, app_instance->gfsdk.fb0_rect.y,
		app_instance->gfsdk.fb0_rect.w * 2,
		app_instance->gfsdk.fb0
	);

	return status;
}
#endif

static entity_t* box;

texture_t *textures = NULL;
unsigned short (*lua)[LUA_HEIGHT] = NULL;
int *sintable = NULL;

static void behaviour(entity_t* const e)
{
  if (g_keyboard[E_KEY_LEFT])
  {
    e->tt -= (12 << 16);
    e->r -= 800000;
  }
  if (g_keyboard[E_KEY_RIGHT])
  {
    e->tt += (12 << 16);
    e->r += 800000;
  }

  if (g_keyboard[E_KEY_UP])
  {
    e->xx += fixsin16(e->t >> 16) >> 5;
    e->yy += fixcos16(e->t >> 16) >> 5;
  }
  if (g_keyboard[E_KEY_DOWN])
  {
    e->xx -= fixsin16(e->t >> 16) >> 5;
    e->yy -= fixcos16(e->t >> 16) >> 5;
  }
  e->xx -= ((e->xx + (e->xx < 0 ? -15 : 15)) >> 4);
  e->yy -= ((e->yy + (e->yy < 0 ? -15 : 15)) >> 4);
  e->tt -= ((e->tt + (e->tt < 0 ? - 3 :  3)) >> 2);

  e->r  -= ((e->r  + (e->r  < 0 ? -7 : 7)) >> 3);

  e->x += e->xx; e->y += e->yy; e->z += e->zz;
  e->r += e->rr; e->t += e->tt; e->p += e->pp;

  entity_to_world_collision(e, 0x8000);
}

#if defined(CELLS_ALLOCA)
static cell_t **cells_array_p;
#endif

static void world_create(APPLICATION_T *app, world_t* world)
{
  APP_INSTANCE_T *appi;

  appi = (APP_INSTANCE_T *) app;

  world->screen = NULL;
  world->buffer = (viewport_t*) appi->p_bitmap;
#if defined(CELLS_ALLOCA)
  {
    UINT32 i;
    world->cells = suAllocMem(MAP_SIZE * sizeof(cell_t *), NULL);
    for (i = 0; i < MAP_SIZE; i++) {
      world->cells[i] = suAllocMem(MAP_SIZE * sizeof(cell_t), NULL);
    }
    cells_array_p = world->cells;
  }
#endif

  camera = entity_create(0, 0, 0);

  camera->x = MAP_SIZE << 15;
  camera->y = MAP_SIZE << 15;
  camera->z = 3 << 15;
  camera->p = 100 << 16;
}

static UINT32 GFX_Draw_Start(APPLICATION_T *app) {
	int x, y;
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

#if defined(EP1) || defined(EP2)
	appi->p_bitmap = (UINT8 *) appi->ahi.bitmap.image;
#elif defined(EM1) || defined(EM2)
	appi->p_bitmap = (UINT8 *) appi->gfsdk.fb0;
#endif

	InitResourses();

	world_create(app, &world);
	box = entity_create(33 << 16, 35 << 16, 1 << 16);
	entity_create(33 << 16, 33 << 16, 1 << 16);

	for (y = 0; y < MAP_SIZE; y++)
	{
	  for (x = 0; x < MAP_SIZE; x++)
	  {
		world.cells[y][x].l   = 0;
		world.cells[y][x].bot = 0;
		world.cells[y][x].top = 4;
		world.cells[y][x].wtx = 0;
		world.cells[y][x].btx = 2;
		world.cells[y][x].ttx = 1;

		if (x == 0 || y == 0 || x == (MAP_SIZE - 1) || y == (MAP_SIZE - 1))
		{
		  world.cells[y][x].top = 0;
		  world.cells[y][x].bot = 0;
		}
	  }
	}

	for (x = 1; x < MAP_SIZE - 1; x++)
	{
	  for (y = 1; y < MAP_SIZE - 1; y += 10)
	  {
		if (rand() % 10)
		{
		  world.cells[y][x].top = 0;
		  world.cells[y][x].bot = 0;
		  world.cells[x][y].top = 0;
		  world.cells[x][y].bot = 0;

		  world.cells[y][x].wtx = 0;
		  world.cells[x][y].wtx = 0;
		}
		else
		{
		  world.cells[y][x].top--;
		}
	  }
	}
	for (x = 0; x < 400; x++)
	{
	  cell_t* cell = &world.cells[rand() % MAP_SIZE][rand() % MAP_SIZE];

	  cell->bot += 2;
	  cell->wtx = 3;
	  cell->btx = 3;
	}

	for (y = 0; y < MAP_SIZE; y += 1)
	{
	  for (x = 0; x < MAP_SIZE; x += 1)
	  {
		world.cells[y][x].l = 0;
	  }
	}

	for (y = 1; y < MAP_SIZE; y += 5)
	{
	  for (x = 1; x < MAP_SIZE; x += 5)
	  {
		draw_light((x << 16) + 0x8000, (y << 16) + 0x8000, 900);
		world.cells[y][x].ttx = 5;
		world.cells[y][x].btx = 6;
	  }
	}

	return RESULT_OK;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	FreeResourses();

#if defined(CELLS_ALLOCA)
	{
		UINT32 i;
		for (i = 0; i < MAP_SIZE; i++) {
			suFreeMem(cells_array_p[i]);
		}
		suFreeMem(cells_array_p);
	}
#endif

	return RESULT_OK;
}

static UINT32 GFX_Draw_Step(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	behaviour(camera);
	box->t += 50 << 16;
	box->r += 40 << 16;
	box->p += 30 << 16;
	draw_world();

	return RESULT_OK;
}

static UINT32 InitResourses(void) {
	INT32 status;
	UINT32 readen;
	FILE_HANDLE_T file_handle;

	readen = 0;
	status = RESULT_OK;

	LOG("Trying to allocate texture %d bytes.\n", TEXTURE_WIDTH * TEXTURE_HEIGHT * TEXTURE_MAX);
#if defined(UIS_HEAP)
	textures = (texture_t *) uisAllocateMemory(TEXTURE_WIDTH * TEXTURE_HEIGHT * TEXTURE_MAX, &status);
#else
	textures = (texture_t *) suAllocMem(TEXTURE_WIDTH * TEXTURE_HEIGHT * TEXTURE_MAX, &status);
#endif
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.tex");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(textures, TEXTURE_WIDTH * TEXTURE_HEIGHT * TEXTURE_MAX, 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (status != RESULT_OK) {
		LOG("%s\n", "Error: Cannot allocate textures array.");
	}
	if (readen == 0) {
		LOG("%s\n", "Error: Cannot read 'Yeti3D.tex' file.");
	}

	LOG("Trying to allocate LUA %d bytes.\n", sizeof(lua_t));
#if defined(UIS_HEAP)
	lua = uisAllocateMemory(sizeof(lua_t), &status);
#else
	lua = suAllocMem(sizeof(lua_t), &status);
#endif
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.lua");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(lua, sizeof(lua_t), 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (status != RESULT_OK) {
		LOG("%s\n", "Error: Cannot allocate LUA array!");
	}
	if (readen == 0) {
		LOG("%s\n", "Error: Cannot read 'Yeti3D.lua' file.");
	}

	LOG("Trying to allocate sintable %d bytes.\n", SINTABLE_SIZE * SINTABLE_MAX);
#if defined(UIS_HEAP)
	sintable = (int *) uisAllocateMemory(SINTABLE_SIZE * SINTABLE_MAX, &status);
#else
	sintable = (int *) suAllocMem(SINTABLE_SIZE * SINTABLE_MAX, &status);
#endif
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.sin");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(sintable, SINTABLE_SIZE * SINTABLE_MAX, 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (status != RESULT_OK) {
		LOG("%s\n", "Error: Cannot allocate sintable array!");
	}
	if (readen == 0) {
		LOG("%s\n", "Error: Cannot read 'Yeti3D.sin' file.");
	}

	return RESULT_OK;
}

static void FreeResourses(void) {
	if (textures) {
		LOG("%s\n", "Free: Textures memory.");
#if defined(UIS_HEAP)
		uisFreeMemory(textures);
#else
		suFreeMem(textures);
#endif
		textures = NULL;
	}
	if (lua) {
		LOG("%s\n", "Free: LUA memory.");
#if defined(UIS_HEAP)
		uisFreeMemory(lua);
#else
		suFreeMem(lua);
#endif
		lua = NULL;
	}
	if (sintable) {
		LOG("%s\n", "Free: sintable memory.");
#if defined(UIS_HEAP)
		uisFreeMemory(sintable);
#else
		suFreeMem(sintable);
#endif
		sintable = NULL;
	}
}
