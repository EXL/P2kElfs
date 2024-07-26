/*
 * About:
 *   The "Flip Detector" is ELF deamon for detect closed flip of Motorola clamshell phones and do some things.
 *
 * Author:
 *   EXL, WoiaW
 *
 * License:
 *   MIT
 *
 * Application type:
 *   Daemon
 */

#include <loader.h>
#include <apps.h>
#include <dl.h>
#include <dl_keypad.h>
#include <utilities.h>

#define KEY_LONG_PRESS_START_MS    (500)
#define KEY_LONG_PRESS_STOP_MS     (1500)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0xE893
} APP_TIMER_T;

typedef struct {
	APPLICATION_T app;

	UINT64 ms_key_press_start;
} APP_INSTANCE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);

static UINT32 HandleEventFlipOpen(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventFlipClose(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "FlipDetect";

static const UINT8 g_key_exit = KEY_0;
static const UINT8 g_key_smart = KEY_SMART;

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_FLIP_OPENED, HandleEventFlipClose },
	{ EV_FLIP_CLOSED, HandleEventFlipOpen },
	{ EV_KEY_PRESS, HandleEventKeyPress },
	{ EV_KEY_RELEASE, HandleEventKeyRelease },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
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

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEventPrepost, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 2, 0, 0);

		app_instance->ms_key_press_start = 0ULL;

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	APP_ConsumeEv(ev_st, app);

	status = APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	return RESULT_OK;
}

static UINT32 HandleEventFlipOpen(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	status = DL_DbFeatureStoreState(
		0xD5 /* DL_DB_FEATURE_ID_MMA_MENU_VIEW */,
		0x00 /* DL_DB_FEATURE_STATE_ENUM_MMA_MENU_VIEW_ICONS */
	);

	return status;
}
static UINT32 HandleEventFlipClose(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	status = DL_DbFeatureStoreState(
		0xD5 /* DL_DB_FEATURE_ID_MMA_MENU_VIEW */,
		0x01 /* DL_DB_FEATURE_STATE_ENUM_MMA_MENU_VIEW_LIST */
	);

	return status;
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT8 key;

	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_exit) {
		app_instance->ms_key_press_start = suPalTicksToMsec(suPalReadTime());
	}

	return RESULT_OK;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT8 key;
	UINT32 ms_key_release_stop;

	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_exit) {
		/*
		 * Detect long key press between 500 ms (0.5 s) and 1500 ms (1.5 s) and ignore rest.
		 */
		ms_key_release_stop = (UINT32) (suPalTicksToMsec(suPalReadTime()) - app_instance->ms_key_press_start);

		if ((ms_key_release_stop >= KEY_LONG_PRESS_START_MS) && (ms_key_release_stop <= KEY_LONG_PRESS_STOP_MS)) {
			APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
		}
	} else if (key == g_key_smart) {
		/*
		 * Detect long key press between 500 ms (0.5 s) and 1500 ms (1.5 s) and ignore rest.
		 */
		ms_key_release_stop = (UINT32) (suPalTicksToMsec(suPalReadTime()) - app_instance->ms_key_press_start);

		if ((ms_key_release_stop >= KEY_LONG_PRESS_START_MS) && (ms_key_release_stop <= KEY_LONG_PRESS_STOP_MS)) {
			/* HACK: Inject keypress for exit event. */
			DL_KeyInjectKeyPress(KEY_SOFT_RIGHT, KEY_PRESS, KEY_PORTABLE_ID);
			DL_KeyInjectKeyPress(KEY_SOFT_RIGHT, KEY_RELEASE, KEY_PORTABLE_ID);
		} else {
			/* HACK: Inject keypress for exit event. */
			DL_KeyInjectKeyPress(KEY_JOY_OK, KEY_PRESS, KEY_PORTABLE_ID);
			DL_KeyInjectKeyPress(KEY_JOY_OK, KEY_RELEASE, KEY_PORTABLE_ID);
		}
	}

	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	if (timer_id == APP_TIMER_EXIT) {
		/* Play an exit sound using quiet speaker. */
		DL_AudPlayTone(0x00,  0xFF);

		/* Exit App! */
		return ApplicationStop(ev_st, app);
	}

	return RESULT_OK;
}
