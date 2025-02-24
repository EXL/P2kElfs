/*
 * About:
 *   The implementation of the popular "Bad Apple" demo for the Motorola P2K platform encoded as 1bpp compressed frames.
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * Additional information:
 *   https://habr.com/ru/articles/645085/
 *   https://en.wikipedia.org/wiki/Bad_Apple!!#Use_as_a_graphical_and_audio_test
 *
 * Application type:
 *   GUI + ATI + zlib
 */

#include <loader.h>
#include <ati.h>
#include <mme_other.h>
#include <apps.h>
#include <dl.h>
#include <dal.h>
#include <uis.h>
#include <mem.h>
#include <time_date.h>
#include <utilities.h>
#include <filesystem.h>
#include <zlib.h>
#include <tasks.h>

#define TIMER_FAST_TRIGGER_MS             (1)
#if defined(FPS_15)
#define TIMER_FAST_UPDATE_MS              (1000 / 15) /* ~15 FPS. */
#elif defined(FPS_30)
#define TIMER_FAST_UPDATE_MS              (1000 / 30) /* ~30 FPS. */
#elif defined(FPS_60)
#define TIMER_FAST_UPDATE_MS              (1000 / 60) /* ~60 FPS. */
#endif

#define ZLIB_IN_BUF_SIZE                  (4 * 1024)
#define ZLIB_OUT_BUF_SIZE                 (ZLIB_IN_BUF_SIZE * 2)

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
	UINT16 width;
	UINT16 height;
	UINT16 frames;
	UINT16 max_compressed_size;
	UINT16 bpp;
	UINT16 frame_size;
} FBM_HEADER_T;

typedef struct {
	UINT32 pressed;
	UINT32 released;
} APP_KEYBOARD_T;

typedef struct {
	APPLICATION_T app;

	BOOL is_CSTN_display;
	UINT16 width;
	UINT16 height;
	FBM_HEADER_T fbm_head;
	UINT8 *fbm_buffer;
	UINT16 fbm_frame;
	UINT8 *zlib_buffer;

	APP_AHI_T ahi;
	APP_KEYBOARD_T keys;
	UINT32 timer_handle;
	UINT8 keyboard_volume_level;
} APP_INSTANCE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */

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

