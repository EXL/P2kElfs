/*
 * About:
 *   The "Yeti3D" is a port of heavy optimized 3D engine demo from GBA to Motorola P2K ELF application.
 *
 * Author:
 *   Derek J. Evans, EXL
 *
 * License:
 *   GPLv2
 *
 * Additional information:
 *   https://web.archive.org/web/20031204145215/http://www.theteahouse.com.au:80/gba/index.html
 *   https://sourceforge.net/projects/yeti3dpro/
 *   https://forum.motofan.ru/index.php?s=&showtopic=170514&view=findpost&p=1459600
 *
 * Application type:
 *   GUI + ATI + Java Heap + Videomode + Tasks
 */

#include "yeti.h"
#include "game.h"

#include <loader.h>
#include <ati.h>
#include <apps.h>
#include <dl.h>
#include <dal.h>
#include <filesystem.h>
#include <uis.h>
#include <mem.h>
#include <time_date.h>
#include <utilities.h>
#include <tasks.h>

#define TIMER_FAST_TRIGGER_MS             (1)
#define TASK_STACK_SIZE                   (0x5000)
#define TASK_PRIORITY                     (0x18)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	GLOBAL_STATE_EXIT,
	GLOBAL_STATE_INIT,
	GLOBAL_STATE_MAIN,
	GLOBAL_STATE_SUSPEND
} GLOBAL_STATE_T;

typedef struct {
	AHIDRVINFO_T *info_driver;
	AHIDEVCONTEXT_T context;
	AHISURFACE_T screen;
	AHISURFACE_T draw;
	AHISURFINFO_T info_surface_screen;
	AHISURFINFO_T info_surface_draw;
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

	UINT8 *p_bitmap;

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
static UINT32 ApplicationHandleLoseFocus(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ApplicationHandleGainFocus(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static void ApplicationTaskStart(void *args);
static BOOL ApplicationTaskInit(void);
static void ApplicationTaskLoop(void);
static BOOL ApplicationTaskFree(void);

static UINT32 CheckKeyboard(APPLICATION_T *app);
static UINT32 ProcessKeyboard(APPLICATION_T *app, UINT32 key, BOOL pressed);

static void FPS_Meter(void);

static UINT32 ATI_Driver_Log(APPLICATION_T *app);
static UINT32 ATI_Driver_Log_Memory(APPLICATION_T *app, AHIPIXFMT_T pixel_format);
static UINT32 ATI_Driver_Start(APPLICATION_T *app);
static UINT32 ATI_Driver_Stop(APPLICATION_T *app);
static UINT32 ATI_Driver_Flush(APPLICATION_T *app);

static UINT32 GFX_Draw_Start(APPLICATION_T *app);
static UINT32 GFX_Draw_Stop(APPLICATION_T *app);
static UINT32 GFX_Draw_Step(APPLICATION_T *app);

static UINT32 InitResourses(void);
static void FreeResourses(void);

static const char g_app_name[APP_NAME_LEN] = "FireEffect";

#if defined(EP2)
static ldrElf g_app_elf;
#endif

typedef enum {
	E_KEY_UP,
	E_KEY_DOWN,
	E_KEY_LEFT,
	E_KEY_RIGHT,
	E_KEY_FIRE,
	E_KEY_JUMP,
	E_KEY_LOOK_UP,
	E_KEY_LOOK_DOWN,
	E_KEY_MAX
} GAME_KEYS_T;
static BOOL g_keyboard[E_KEY_MAX] = { FALSE };

static WCHAR g_res_file_path[FS_MAX_URI_NAME_LENGTH];

static GLOBAL_STATE_T g_state = GLOBAL_STATE_INIT;
static SEMAPHORE_HANDLE_T g_semaphore;
static APPLICATION_T *g_app = NULL;

static EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ EV_LOSE_FOCUS, ApplicationHandleLoseFocus },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ EV_GAIN_FOCUS, ApplicationHandleGainFocus },
	{ EV_DISPLAY_ACTIVE, ApplicationHandleGainFocus },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DISPLAY_NO_ACTIVE, ApplicationHandleLoseFocus },
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
		cprint("Spout: Error! Application has already been loaded!\n");
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
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 0, 1, AFW_APP_CENTRICITY_PRIMARY, AFW_FOCUS, AFW_POSITION_TOP);

		app_instance->ahi.info_driver = NULL;
		app_instance->bmp_width = YETI_VIEWPORT_WIDTH;
		app_instance->bmp_height = YETI_VIEWPORT_HEIGHT;
		app_instance->p_bitmap = NULL;
		app_instance->timer_handle = 0;
		app_instance->keys.pressed = 0;
		app_instance->keys.released = 0;

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

	APP_ConsumeEv(ev_st, app);

	DeleteDialog(app);

	DL_AudSetVolumeSetting(PHONE, app_instance->keyboard_volume_level);

	status |= GFX_Draw_Stop(app);
	status |= ATI_Driver_Stop(app);
	status |= APP_Exit(ev_st, app, 0);

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#endif

	return status;
}

