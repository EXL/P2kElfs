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
 *   https://fabiensanglard.net/doom_fire_psx/
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
#define TIMER_FAST_UPDATE_MS              (1000 / 27) /* 27 FPS. */

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0x0001,
	APP_TIMER_UPDATE_FRAME
} APP_TIMER_T;

typedef struct {
	APPLICATION_T app;

	BOOL is_CSTN_display;
	UINT16 width;
	UINT16 height;

	UINT8 *fire_map;
	UINT16 y_coord;

	AHIDRVINFO_T *ahi_driver_info;
	AHIDEVCONTEXT_T ahi_device_context;
	AHISURFACE_T ahi_surface;
	AHISURFINFO_T ahi_surface_info;
	AHIBITMAP_T ahi_bitmap;
} APP_INSTANCE_T;

/******** MOVE IT TO SDK ***/

typedef struct {
	UINT32 size;
	BOOL sync;
	AHIRECT_T rect;
} AHIUPDATEPARAMS_T;

extern UINT32 AhiDispUpdate(AHIDEVCONTEXT_T context, AHIUPDATEPARAMS_T *update_params);

/******** MOVE IT TO SDK ***/

static const UINT16 g_palette_rgb_table[] = {
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

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 ATI_Driver_Start(APPLICATION_T *app);
static UINT32 ATI_Driver_Stop(APPLICATION_T *app);
static UINT32 ATI_Driver_Flush(APPLICATION_T *app);

static UINT32 GFX_Draw_Start(APPLICATION_T *app);
static UINT32 GFX_Draw_Stop(APPLICATION_T *app);
static UINT32 GFX_Draw_Step(APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "FireEffect";

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
	{ EV_INK_KEY_PRESS, HandleEventKeyPress },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, NULL, g_state_main_hdls }
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

		app_instance->ahi_driver_info = NULL;
		app_instance->fire_map = NULL;
		app_instance->y_coord = 440;

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
	status |= APP_UtilStopTimer(app);
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
	GRAPHIC_POINT_T point;
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (state != ENTER_STATE_ENTER) {
		if (app->state != APP_STATE_MAIN) {
			return RESULT_OK;
		}
	}

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;
	dialog = DialogType_None;

	switch (app_state) {
		case APP_STATE_MAIN:
			point = UIS_CanvasGetDisplaySize();
			buffer.w = point.x + 1;
			buffer.h = point.y + 1;
			buffer.buf = NULL;
			app_instance->is_CSTN_display = (buffer.w < DISPLAY_WIDTH) || (buffer.h < DISPLAY_HEIGHT); /* Motorola L6 */
			app_instance->width = buffer.w;
			app_instance->height = buffer.h;

			dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);

			GFX_Draw_Start(app);

			APP_UtilStartCyclicalTimer(TIMER_FAST_UPDATE_MS, APP_TIMER_UPDATE_FRAME, app);

			break;
		default:
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;
/*
	switch (app_state) {
		case APP_STATE_MAIN:
			GFX_Draw_Step(app);
			ATI_Driver_Flush(app);
			break;
		default:
			break;
	}
*/
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

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;

	GRAPHIC_POINT_T p;
	p.x = 10;
	p.y = 10;

	status = RESULT_OK;
	event = AFW_GetEv(ev_st);

	APP_ConsumeEv(ev_st, app);

	switch (event->data.key_pressed) {
		case KEY_RED:
		case KEY_0:
			status |= APP_UtilStartTimer(TIMER_FAST_TRIGGER_MS, APP_TIMER_EXIT, app);
			break;
		case KEY_STAR:
			DL_AudPlayTone(0x00,  0xFF);
			return ApplicationStop(ev_st, app);
			break;
		default:
			break;
	}

	return status;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	APP_ConsumeEv(ev_st, app);

	switch (timer_id) {
		case APP_TIMER_UPDATE_FRAME:
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

	appi = (APP_INSTANCE_T *) app;

	status = RESULT_OK;
	result = RESULT_OK;

	appi->ahi_driver_info = suAllocMem(sizeof(AHIDRVINFO_T), &result);
	if (!appi->ahi_driver_info && result) {
		return RESULT_FAIL;
	}
	status |= AhiDevEnum(&ahi_device, appi->ahi_driver_info, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}
	status |= AhiDevOpen(&appi->ahi_device_context, ahi_device, g_app_name, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}

#define LOG_ATI(format, ...) UtilLogStringData(format, ##__VA_ARGS__); PFprintf(format, ##__VA_ARGS__)
	{
		UINT32 result;
		UINT32 size;
		UINT32 align;

		LOG_ATI("ATI Driver Name: %s\n", appi->ahi_driver_info->drvName);
		LOG_ATI("ATI Driver Version: %s\n", appi->ahi_driver_info->drvVer);
		LOG_ATI("ATI S/W Revision: %d (0x%08X)\n",
			appi->ahi_driver_info->swRevision, appi->ahi_driver_info->swRevision);
		LOG_ATI("ATI Chip ID: %d (0x%08X)\n",
			appi->ahi_driver_info->chipId, appi->ahi_driver_info->chipId);
		LOG_ATI("ATI Revision ID: %d (0x%08X)\n",
			appi->ahi_driver_info->revisionId, appi->ahi_driver_info->revisionId);
		LOG_ATI("ATI CPU Bus Interface Mode: %d (0x%08X)\n",
			appi->ahi_driver_info->cpuBusInterfaceMode, appi->ahi_driver_info->cpuBusInterfaceMode);
		LOG_ATI("ATI Total Memory: %d (%d KiB)\n",
			appi->ahi_driver_info->totalMemory, appi->ahi_driver_info->totalMemory / 1024);
		LOG_ATI("ATI Internal Memory: %d (%d KiB)\n",
			appi->ahi_driver_info->internalMemSize, appi->ahi_driver_info->internalMemSize / 1024);
		LOG_ATI("ATI External Memory: %d (%d KiB)\n",
			appi->ahi_driver_info->externalMemSize, appi->ahi_driver_info->externalMemSize / 1024);
		LOG_ATI("ATI CAPS 1: %d (0x%08X)\n", appi->ahi_driver_info->caps1, appi->ahi_driver_info->caps1);
		LOG_ATI("ATI CAPS 2: %d (0x%08X)\n", appi->ahi_driver_info->caps2, appi->ahi_driver_info->caps2);

		result = AhiSurfGetLargestFreeBlockSize(appi->ahi_device_context, AHIFMT_16BPP_565,
			&size, &align, AHIFLAG_INTMEMORY);
		LOG_ATI("ATI Internal Memory Largest Block: result=%d, size=%d, size=%d KiB, align=%d\n",
			result, size, size / 1024, align);

		result = AhiSurfGetLargestFreeBlockSize(appi->ahi_device_context, AHIFMT_16BPP_565,
			&size, &align, AHIFLAG_EXTMEMORY);
		LOG_ATI("ATI External Memory Largest Block: result=%d, size=%d, size=%d KiB, align=%d\n",
			result, size, size / 1024, align);
	}
#undef LOG_ATI

	status |= AhiDispSurfGet(appi->ahi_device_context, &appi->ahi_surface);
	status |= AhiDrawSurfDstSet(appi->ahi_device_context, appi->ahi_surface, 0);
	status |= AhiDrawClipDstSet(appi->ahi_device_context, NULL);
	status |= AhiDrawClipSrcSet(appi->ahi_device_context, NULL);

	status |= AhiSurfInfo(appi->ahi_device_context, appi->ahi_surface, &appi->ahi_surface_info);
	appi->ahi_bitmap.width = appi->ahi_surface_info.width;
	appi->ahi_bitmap.height = appi->ahi_surface_info.height;
	appi->ahi_bitmap.stride = appi->ahi_surface_info.width *
		(appi->ahi_surface_info.byteSize / (appi->ahi_surface_info.width * appi->ahi_surface_info.height));
	appi->ahi_bitmap.format = AHIFMT_16BPP_565;
	appi->ahi_bitmap.image = (void *) display_source_buffer;

	return status;
}

static UINT32 ATI_Driver_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	status = RESULT_OK;

	status |= AhiDevClose(app_instance->ahi_device_context);
	if (app_instance->ahi_driver_info) {
		suFreeMem(app_instance->ahi_driver_info);
	}

	return status;
}