static UINT32 MME_PlayHandler(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 MME_StopHandler(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static void *zcalloc(void *opaque, UINT32 items, UINT32 size);
static void zcfree(void *opaque, void *ptr);

static UINT32 ZLIB_Start(APPLICATION_T *app);
static UINT32 ZLIB_Stop(APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "BadApple";

static WCHAR g_res_file_path[FS_MAX_URI_NAME_LENGTH];
static FILE_HANDLE_T file_handle;
static MME_GC_MEDIA_FILE mme_media_file;
static z_stream d_stream;

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
	{ EV_MME_OPEN_SUCCESS, MME_PlayHandler},
	{ EV_MME_PLAY_COMPLETE, MME_StopHandler},
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

	u_atou(elf_path_uri, g_res_file_path);

	LdrStartApp(ev_code_base);

	return status;
}

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	UINT32 readen;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 0, 1, 1, 1, 0);

		*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
		u_strcat(g_res_file_path, L"BadApple_160p.fbm");
		if (!DL_FsFFileExist(g_res_file_path)) {
			*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
			u_strcat(g_res_file_path, L"BadApple_220p.fbm");
		}
		file_handle = DL_FsOpenFile(g_res_file_path, FILE_READ_MODE, 0);
		DL_FsReadFile(&app_instance->fbm_head, sizeof(FBM_HEADER_T), 1, file_handle, &readen);
		if (readen == 0) {
			LOG("%s\n", "BadApple.fbm: Error reading file.");
			DL_FsCloseFile(file_handle);
			return RESULT_FAIL;
		}

		LOG("fbm_head.width=%d\n", app_instance->fbm_head.width);
		LOG("fbm_head.height=%d\n", app_instance->fbm_head.height);
		LOG("fbm_head.frames=%d\n", app_instance->fbm_head.frames);
		LOG("fbm_head.max_compressed_size=%d\n", app_instance->fbm_head.max_compressed_size);
		LOG("fbm_head.bpp=%d\n", app_instance->fbm_head.bpp);
		LOG("fbm_head.frame_size=%d\n", app_instance->fbm_head.frame_size);

		app_instance->ahi.info_driver = NULL;

		app_instance->fbm_buffer = NULL;
		app_instance->fbm_frame = 0;
		app_instance->timer_handle = 0;
		app_instance->keys.pressed = 0;
		app_instance->keys.released = 0;

		DL_AudGetVolumeSetting(PHONE, &app_instance->keyboard_volume_level);
		DL_AudSetVolumeSetting(PHONE, 0);

		status |= ATI_Driver_Start((APPLICATION_T *) app_instance);

		status |= APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);
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
	status |= ATI_Driver_Stop(app);
	status |= APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

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
	#define KK_2 MULTIKEY_2
	#define KK_UP MULTIKEY_UP
	#define KK_4 MULTIKEY_4
	#define KK_LEFT MULTIKEY_LEFT
	#define KK_6 MULTIKEY_6
	#define KK_RIGHT MULTIKEY_RIGHT
	#define KK_8 MULTIKEY_8
	#define KK_DOWN MULTIKEY_DOWN

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

	appi->ahi.bitmap.width = appi->fbm_head.width;
	appi->ahi.bitmap.height = appi->fbm_head.height;
	appi->ahi.bitmap.stride = appi->fbm_head.frame_size / appi->fbm_head.height; /* (width * bpp) */
	appi->ahi.bitmap.format = AHIFMT_1BPP;
	appi->ahi.bitmap.image = suAllocMem(ZLIB_OUT_BUF_SIZE, &result);
	if (result) {
		return RESULT_FAIL;
	}
	appi->ahi.rect_bitmap.x1 = 0;
	appi->ahi.rect_bitmap.y1 = 0;
	appi->ahi.rect_bitmap.x2 = 0 + appi->fbm_head.width;
	appi->ahi.rect_bitmap.y2 = 0 + appi->fbm_head.height;

	appi->ahi.rect_draw.x1 = 0;
	appi->ahi.rect_draw.y1 = 0;
	appi->ahi.rect_draw.x2 = 0 + appi->fbm_head.width;
	appi->ahi.rect_draw.y2 = 0 + appi->fbm_head.height;

	status |= AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);

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

	AhiDrawSurfDstSet(appi->ahi.context, appi->ahi.screen, 0);
	AhiDrawBitmapBlt(appi->ahi.context,
		&appi->ahi.rect_draw, &appi->ahi.point_bitmap, &appi->ahi.bitmap, NULL, 0);
	AhiDispWaitVBlank(appi->ahi.context, 0);

	if (appi->is_CSTN_display) {
		AhiDispUpdate(appi->ahi.context, &appi->ahi.update_params);
	}

	return RESULT_OK;
}

static UINT32 GFX_Draw_Start(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;
	IFACE_DATA_T if_data;

	appi = (APP_INSTANCE_T *) app;
	if_data.port = app->port;

	*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
	u_strcat(g_res_file_path, L"BadApple.m4a");
	if (!DL_FsFFileExist(g_res_file_path)) {
		*(u_strrchr(g_res_file_path, L'/') + 1) = '\0';
		u_strcat(g_res_file_path, L"BadApple.mp3");
	}

	mme_media_file = MME_GC_playback_create(&if_data, g_res_file_path, NULL, 0, NULL, 0, 0, NULL, NULL);

	appi->fbm_buffer = (UINT8 *) appi->ahi.bitmap.image;

	/* Fill screen to black. */
	memset(appi->fbm_buffer, 0, appi->fbm_head.frame_size);

#if !defined(FPS_60)
	ZLIB_Start(app);
#endif

	APP_MMC_UtilStopVariousTimers();

	return RESULT_OK;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;

	appi = (APP_INSTANCE_T *) app;

#if !defined(FPS_60)
	ZLIB_Stop(app);
#endif

	MME_StopHandler(NULL, app);

	if (appi->fbm_buffer) {
		suFreeMem(appi->fbm_buffer);
		appi->fbm_buffer = NULL;
	}

	DL_FsCloseFile(file_handle);

	APP_MMC_UtilRestartVariousTimers();

	return RESULT_OK;
}