static UINT32 ApplicationHandleLoseFocus(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	LOG("%s\n", "ApplicationHandleLoseFocus: Enter!");

	if (g_state != GLOBAL_STATE_MAIN) {
		return status;
	}

	g_state = GLOBAL_STATE_SUSPEND;

	LOG("%s\n", "ApplicationHandleLoseFocus: Acquiring semaphore!");

	suAcquireSem(g_semaphore, SEMAPHORE_WAIT_FOREVER, NULL);

	LOG("%s\n", "ApplicationHandleLoseFocus: Acquire passed!");

	status |= APP_UtilChangeState(APP_STATE_INIT, ev_st, app);

	return status;
}

static UINT32 ApplicationHandleGainFocus(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	LOG("%s\n", "ApplicationHandleGainFocus: Enter!");

	if (g_state == GLOBAL_STATE_SUSPEND) {
		status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);
	}

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	APP_INSTANCE_T *app_instance;
	DRAWING_BUFFER_T buffer_draw;

	app_instance = (APP_INSTANCE_T *) app;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;
	dialog = DialogType_None;

	switch (app_state) {
		case APP_STATE_MAIN:
			buffer_draw.buf = NULL;
			buffer_draw.w = app_instance->width;
			buffer_draw.h = app_instance->height;
			dialog = UIS_CreateColorCanvas(&port, &buffer_draw, TRUE);
			break;
		default:
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	g_state = GLOBAL_STATE_MAIN;

	GFX_Draw_Start(app);
	g_app = app;
	suCreateTask(ApplicationTaskStart, TASK_STACK_SIZE, TASK_PRIORITY);

	return RESULT_OK;
}

static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state) {
	if (state == EXIT_STATE_EXIT) {
		if (g_state == GLOBAL_STATE_MAIN) {
			g_state = GLOBAL_STATE_SUSPEND;

			LOG("%s\n", "HandleStateExit: Acquiring semaphore!");

			suAcquireSem(g_semaphore, SEMAPHORE_WAIT_FOREVER, NULL);

			LOG("%s\n", "HandleStateExit: Acquire passed!");
		}
		if (app->state != APP_STATE_MAIN) {
			DeleteDialog(app);
		}
		g_app = NULL;
		return RESULT_OK;
	} else {
		app->exit_status = TRUE;
	}

	return RESULT_OK;
}

static UINT32 DeleteDialog(APPLICATION_T *app) {
	if (app->dialog != DialogType_None) {
		UIS_Delete(app->dialog);
		app->dialog = DialogType_None;
		return RESULT_OK;
	}
	return RESULT_FAIL;
}

static void ApplicationTaskStart(void *args) {
	if (ApplicationTaskInit()) {
		ApplicationTaskLoop();
	}
	ApplicationTaskFree();

	/* HACK: Inject keypress for exit event. */
	DL_KeyInjectKeyPress(KEY_RED, KEY_PRESS, KEY_PORTABLE_ID);
	DL_KeyInjectKeyPress(KEY_RED, KEY_RELEASE, KEY_PORTABLE_ID);
}

static BOOL ApplicationTaskInit(void) {
	INT32 error;

	error = RESULT_OK;

	LOG("%s\n", "ApplicationTaskInit: Creating semaphore!");
	g_semaphore = suCreateBSem(SEMAPHORE_LOCKED, &error);
	LOG("ApplicationTaskInit: g_semaphore=0x%08X, err=%d\n", g_semaphore, error);

	DL_KeyKjavaGetKeyState();

	randomize();

	return (error == RESULT_OK);
}

static void ApplicationTaskLoop(void) {
	while ((g_state != GLOBAL_STATE_EXIT) && (g_state != GLOBAL_STATE_SUSPEND)) {
		FPS_Meter();
		CheckKeyboard(g_app);
		GFX_Draw_Step(g_app);
		ATI_Driver_Flush(g_app);
		suSleep(TIMER_FAST_TRIGGER_MS, NULL);
	}
}