static UINT32 ATI_Driver_Flush(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;
	AHIRECT_T ahi_rect;
	AHIPOINT_T ahi_point;

	status = RESULT_OK;

	appi = (APP_INSTANCE_T *) app;

	ahi_rect.x1 = 0;
	ahi_rect.y1 = 0;
	ahi_rect.x2 = 0 + appi->ahi_surface_info.width;
	ahi_rect.y2 = 0 + appi->ahi_surface_info.height;

	ahi_point.x = 0;
	ahi_point.y = 0;

	AhiDrawRopSet(appi->ahi_device_context, AHIROP3(AHIROP_SRCCOPY));
	status |= AhiDrawBitmapBlt(appi->ahi_device_context, &ahi_rect, &ahi_point, &appi->ahi_bitmap, NULL, 0);

	if (appi->is_CSTN_display) {
		AHIUPDATEPARAMS_T update_params;

		update_params.size = sizeof(AHIUPDATEPARAMS_T);
		update_params.sync = FALSE;
		update_params.rect = ahi_rect;

		status |= AhiDispUpdate(appi->ahi_device_context, &update_params);
	}

	return status;
}

static UINT32 GFX_Draw_Start(APPLICATION_T *app) {
	UINT32 status;
	UINT16 i;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	if (!appi->fire_map) {
		appi->fire_map = suAllocMem(appi->width * appi->height, NULL);
	}

	for (i = 0; i < appi->width * appi->height; ++i) {
		appi->fire_map[i] = 0; /* 0x07, 0x07, 0x07 */
	}

	for (i = 0; i < appi->width; ++i) {
		appi->fire_map[(appi->height - 1) * appi->width + i] = 36; /* 0xFF, 0xFF, 0xFF */
	}

	return status;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	UINT32 status;
	UINT16 i;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	if (appi->fire_map) {
		suFreeMem(appi->fire_map);
	}

	return status;
}

