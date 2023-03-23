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
#include <dal.h>
#include <uis.h>
#include <mem.h>
#include <time_date.h>

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

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 GFX_Start(APPLICATION_T *app);
static UINT32 GFX_Stop(APPLICATION_T *app);
static UINT32 GFX_Flush(APPLICATION_T *app);
static UINT32 GFX_Draw(APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "FireEffect";

static EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DONE, ApplicationStop },
	{ EV_DIALOG_DONE, ApplicationStop },
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

		status |= GFX_Start((APPLICATION_T *) app_instance);

		status |= APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	DeleteDialog(app);

	status |= APP_Exit(ev_st, app, 0);
	status |= GFX_Stop(app);

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
			dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);
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
		/* TODO: Draw first frame/step. */
		break;
	default:
		break;
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
		case KEY_5:
			GFX_Draw(app);
			GFX_Flush(app);
			break;
		case KEY_RED:
		case KEY_0:
			status |= APP_UtilStartTimer(TIMER_FAST_TRIGGER_MS, APP_TIMER_EXIT, app);
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

	switch (timer_id) {
		case APP_TIMER_UPDATE_FRAME:
			/* TODO: Draw next frame/step. */
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

static UINT32 GFX_Start(APPLICATION_T *app) {
	UINT32 status;
	INT32 result;
	APP_INSTANCE_T *app_instance;
	AHIDEVICE_T ahi_device;

	app_instance = (APP_INSTANCE_T *) app;

	status = RESULT_OK;
	result = RESULT_OK;

	app_instance->ahi_driver_info = suAllocMem(sizeof(AHIDRVINFO_T), &result);
	if (!app_instance->ahi_driver_info && result) {
		return RESULT_FAIL;
	}
	status |= AhiDevEnum(&ahi_device, app_instance->ahi_driver_info, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}
	status |= AhiDevOpen(&app_instance->ahi_device_context, ahi_device, g_app_name, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}

	status |= AhiDispSurfGet(app_instance->ahi_device_context, &app_instance->ahi_surface);
	status |= AhiDrawSurfDstSet(app_instance->ahi_device_context, app_instance->ahi_surface, 0);
	status |= AhiDrawClipDstSet(app_instance->ahi_device_context, NULL);
	status |= AhiDrawClipSrcSet(app_instance->ahi_device_context, NULL);

	status |= AhiSurfInfo(app_instance->ahi_device_context, app_instance->ahi_surface, &app_instance->ahi_surface_info);
	app_instance->ahi_bitmap.width = app_instance->ahi_surface_info.width;
	app_instance->ahi_bitmap.height = app_instance->ahi_surface_info.height;
	app_instance->ahi_bitmap.stride =
		app_instance->ahi_surface_info.width * (app_instance->ahi_surface_info.byteSize /
			(app_instance->ahi_surface_info.width * app_instance->ahi_surface_info.height));
	app_instance->ahi_bitmap.format = AHIFMT_16BPP_565;
	app_instance->ahi_bitmap.image = (void *) display_source_buffer;

	return status;
}

static UINT32 GFX_Stop(APPLICATION_T *app) {
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

static UINT32 GFX_Flush(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	AHIRECT_T ahi_rect;
	AHIPOINT_T ahi_point;

	status = RESULT_OK;

	app_instance = (APP_INSTANCE_T *) app;

	ahi_rect.x1 = 0;
	ahi_rect.y1 = 0;
	ahi_rect.x2 = 0 + app_instance->ahi_surface_info.width;
	ahi_rect.y2 = 0 + app_instance->ahi_surface_info.height;

	ahi_point.x = 0;
	ahi_point.y = 0;

	status |= AhiSurfCopy(app_instance->ahi_device_context, app_instance->ahi_surface, &app_instance->ahi_bitmap, &ahi_rect, &ahi_point, 0, 0);

	if (app_instance->is_CSTN_display) {
		AHIUPDATEPARAMS_T update_params;

		update_params.size = sizeof(AHIUPDATEPARAMS_T);
		update_params.sync = FALSE;
		update_params.rect = ahi_rect;

		status |= AhiDispUpdate(app_instance->ahi_device_context, &update_params);
	}

	return status;
}

static UINT32 GFX_Draw(APPLICATION_T *app) {
	UINT32 status;
	UINT16 x;
	UINT16 y;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	for (x = 0; x < app_instance->ahi_surface_info.width; ++x) {
		for (y = 0; y < app_instance->ahi_surface_info.height; ++y) {
			((UINT16 *) app_instance->ahi_bitmap.image)[x + y * app_instance->ahi_surface_info.width] =
					ATI_565RGB(rand(), rand(), rand());
		}
	}

	return status;
}