static BOOL ApplicationTaskFree(void) {
	LOG("%s\n", "ApplicationTaskFree: Enter!");

	if (g_state == GLOBAL_STATE_EXIT) {
		g_app->exit_status = TRUE;
	}

	LOG("%s\n", "ApplicationTaskFree: Release Semaphore!");
	suReleaseSem(g_semaphore, NULL);

	LOG("%s\n", "ApplicationTaskFree: Delete Semaphore!");
	suDeleteSem(g_semaphore, NULL);

	return TRUE;
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

static UINT32 ProcessKeyboard(APPLICATION_T *app, UINT32 key, BOOL pressed) {
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
			g_state = GLOBAL_STATE_EXIT;
			break;
		case MULTIKEY_1:
			g_keyboard[E_KEY_LOOK_DOWN] = pressed;
			break;
		case KK_2:
		case KK_UP:
			g_keyboard[E_KEY_UP] = pressed;
			break;
		case MULTIKEY_3:
			g_keyboard[E_KEY_LOOK_UP] = pressed;
			break;
		case KK_4:
		case KK_LEFT:
			g_keyboard[E_KEY_LEFT] = pressed;
			break;
		case MULTIKEY_5:
		case MULTIKEY_JOY_OK:
			g_keyboard[E_KEY_FIRE] = pressed;
			break;
		case KK_6:
		case KK_RIGHT:
			g_keyboard[E_KEY_RIGHT] = pressed;
			break;
		case MULTIKEY_7:
		case MULTIKEY_9:
			g_keyboard[E_KEY_JUMP] = pressed;
			break;
		case KK_8:
		case KK_DOWN:
			g_keyboard[E_KEY_DOWN] = pressed;
			break;
		case MULTIKEY_STAR:
			break;
		case MULTIKEY_POUND:
			break;
		case MULTIKEY_SOFT_RIGHT:
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

static UINT32 ATI_Driver_Log(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	LOG("%s\n", "ATI Driver Dump.");
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

	LOG("ATI Surface Screen Info: width=%d, height=%d, pixFormat=%d, byteSize=%d, byteSize=%d KiB\n",
		appi->ahi.info_surface_screen.width, appi->ahi.info_surface_screen.height,
		appi->ahi.info_surface_screen.pixFormat,
		appi->ahi.info_surface_screen.byteSize, appi->ahi.info_surface_screen.byteSize / 1024);
	LOG("ATI Surface Screen Info: offset=%d, stride=%d, numPlanes=%d\n",
		appi->ahi.info_surface_screen.offset,
		appi->ahi.info_surface_screen.stride,
		appi->ahi.info_surface_screen.numPlanes);

	LOG("ATI Surface Draw Info: width=%d, height=%d, pixFormat=%d, byteSize=%d, byteSize=%d KiB\n",
		appi->ahi.info_surface_draw.width, appi->ahi.info_surface_draw.height,
		appi->ahi.info_surface_draw.pixFormat,
		appi->ahi.info_surface_draw.byteSize, appi->ahi.info_surface_draw.byteSize / 1024);
	LOG("ATI Surface Draw Info: offset=%d, stride=%d, numPlanes=%d\n",
		appi->ahi.info_surface_draw.offset,
		appi->ahi.info_surface_draw.stride,
		appi->ahi.info_surface_draw.numPlanes);

	return RESULT_OK;
}

static UINT32 ATI_Driver_Log_Memory(APPLICATION_T *app, AHIPIXFMT_T pixel_format) {
	enum {
		INTERNAL_MEMORY,
		EXTERNAL_MEMORY,
		SYSTEM_MEMORY,
		MEMORY_MAX
	};

	APP_INSTANCE_T *appi;
	UINT32 status[MEMORY_MAX] = { 0 };
	UINT32 sizes[MEMORY_MAX] = { 0 };
	UINT32 alignment[MEMORY_MAX] = { 0 };

	appi = (APP_INSTANCE_T *) app;

	status[INTERNAL_MEMORY] = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, pixel_format,
		&sizes[INTERNAL_MEMORY], &alignment[INTERNAL_MEMORY], AHIFLAG_INTMEMORY);
	status[EXTERNAL_MEMORY] = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, pixel_format,
		&sizes[EXTERNAL_MEMORY], &alignment[EXTERNAL_MEMORY], AHIFLAG_EXTMEMORY);
	status[SYSTEM_MEMORY] = AhiSurfGetLargestFreeBlockSize(appi->ahi.context, pixel_format,
		&sizes[SYSTEM_MEMORY], &alignment[SYSTEM_MEMORY], AHIFLAG_SYSMEMORY);

	LOG("%s\n", "ATI Memory Dump.");
	LOG("\tATI Internal Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[INTERNAL_MEMORY], pixel_format, sizes[INTERNAL_MEMORY], sizes[INTERNAL_MEMORY] / 1024, alignment[INTERNAL_MEMORY]);
	LOG("\tATI External Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[EXTERNAL_MEMORY], pixel_format, sizes[EXTERNAL_MEMORY], sizes[EXTERNAL_MEMORY] / 1024, alignment[EXTERNAL_MEMORY]);
	LOG("\tATI System Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[SYSTEM_MEMORY], pixel_format, sizes[SYSTEM_MEMORY], sizes[SYSTEM_MEMORY] / 1024, alignment[SYSTEM_MEMORY]);

	return status[INTERNAL_MEMORY] && status[EXTERNAL_MEMORY] && status[SYSTEM_MEMORY];
}

//#define NO_STRETCH
#if defined(NO_STRETCH)
static UINT32 Find_Surface_Addresses_In_RAM(APPLICATION_T *app, UINT32 start_address, UINT32 size_search_region) {
	APP_INSTANCE_T *appi;
	UINT32 surface_block_offset;
	UINT32 *start_ptr;

	appi = (APP_INSTANCE_T *) app;
	surface_block_offset = 0;
	start_ptr = (UINT32 *) start_address;

	while (surface_block_offset < size_search_region) {
		/*
		 * Find the block of:
		 * o-4: ...
		 * o  : curDrawSurf
		 * o+4: curDispSurf
		 * o+8: ...
		*/

		if(*start_ptr == (UINT32) (appi->ahi.screen)) {
			if(*(start_ptr + 1) == (UINT32) (appi->ahi.draw)) {
				break;
			}
		}

		start_ptr++;
		surface_block_offset += 4;
	}
	if (surface_block_offset >= size_search_region) {
		/* Not Found? */
		surface_block_offset = 0;
	}

	return surface_block_offset;
}

static UINT32 ATI_Driver_Set_Display_Mode(APPLICATION_T *app, AHIROTATE_T mode) {
	UINT32 status;
	APP_INSTANCE_T *appi;
	AHIPOINT_T size_landscape;
	AHIPOINT_T size_portrait;
	AHIDEVCONTEXT_T device_context;
	UINT32 *surface_disp_addr;
	UINT32 *surface_draw_addr;
	AHIDISPMODE_T display_mode;
	UINT32 start_addr;
	UINT32 search_region;
	UINT32 surface_block_offset;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	device_context = DAL_GetDeviceContext(DISPLAY_MAIN);

//	start_addr = 0x12300000;
//	search_region = 0x01000000;
	start_addr = (UINT32) Class_dal;
	search_region = 0x00000100;
	surface_block_offset = Find_Surface_Addresses_In_RAM(app, start_addr, search_region);

	status |= AhiDispModeGet(appi->ahi.context, &display_mode);

	size_landscape.x = display_mode.size.y;
	size_landscape.y = display_mode.size.x;
	size_portrait.x = display_mode.size.x;
	size_portrait.y = display_mode.size.y;

	surface_disp_addr = (UINT32 *) (start_addr + surface_block_offset + 0x00);
	surface_draw_addr = (UINT32 *) (start_addr + surface_block_offset + 0x04);

	LOG("ATI Display Mode Dumps 1:\n\t"
		"Class_dal=0x%08X 0x%08X 0x%08X\n\t"
		"start_addr=0x%08X 0x%08X 0x%08X\n\t"
		"search_region=0x%08X\n\t"
		"surface_disp_addr=0x%08X\n\t"
		"surface_draw_addr=0x%08X\n\t"
		"surface_block_offset=0x%08X\n",
		*Class_dal, &Class_dal, Class_dal, *((UINT32 *)start_addr), &start_addr, start_addr,
		search_region, surface_disp_addr, surface_draw_addr, surface_block_offset, surface_block_offset);

	LOG("ATI Display Mode Dumps 2:\n\t"
		"display_source_buffer=0x%08X 0x%08X\n\t"
		"appi->ahi.screen=0x%08X 0x%08X\n\t"
		"appi->ahi.draw=0x%08X 0x%08X\n",
		display_source_buffer, &display_source_buffer,
		(UINT32) (appi->ahi.screen), &appi->ahi.screen,
		(UINT32) (appi->ahi.draw), &appi->ahi.draw);

	if (mode == AHIROT_90 || mode == AHIROT_270) {
		status |= AhiSurfFree(device_context, (AHISURFACE_T) (*surface_disp_addr));
		status |= AhiSurfFree(device_context, (AHISURFACE_T) (*surface_draw_addr));

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);

		status = AhiSurfAlloc(appi->ahi.context,
			&appi->ahi.screen, &size_landscape, AHIFMT_16BPP_565, AHIFLAG_INTMEMORY);
		if (status != RESULT_OK) {
			LOG("ATI_Driver_Set_Display_Mode: Cannot allocate display (landscape) surface, status = %d\n", status);
			return RESULT_FAIL;
		}
		status = AhiSurfAlloc(appi->ahi.context,
			&appi->ahi.draw, &size_landscape, AHIFMT_16BPP_565, 0);
		if (status != RESULT_OK) {
			LOG("ATI_Driver_Set_Display_Mode: Cannot allocate drawing (landscape) surface, status = %d\n", status);
			status |= AhiSurfFree(device_context, appi->ahi.screen);
			appi->ahi.screen = 0;
			return RESULT_FAIL;
		}

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);
	} else {
		if (appi->ahi.screen) {
			status |= AhiSurfFree(appi->ahi.context, appi->ahi.screen);
		}
		if (appi->ahi.draw) {
			status |= AhiSurfFree(appi->ahi.context, appi->ahi.draw);
		}

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);

		status = AhiSurfAlloc(device_context,
			&appi->ahi.screen, &size_portrait, AHIFMT_16BPP_565, AHIFLAG_INTMEMORY);
		if (status != RESULT_OK) {
			LOG("ATI_Driver_Set_Display_Mode: Cannot allocate display (portrait) surface, status = %d\n", status);
			return RESULT_FAIL;
		}
		status = AhiSurfAlloc(device_context,
			&appi->ahi.draw, &size_portrait, AHIFMT_16BPP_565, 0);
		if (status != RESULT_OK) {
			LOG("ATI_Driver_Set_Display_Mode: Cannot allocate drawing (portrait) surface, status = %d\n", status);
			status |= AhiSurfFree(device_context, appi->ahi.screen);
			appi->ahi.screen = 0;
			return RESULT_FAIL;
		}

		*surface_disp_addr = (UINT32) appi->ahi.screen;
		*surface_draw_addr = (UINT32) appi->ahi.draw;

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);
	}

	status |= AhiDispModeGet(appi->ahi.context, &display_mode);
	status |= AhiDispState(appi->ahi.context, DISPLAY_OFF, 0);
	display_mode.rotation = mode;
	status = AhiDispModeSet(appi->ahi.context, &display_mode, 0);
	if (status != RESULT_OK) {
		LOG("ATI_Driver_Set_Display_Mode: Cannot change display mode, status = %d\n", status);
		return RESULT_FAIL;
	}
	status |= AhiDispSurfSet(appi->ahi.context, appi->ahi.screen, 0);
	status |= AhiDispState(appi->ahi.context, DISPLAY_ON, 0);

	return status;
}
#endif