static UINT32 GFX_Draw_Step(APPLICATION_T *app) {
	APP_INSTANCE_T *appi;
	UINT32 zl_size;
	UINT32 readen;

	appi = (APP_INSTANCE_T *) app;
#if !defined(FPS_60)

	inflateReset(&d_stream);

	appi->fbm_frame += 1;
	if (appi->fbm_frame > appi->fbm_head.frames) {
		app->exit_status = TRUE;
	}

	DL_FsReadFile(&zl_size, 1, sizeof(UINT32), file_handle, &readen);
	DL_FsReadFile(appi->zlib_buffer, 1, zl_size, file_handle, &readen);

	d_stream.next_in = (BYTE *) appi->zlib_buffer;
	d_stream.avail_in = readen;

	d_stream.next_out = (BYTE *) appi->fbm_buffer;
	d_stream.avail_out = ZLIB_OUT_BUF_SIZE;

	inflate(&d_stream, Z_SYNC_FLUSH);
#else
	if (appi->fbm_frame <= appi->fbm_head.frames) {
		DL_FsReadFile(appi->fbm_buffer, appi->fbm_head.frame_size, 1, file_handle, &readen);
		appi->fbm_frame += 1;
		suSleep(5, NULL);
	} else {
		DL_FsFSeekFile(file_handle, 0 + sizeof(FBM_HEADER_T), 0);
		appi->fbm_frame = 0;
	}
#endif
	return RESULT_OK;
}

static UINT32 MME_PlayHandler(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	P();

	MME_GC_playback_start(mme_media_file, 0, 0);

	return RESULT_OK;
}

static UINT32 MME_StopHandler(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	P();

	MME_GC_playback_stop(mme_media_file);
	MME_GC_playback_delete(mme_media_file);

	return RESULT_OK;
}

static void *zcalloc(void *opaque, UINT32 items, UINT32 size) {
	INT32 err;
	UINT32 sz;
	void *p;

	sz = items * size;
	p = suAllocMem(sz, &err);
	if (err != RESULT_OK) {
		return NULL;
	}
	return p;
}

static void zcfree(void *opaque, void *ptr) {
	suFreeMem(ptr);
}

static UINT32 ZLIB_Start(APPLICATION_T *app) {
	int err;
	UINT32 zl_size;
	UINT32 readen;
	APP_INSTANCE_T *appi;

	err = 0;
	appi = (APP_INSTANCE_T *) app;

	appi->zlib_buffer = (UINT8 *) malloc(ZLIB_IN_BUF_SIZE);
	memclr(appi->zlib_buffer, ZLIB_IN_BUF_SIZE);

	d_stream.zalloc_func = (void *) zcalloc;
	d_stream.zfree_func = (void *) zcfree;
	d_stream.opaque_func = NULL;

	DL_FsReadFile(&zl_size, 1, sizeof(UINT32), file_handle, &readen);
	DL_FsReadFile(appi->zlib_buffer, 1, zl_size, file_handle, &readen);
	d_stream.next_in = (BYTE *) appi->zlib_buffer;
	d_stream.avail_in = readen;
	err = inflateInit2(&d_stream, -MAX_WBITS);
	LOG("inflateInit2 DONE, err = %d, zl_size = %d\n", err, zl_size);

	d_stream.next_out = (BYTE *) appi->fbm_buffer;
	d_stream.avail_out = ZLIB_OUT_BUF_SIZE;

	LOG("avail_in = %d, next_in = 0x%p\n", d_stream.avail_in, d_stream.next_in);
	LOG("avail_out = %d, next_out = 0x%p\n", d_stream.avail_out, d_stream.next_out);
	err = inflate(&d_stream, Z_SYNC_FLUSH);
	LOG("inflate DONE, err = %d\n", err);
	LOG("avail_in = %d, total_in = %d\n", d_stream.avail_in, d_stream.total_in);
	LOG("avail_out = %d, total_out = %d\n", d_stream.avail_out, d_stream.total_out);

#if 0
	{
		FILE_HANDLE_T a, b;
		a = DL_FsOpenFile(L"/c/Elf/1.dump", FILE_WRITE_MODE, 0);
		DL_FsWriteFile(appi->zlib_buffer, ZLIB_IN_BUF_SIZE, 1, a, &readen);
		DL_FsCloseFile(a);

		b = DL_FsOpenFile(L"/c/Elf/2.dump", FILE_WRITE_MODE, 0);
		DL_FsWriteFile(appi->fbm_buffer, ZLIB_OUT_BUF_SIZE, 1, b, &readen);
		DL_FsCloseFile(b);
	}
#endif

	return RESULT_OK;
}

static UINT32 ZLIB_Stop(APPLICATION_T *app) {
	P();

	inflateEnd(&d_stream);

	return RESULT_OK;
}
