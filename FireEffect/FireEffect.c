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

/******** MOVE IT TO SDK ***/

typedef struct {
	UINT32 size;
	BOOL sync;
	AHIRECT_T rect;
} AHIUPDATEPARAMS_T;

extern UINT32 AhiDispUpdate(AHIDEVCONTEXT_T context, AHIUPDATEPARAMS_T *update_params);

/******** MOVE IT TO SDK ***/

#define TIMER_FAST_TRIGGER_MS             (1)
#define TIMER_FAST_UPDATE_MS              (1000 / 10) /* 27 FPS. */

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
	APPLICATION_T app;

	BOOL is_CSTN_display;
	UINT16 width;
	UINT16 height;
	UINT16 bmp_width;
	UINT16 bmp_height;

	UINT8 *p_fire;
	UINT16 y_coord;

	APP_AHI_T ahi;

	UINT32 timer_handle;
} APP_INSTANCE_T;

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

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);
static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period);

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 ATI_Driver_Start(APPLICATION_T *app);
static UINT32 ATI_Driver_Stop(APPLICATION_T *app);
static UINT32 ATI_Driver_Flush(APPLICATION_T *app);

static UINT32 GFX_Draw_Start(APPLICATION_T *app);
static UINT32 GFX_Draw_Stop(APPLICATION_T *app);
static UINT32 GFX_Draw_Step(APPLICATION_T *app);

static UINT32 SetWorikingArea(GRAPHIC_REGION_T *working_area);

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
		app_instance->p_fire = NULL;
		app_instance->bmp_width = 128;
		app_instance->bmp_height = 88;
		app_instance->timer_handle = 0;

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

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;

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
		case APP_TIMER_LOOP:
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
	GRAPHIC_REGION_T draw_region;

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
	appi->ahi.bitmap.image = suAllocMem(appi->bmp_width * appi->bmp_height, &result);
	if (result) {
		return RESULT_FAIL;
	}
	appi->ahi.rect_bitmap.x1 = 0;
	appi->ahi.rect_bitmap.y1 = 0;
	appi->ahi.rect_bitmap.x2 = 0 + appi->bmp_width;
	appi->ahi.rect_bitmap.y2 = 0 + appi->bmp_height;

	status |= SetWorikingArea(&draw_region);
	appi->ahi.rect_draw.x1 = draw_region.ulc.x;
	appi->ahi.rect_draw.y1 = draw_region.ulc.y;
	appi->ahi.rect_draw.x2 = draw_region.lrc.x + 1;
	appi->ahi.rect_draw.y2 = draw_region.lrc.y + 1;

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
		&appi->ahi.rect_draw, &appi->ahi.point_bitmap, &appi->ahi.bitmap, (void *) fire_palette, 0);

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

	appi->y_coord = 440;
	appi->p_fire = (UINT8 *) appi->ahi.bitmap.image;

	/* Fill all screen to RGB(0x07, 0x07, 0x07) except last line. */
	memset(appi->p_fire, 0, appi->bmp_width * (appi->bmp_height - 1));

	/* Fill last line to RGB(0xFF, 0xFF, 0xFF) except last line. */
	memset((UINT8 *) (appi->p_fire + (appi->bmp_height - 1) * appi->bmp_width), 36, appi->bmp_width);

	return status;
}

static UINT32 GFX_Draw_Stop(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	if (appi->p_fire) {
		suFreeMem(appi->p_fire);
		appi->p_fire = NULL;
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

	if (appi->y_coord != appi->bmp_height / 4) {
		appi->y_coord -= 2;
	} else {
		for(y = appi->bmp_height - 1; y > appi->bmp_height - 8; --y) {
			for(x = 0; x < appi->bmp_width; ++x) {
				if (appi->p_fire[y * appi->bmp_width + x] > 0) {
					appi->p_fire[y * appi->bmp_width + x] -= ((rand() % 2) & 3);
				}
			}
		}
	}

	return status;
}

static UINT32 SetWorikingArea(GRAPHIC_REGION_T *working_area) {
	UINT32 status;
	GRAPHIC_REGION_T rect;
	UINT8 count_lines;
	UINT8 chars_on_line;
	UINT8 height_title_bar_end;
	UINT8 height_soft_keys_start;

	status = RESULT_OK;

	UIS_CanvasGetWorkingArea(&rect, &count_lines, &chars_on_line, TITLE_BAR_AREA, TRUE, 1);
	height_title_bar_end = rect.lrc.y + 1;

	UIS_CanvasGetWorkingArea(&rect, &count_lines, &chars_on_line, SOFTKEY_AREA, TRUE, 1);
	height_soft_keys_start = rect.ulc.y - 1;

	rect.ulc.y = height_title_bar_end;
	/* rect.lrc.x -= 1; */
	rect.lrc.y = height_soft_keys_start;
	/* rect.lrc.y += 1; */

	memcpy(working_area, &rect, sizeof(GRAPHIC_REGION_T));

	return status;
}
