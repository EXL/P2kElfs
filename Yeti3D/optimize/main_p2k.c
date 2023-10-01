/*
 * About:
 *   The "Yeti3D" is a port of heavy optimized 3D engine demo from GBA to Motorola P2K ELF application.
 *
 * Author:
 *   Derek J. Evans, EXL
 *
 * License:
 *   MIT
 *
 * Additional information:
 *   https://www.gbadev.org/demos.php?showinfo=568
 *
 * Application type:
 *   GUI + ATI
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

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period);

static UINT32 CheckKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ProcessKeyboard(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 key, BOOL pressed);

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);
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
			reg_id, 0, 0, 1, 1, 1, 0);

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

	/* Use this if `Class_dal` constant is unknown or buggy. */
//	start_addr = 0x12000000;
//	search_region = 0x03FFFFFF; /* 4 MB RAM */
//	search_region = 0x07FFFFFF; /* 8 MB RAM */
	start_addr = (UINT32) Class_dal;
#if !defined(SEARCH_LONG_RANGE)
	search_region = 0x00000100;
#else
	search_region = 0x00100000; /* Damn! This can be 0x100000 for Motorola RAZR V3r! */
#endif
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
		"start_addr=0x%08X 0x%08X 0x%08X\n",
		*Class_dal, &Class_dal, Class_dal, *((UINT32 *)start_addr), &start_addr, start_addr);

	LOG("ATI Display Mode Dumps 2:\n\t"
		"search_region=0x%08X\n\t"
		"surface_disp_addr=0x%08X\n\t"
		"surface_draw_addr=0x%08X\n\t"
		"surface_block_offset=0x%08X\n",
		search_region, surface_disp_addr, surface_draw_addr, surface_block_offset);

	LOG("ATI Display Mode Dumps 3:\n\t"
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
			appi->ahi.screen = NULL;
			return RESULT_FAIL;
		}

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);
	} else {
		if (appi->ahi.screen != NULL) {
			status |= AhiSurfFree(appi->ahi.context, appi->ahi.screen);
		}
		if (appi->ahi.draw != NULL) {
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
			appi->ahi.screen = NULL;
			return RESULT_FAIL;
		}

		*surface_disp_addr = (UINT32) appi->ahi.screen;
		*surface_draw_addr = (UINT32) appi->ahi.draw;

		status |= ATI_Driver_Log_Memory(app, AHIFMT_8BPP);
	}

	status |= AhiDispModeGet(appi->ahi.context, &display_mode);

#if defined(FTR_V600)
	DAL_DisableDisplay(DISPLAY_MAIN);
#else
	status |= AhiDispState(appi->ahi.context, DISPLAY_OFF, 0);
#endif

	display_mode.rotation = mode;
	status = AhiDispModeSet(appi->ahi.context, &display_mode, 0);
	if (status != RESULT_OK) {
		LOG("ATI_Driver_Set_Display_Mode: Cannot change display mode, status = %d\n", status);
		return RESULT_FAIL;
	}
	status |= AhiDispSurfSet(appi->ahi.context, appi->ahi.screen, 0);

#if defined(FTR_V600)
	DAL_EnableDisplay(DISPLAY_MAIN);
#else
	status |= AhiDispState(appi->ahi.context, DISPLAY_ON, 0);
#endif

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
	status |= ATI_Driver_Set_Display_Mode(app, (appi->is_CSTN_display) ? AHIROT_270 : AHIROT_90);
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
#if defined(JAVA_HEAP)
	appi->ahi.bitmap.image = AmMemAllocPointer(appi->bmp_width * appi->bmp_height * 2);
	if (!appi->ahi.bitmap.image) {
#else
	appi->ahi.bitmap.image = suAllocMem(appi->bmp_width * appi->bmp_height * 2, &result);
	if (result != RESULT_OK) {
#endif
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
	status |= ATI_Driver_Set_Display_Mode(app, (app_instance->is_CSTN_display) ? AHIROT_180 : AHIROT_0);
#endif

	if (app_instance->p_bitmap) {
		LOG("%s\n", "Free: ATI Bitmap memory.");
#if defined(JAVA_HEAP)
		AmMemFreePointer(app_instance->p_bitmap);
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
	if (yeti) {
		LOG("yeti: freed %d bytes.\n", sizeof(yeti_t));
		AmMemFreePointer(yeti);
		yeti = NULL;
	}
}