static UINT32 GFX_Draw_Step(APPLICATION_T *app) {
	UINT32 status;
	UINT16 x;
	UINT16 y;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	for (x = 0; x < appi->width; ++x) {
		for (y = 1; y < appi->height; ++y) {
			const int pixel = appi->fire_map[y * appi->width + x];
			if (pixel == 0) {
				appi->fire_map[(y * appi->width + x) - appi->width] = 0;
			} else {
				const int randIdx = rand() % 4;
				const int dst = (y * appi->width + x) - randIdx + 1;
				appi->fire_map[dst - appi->width] = pixel - (randIdx & 1);
			}
		}
	}

	// TODO: Clear to Black?
	// TODO: Blit bmp?

	for (y = 0; y < appi->height; ++y) {
		for (x = 0; x < appi->width; ++x) {
			const UINT16 pixel = g_palette_rgb_table[appi->fire_map[y * appi->width + x]];
			((UINT16 *) appi->ahi_bitmap.image)[x + y * appi->ahi_surface_info.width] = pixel;
		}
	}

	if (appi->y_coord != appi->height / 4) {
		appi->y_coord -= 2;
	} else {
		for(y = appi->height - 1; y > appi->height - 8; --y) {
			for(x = 0; x < appi->width; ++x) {
				if (appi->fire_map[y * appi->width + x] > 0) {
					appi->fire_map[y * appi->width + x] -= ((rand() % 2) & 3);
				}
			}
		}
	}

	return status;
}