static UINT32 ATI_Driver_Start(APPLICATION_T *app) {
	UINT32 status;
	INT32 result;
	APP_INSTANCE_T *appi;
	AHIDEVICE_T ahi_device;
	AHIDISPMODE_T display_mode;

	status = RESULT_OK;
	result = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	LOG("%s\n", "ATI Driver Start Initialization.");

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

	/*
	 * Motorola SLVR L6: 128x160
	 * Motorola ROKR E1: 176x220
	 */
	appi->is_CSTN_display =
			(display_mode.size.x < DISPLAY_WIDTH) ||
			(display_mode.size.y < DISPLAY_HEIGHT);

#if defined(NO_STRETCH)
#if defined(FTR_L6)
	status |= ATI_Driver_Set_Display_Mode(app, AHIROT_90);
#else
	status |= ATI_Driver_Set_Display_Mode(app, (appi->is_CSTN_display) ? AHIROT_270 : AHIROT_90);
#endif
#endif

	status |= AhiDrawClipDstSet(appi->ahi.context, NULL);
	status |= AhiDrawClipSrcSet(appi->ahi.context, NULL);

	status |= AhiSurfInfo(appi->ahi.context, appi->ahi.screen, &appi->ahi.info_surface_screen);
	status |= AhiSurfInfo(appi->ahi.context, appi->ahi.draw, &appi->ahi.info_surface_draw);

	appi->width = appi->ahi.info_surface_screen.width;
	appi->height = appi->ahi.info_surface_screen.height;

	appi->ahi.update_params.size = sizeof(AHIUPDATEPARAMS_T);
	appi->ahi.update_params.sync = FALSE;
	appi->ahi.update_params.rect.x1 = 0;
	appi->ahi.update_params.rect.y1 = 0;
	appi->ahi.update_params.rect.x2 = 0 + display_mode.size.x;
	appi->ahi.update_params.rect.y2 = 0 + display_mode.size.y;
	appi->ahi.point_bitmap.x = 0;
	appi->ahi.point_bitmap.y = 0;

	appi->ahi.bitmap.width = appi->bmp_width;
	appi->ahi.bitmap.height = appi->bmp_height;
	appi->ahi.bitmap.stride = appi->bmp_width * 2; /* (width * bpp) */
	appi->ahi.bitmap.format = AHIFMT_16BPP_565;
	appi->ahi.bitmap.image = suAllocMem(appi->bmp_width * appi->bmp_height * 2, &result);
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
	appi->ahi.rect_draw.y1 = appi->bmp_height + 1;
	appi->ahi.rect_draw.x2 = 0 + appi->bmp_height;
	appi->ahi.rect_draw.y2 = appi->bmp_height + 1 + appi->bmp_width;
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

	status |= ATI_Driver_Log(app);
	status |= ATI_Driver_Log_Memory(app, AHIFMT_16BPP_565);

	return status;
}

