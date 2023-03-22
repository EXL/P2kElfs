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
 * Information:
 *   https://fabiensanglard.net/doom_fire_psx/
 *   https://github.com/EXL/Stuff/tree/master/Sandbox/SDL/doom_fire_demo
 *
 * Application type:
 *   GUI + ATI
 */

#include <loader.h>
#include <apps.h>
#include <ati.h>
#include <dal.h>
#include <mem.h>
#include <time_date.h>
#include <uis.h>

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0x0001,
	APP_TIMER_UPDATE
} APP_TIMER_T;

typedef struct {
	APPLICATION_T app;
} APP_INSTANCE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

// TODO: Rename these functions to GFX or something.
static UINT32 ATI_Init(AHIDEVCONTEXT_T context, AHIDRVINFO_T *info, AHISURFACE_T sDisp, AHISURFACE_T sDraw);
static UINT32 ATI_Deinit(AHIDEVCONTEXT_T context, AHIDRVINFO_T *info);
static UINT32 ATI_Flush(AHIDEVCONTEXT_T context, AHISURFACE_T sDisp, AHISURFACE_T sDraw, AHIRECT_T *rect);
static UINT32 ATI_Draw(AHIDEVCONTEXT_T context);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "FireEffect";

static const UINT32 g_timer_ms = 500;

static AHIDEVCONTEXT_T g_ahi_device_context;
static AHIDRVINFO_T *g_ahi_driver_info;
static AHISURFACE_T g_ahi_surface_display;
static AHISURFACE_T g_ahi_surface_draw;

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DONE, ApplicationStop },
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls },
};

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	status = RESULT_OK;
	ev_code_base = ev_code;

	status |= APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	status |= ATI_Init(g_ahi_device_context, g_ahi_driver_info, g_ahi_surface_display, g_ahi_surface_draw);

	LdrStartApp(ev_code_base);

	return status;
}

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, 1, 0);

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	DeleteDialog(app);

	status |= APP_Exit(ev_st, app, 0);
//	status |= APP_ExitStateAndApp(ev_st, app, 0); // TODO: Reboot on exit. Why? No function in Lib?
	status |= ATI_Deinit(g_ahi_device_context, g_ahi_driver_info);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 ATI_Init(AHIDEVCONTEXT_T context, AHIDRVINFO_T *info, AHISURFACE_T sDisp, AHISURFACE_T sDraw) {
	UINT32 status;
	INT32 result;
	AHIDEVICE_T ahi_device;

	status = RESULT_OK;
	result = RESULT_OK;

	info = suAllocMem(sizeof(AHIDRVINFO_T), &result);
	if (!info && result) {
		return RESULT_FAIL;
	}
	status |= AhiDevEnum(&ahi_device, info, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}
	status |= AhiDevOpen(&context, ahi_device, g_app_name, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}

	sDraw = DAL_GetDrawingSurface(DISPLAY_MAIN);
	status |= AhiDispSurfGet(context, &sDisp);
	status |= AhiDrawSurfDstSet(context, sDisp, 0);
	status |= AhiDrawSurfSrcSet(context, sDraw, 0);
	status |= AhiDrawClipDstSet(context, NULL);
	status |= AhiDrawClipSrcSet(context, NULL);

	return status;
}

static UINT32 ATI_Deinit(AHIDEVCONTEXT_T context, AHIDRVINFO_T *info) {
	UINT32 status;

	status = RESULT_OK;

	status |= AhiDevClose(context);

	if (info) {
		suFreeMem(info);
		info = NULL;
	}

	return status;
}

static UINT32 ATI_Flush(AHIDEVCONTEXT_T context, AHISURFACE_T sDisp, AHISURFACE_T sDraw, AHIRECT_T *rect) {
	UINT32 status;
	AHIPOINT_T point;

	status = RESULT_OK;

	point.x = rect->x1;
	point.y = rect->y1;

	status |= AhiDrawSurfSrcSet(context, sDraw, 0);
	status |= AhiDrawSurfDstSet(context, sDisp, 0);
	status |= AhiDrawClipSrcSet(context, NULL);
	status |= AhiDrawClipDstSet(context, NULL);

	status |= AhiDrawRopSet(context, AHIROP3(AHIROP_SRCCOPY));
	status |= AhiDispWaitVBlank(context, 0);
	status |= AhiDrawBitBlt(context, rect, &point);

	return status;
}

static UINT32 ATI_Draw(AHIDEVCONTEXT_T context) {
	UINT32 status;
	AHIRECT_T rect;

	status = RESULT_OK;

	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = 100;
	rect.y2 = 100;

	status |= AhiDrawSurfDstSet(context, g_ahi_surface_draw, 0);
	status |= AhiDrawBrushFgColorSet(context, ATI_565RGB(0xFF, 0x00, 0x00));
//	status |= AhiDrawBrushBgColorSet(context, ATI_565RGB(0xFF, 0x00, 0x00));
	status |= AhiDrawBrushSet(context, NULL, NULL, 0, AHIFLAG_BRUSH_SOLID);
	status |= AhiDrawRopSet(context, AHIROP3(AHIROP_PATCOPY));

	status |= AhiDrawSpans(context, &rect, 1, 0);

//	AhiDrawBrushFgColorSet( dCtx, color );
//	AhiDrawBrushSet( dCtx, NULL, NULL, 0, AHIFLAG_BRUSH_SOLID );
//	AhiDrawRopSet( dCtx, AHIROP3(AHIROP_PATCOPY) );
//	AhiDrawSpans( dCtx, r, 1, 0 );


	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	DRAWING_BUFFER_T drawing_buffer;
	CONTENT_T content;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;

	switch (app_state) {
		case APP_STATE_MAIN:
			drawing_buffer.w = 128;
			drawing_buffer.h = 160;
			drawing_buffer.buf = NULL;
			APP_UtilStartCyclicalTimer(g_timer_ms, APP_TIMER_UPDATE, app);
			dialog = UIS_CreateColorCanvas(&port, &drawing_buffer, TRUE);
			break;
		default:
			dialog = DialogType_None;
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	// TODO:
	{
	AHIRECT_T rect;
	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = 128;
	rect.y1 = 160;
	ATI_Draw(g_ahi_device_context);
	ATI_Flush(g_ahi_device_context, g_ahi_surface_display, g_ahi_surface_draw, &rect);
	}
	return RESULT_OK;
}

static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state) {
	if (state == EXIT_STATE_EXIT) {
		DeleteDialog(app);
		APP_UtilStopTimer(app);
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

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	if (timer_id == APP_TIMER_UPDATE) {
		AHIRECT_T rect;
		rect.x1 = 0;
		rect.y1 = 0;
		rect.x2 = 128;
		rect.y1 = 160;
		ATI_Draw(g_ahi_device_context);
		ATI_Flush(g_ahi_device_context, g_ahi_surface_display, g_ahi_surface_draw, &rect);
	} else if (timer_id == APP_TIMER_EXIT) {
		/* Play an exit sound using quiet speaker. */
		DL_AudPlayTone(0x00,  0xFF);

		/* Exit App! */
		return ApplicationStop(ev_st, app);
	}

	return RESULT_OK;
}