static UINT32 ATI_Driver_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

#if defined(NO_STRETCH)
#if defined(FTR_L6)
	status |= ATI_Driver_Set_Display_Mode(app, AHIROT_0);
#else
	status |= ATI_Driver_Set_Display_Mode(app, (app_instance->is_CSTN_display) ? AHIROT_180 : AHIROT_0);
#endif
#endif

	if (app_instance->p_bitmap) {
		LOG("%s\n", "Free: ATI Bitmap memory.");
		suFreeMem(app_instance->p_bitmap);
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

yeti_t *yeti = NULL;

static texture_t *res_tex = NULL;
static rgb555_t (*res_lua)[256] = NULL;
int *reciprocal = NULL;
int *sintable = NULL;

rom_map_t *e1m1 = NULL;

#define SPRITE_0_SIZE 1430
#define SPRITE_BALL_1_SIZE 32772
u16 *spr_00 = NULL;
u16 *spr_01 = NULL;
u16 *spr_02 = NULL;
u16 *spr_03 = NULL;
u16 *spr_ball1 = NULL;

sprite_t sprites[YETI_SPRITE_MAX];

static void check_keys(void) {
	yeti->keyboard.a      = g_keyboard[E_KEY_FIRE];
	yeti->keyboard.b      = g_keyboard[E_KEY_JUMP];
//	yeti->keyboard.select = g_keyboard[E_KEY_EXIT];
	yeti->keyboard.right  = g_keyboard[E_KEY_RIGHT];
	yeti->keyboard.left   = g_keyboard[E_KEY_LEFT];
	yeti->keyboard.up     = g_keyboard[E_KEY_UP];
	yeti->keyboard.down   = g_keyboard[E_KEY_DOWN];
	yeti->keyboard.l      = g_keyboard[E_KEY_LOOK_UP];
	yeti->keyboard.r      = g_keyboard[E_KEY_LOOK_DOWN];
}

//#define MEMORY_MANUAL_ALLOCATION
#if defined(MEMORY_MANUAL_ALLOCATION)
#define MEMORY_ATTEMPTS (64)
#define MEMORY_END_BLOCK_SIZE (4096)
typedef struct {
	void *ptr;
	UINT32 size;
} MEMORY_BLOCK_ALLOCATED;
static MEMORY_BLOCK_ALLOCATED mem_blocks[MEMORY_ATTEMPTS];
static int mem_total_size;
static int mem_block_count;
static void Allocate_Memory_Blocks(int start_size) {
	INT32 status;
	int i, size, block_idx;

	status = RESULT_OK;
	mem_total_size = 0;
	mem_block_count = 0;
	block_idx = 0;
	size = start_size;

	for (i = 0; i < MEMORY_ATTEMPTS; ++i) {
		mem_blocks[i].ptr = NULL;
		mem_blocks[i].size = 0;
	}
	for (i = 0; i < MEMORY_ATTEMPTS; ++i) {
		mem_blocks[block_idx].ptr = suAllocMem(size, &status);
		if (status != RESULT_OK) {
			LOG("C=%d E=%d T=%d\n", i+1, size, mem_total_size);
			size /= 2;
			if (size < MEMORY_END_BLOCK_SIZE) {
				break;
			}
		} else {
			mem_total_size += size;
			mem_blocks[block_idx].size = size;
			LOG("C=%d A=%d T=%d P=0x%X\n", i+1, size, mem_total_size, mem_blocks[block_idx]);
			block_idx++;
		}
	}

	LOG("\n\n%s\n\n", "=== MEMORY BLOCKS STATISTIC TABLE ===");
	for (i = 0; i < block_idx; ++i) {
		LOG("Memory Block #%d: %d bytes, %d KiB, 0x%X\n",
			i+1, mem_blocks[i].size, mem_blocks[i].size / 1024, mem_blocks[i].ptr);
	}
	LOG("Total Memory: %d bytes, %d KiB.\n\n", mem_total_size, mem_total_size / 1024);

	mem_block_count = block_idx + 1;
}

static void Free_Memory_Blocks(void) {
	int i;

	for (i = 0; i < mem_block_count - 1; ++i) {
		if (mem_blocks[i].ptr) {
			LOG("Free Memory Block #%d: %d bytes, %d KiB, 0x%X\n",
				i+1, mem_blocks[i].size, mem_blocks[i].size / 1024, mem_blocks[i].ptr);
			suFreeMem(mem_blocks[i].ptr);
		}
	}
}
#endif

static UINT32 GFX_Draw_Start(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	appi->p_bitmap = (UINT8 *) appi->ahi.bitmap.image;

#if defined(MEMORY_MANUAL_ALLOCATION)
	Allocate_Memory_Blocks(131072);
	Free_Memory_Blocks();
#endif

	yeti = (yeti_t *) AmMemAllocPointer(sizeof(yeti_t));
	if (!yeti) {
		LOG("yeti: Error alloc %d bytes.\n", sizeof(yeti_t));
	} else {
		LOG("yeti: OK alloc %d bytes.\n", sizeof(yeti_t));
	}

	InitResourses();

	if (yeti) {
		yeti_init(
			yeti,
			NULL,
			(framebuffer_t *) appi->p_bitmap,
			res_tex,
			NULL,
			res_lua
		);
		game_init(yeti);
	}

	return RESULT_OK;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	FreeResourses();

	return RESULT_OK;
}

static UINT32 GFX_Draw_Step(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

	yeti_tick(yeti);
	yeti_draw(yeti);

	check_keys();

	return RESULT_OK;
}

static UINT32 InitResourses(void) {
	UINT32 readen;
	FILE_HANDLE_T file_handle;

	readen = 0;

	res_tex = (texture_t *) AmMemAllocPointer(sizeof(texture_t) * YETI_TEXTURE_MAX);
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.tex");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(res_tex, sizeof(texture_t) * YETI_TEXTURE_MAX, 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (!res_tex) {
		LOG("Yeti3D.tex: Error alloc %d bytes.\n", sizeof(texture_t) * YETI_TEXTURE_MAX);
	} else {
		LOG("Yeti3D.tex: OK alloc %d bytes.\n", sizeof(texture_t) * YETI_TEXTURE_MAX);
	}
	if (readen == 0) {
		LOG("%s\n", "Yeti3D.tex: Error reading file.");
	}

	res_lua = (rgb555_t (*)[256]) AmMemAllocPointer(sizeof(lua_t));
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.lua");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(res_lua, sizeof(lua_t), 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (!res_lua) {
		LOG("Yeti3D.lua: Error alloc %d bytes.\n", sizeof(lua_t));
	} else {
		LOG("Yeti3D.lua: OK alloc %d bytes.\n", sizeof(lua_t));
	}
	if (readen == 0) {
		LOG("%s\n", "Yeti3D.lua: Error reading file.");
	}

	reciprocal = (int *) AmMemAllocPointer(sizeof(int) * YETI_RECIPROCAL_MAX);
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.rec");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(reciprocal, sizeof(int) * YETI_RECIPROCAL_MAX, 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (!reciprocal) {
		LOG("Yeti3D.rec: Error alloc %d bytes.\n", sizeof(int) * YETI_RECIPROCAL_MAX);
	} else {
		LOG("Yeti3D.rec: OK alloc %d bytes.\n", sizeof(int) * YETI_RECIPROCAL_MAX);
	}
	if (readen == 0) {
		LOG("%s\n", "Yeti3D.rec: Error reading file.");
	}

	sintable = (int *) AmMemAllocPointer(sizeof(int) * YETI_SINTABLE_MAX);
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.sin");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(sintable, sizeof(int) * YETI_SINTABLE_MAX, 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (!sintable) {
		LOG("Yeti3D.sin: Error alloc %d bytes.\n", sizeof(int) * YETI_SINTABLE_MAX);
	} else {
		LOG("Yeti3D.sin: OK alloc %d bytes.\n", sizeof(int) * YETI_SINTABLE_MAX);
	}
	if (readen == 0) {
		LOG("%s\n", "Yeti3D.sin: Error reading file.");
	}

	e1m1 = (rom_map_t *) AmMemAllocPointer(sizeof(rom_map_t));
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.map");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(e1m1, sizeof(rom_map_t), 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (!e1m1) {
		LOG("Yeti3D.map: Error alloc %d bytes.\n", sizeof(rom_map_t));
	} else {
		LOG("Yeti3D.map: OK alloc %d bytes.\n", sizeof(rom_map_t));
	}
	if (readen == 0) {
		LOG("%s\n", "Yeti3D.map: Error reading file.");
	}

	spr_00 = (u16 *) AmMemAllocPointer(SPRITE_0_SIZE);
	if (!spr_00) {
		LOG("Yeti3D.spr:spr_00: Error alloc %d bytes.\n", SPRITE_0_SIZE);
	} else {
		LOG("Yeti3D.spr:spr_00: OK alloc %d bytes.\n", SPRITE_0_SIZE);
	}
	spr_01 = (u16 *) AmMemAllocPointer(SPRITE_0_SIZE);
	if (!spr_01) {
		LOG("Yeti3D.spr:spr_01: Error alloc %d bytes.\n", SPRITE_0_SIZE);
	} else {
		LOG("Yeti3D.spr:spr_01: OK alloc %d bytes.\n", SPRITE_0_SIZE);
	}
	spr_02 = (u16 *) AmMemAllocPointer(SPRITE_0_SIZE);
	if (!spr_02) {
		LOG("Yeti3D.spr:spr_02: Error alloc %d bytes.\n", SPRITE_0_SIZE);
	} else {
		LOG("Yeti3D.spr:spr_02: OK alloc %d bytes.\n", SPRITE_0_SIZE);
	}
	spr_03 = (u16 *) AmMemAllocPointer(SPRITE_0_SIZE);
	if (!spr_03) {
		LOG("Yeti3D.spr:spr_03: Error alloc %d bytes.\n", SPRITE_0_SIZE);
	} else {
		LOG("Yeti3D.spr:spr_03: OK alloc %d bytes.\n", SPRITE_0_SIZE);
	}
	spr_ball1 = (u16 *) AmMemAllocPointer(SPRITE_BALL_1_SIZE);
	if (!spr_ball1) {
		LOG("Yeti3D.spr:spr_ball1: Error alloc %d bytes.\n", SPRITE_BALL_1_SIZE);
	} else {
		LOG("Yeti3D.spr:spr_ball1: OK alloc %d bytes.\n", SPRITE_BALL_1_SIZE);
	}
	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"Yeti3D.spr");
	file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
	DL_FsReadFile(spr_00, SPRITE_0_SIZE, 1, file_handle, &readen);
	DL_FsReadFile(spr_01, SPRITE_0_SIZE, 1, file_handle, &readen);
	DL_FsReadFile(spr_02, SPRITE_0_SIZE, 1, file_handle, &readen);
	DL_FsReadFile(spr_03, SPRITE_0_SIZE, 1, file_handle, &readen);
	DL_FsReadFile(spr_ball1, SPRITE_BALL_1_SIZE, 1, file_handle, &readen);
	DL_FsCloseFile(file_handle);
	if (readen == 0) {
		LOG("%s\n", "Yeti3D.spr: Error reading file.");
	}
	sprites[0] = spr_00;
	sprites[1] = spr_01;
	sprites[2] = spr_02;
	sprites[3] = spr_03;
	sprites[4] = spr_ball1;

	return RESULT_OK;
}

static void FreeResourses(void) {
	if (res_tex) {
		LOG("Yeti3D.tex: freed %d bytes.\n", sizeof(texture_t) * YETI_TEXTURE_MAX);
		AmMemFreePointer(res_tex);
		res_tex = NULL;
	}
	if (res_lua) {
		LOG("Yeti3D.lua: freed %d bytes.\n", sizeof(lua_t));
		AmMemFreePointer(res_lua);
		res_lua = NULL;
	}
	if (reciprocal) {
		LOG("Yeti3D.rec: freed %d bytes.\n", sizeof(int) * YETI_RECIPROCAL_MAX);
		AmMemFreePointer(reciprocal);
		reciprocal = NULL;
	}
	if (sintable) {
		LOG("Yeti3D.sin: freed %d bytes.\n", sizeof(int) * YETI_SINTABLE_MAX);
		AmMemFreePointer(sintable);
		sintable = NULL;
	}
	if (e1m1) {
		LOG("Yeti3D.map: freed %d bytes.\n", sizeof(rom_map_t));
		AmMemFreePointer(e1m1);
		e1m1 = NULL;
	}
	if (spr_00) {
		LOG("Yeti3D.spr:spr_00: freed %d bytes.\n", SPRITE_0_SIZE);
		AmMemFreePointer(spr_00);
		sprites[0] = spr_00 = NULL;
	}
	if (spr_01) {
		LOG("Yeti3D.spr:spr_01: freed %d bytes.\n", SPRITE_0_SIZE);
		AmMemFreePointer(spr_01);
		sprites[1] = spr_01 = NULL;
	}
	if (spr_02) {
		LOG("Yeti3D.spr:spr_02: freed %d bytes.\n", SPRITE_0_SIZE);
		AmMemFreePointer(spr_02);
		sprites[2] = spr_02 = NULL;
	}
	if (spr_03) {
		LOG("Yeti3D.spr:spr_03: freed %d bytes.\n", SPRITE_0_SIZE);
		AmMemFreePointer(spr_03);
		sprites[3] = spr_03 = NULL;
	}
	if (spr_ball1) {
		LOG("Yeti3D.spr:spr_ball1: freed %d bytes.\n", SPRITE_BALL_1_SIZE);
		AmMemFreePointer(spr_ball1);
		sprites[4] = spr_ball1 = NULL;
	}
	if (yeti) {
		LOG("yeti: freed %d bytes.\n", sizeof(yeti_t));
		AmMemFreePointer(yeti);
		yeti = NULL;
	}
}
