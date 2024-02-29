/*
 * About:
 *   Some example of strange Motorola SLVR L7e bug in ElfPack 1.x
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * Application type:
 *   GUI
 */

#include <loader.h>
#include <apps.h>
#include <mem.h>
#include <mme.h>
#include <uis.h>
#include <utilities.h>

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_OK,
	APP_STATE_FAIL,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_OK = 0xE398,
	APP_TIMER_FAIL
} APP_TIMER_T;

typedef struct {
	APPLICATION_T app;
} APP_INSTANCE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventNo(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 BogoMIPS(void);

static const char g_app_name[APP_NAME_LEN] = "DebugL7e";

static const WCHAR g_msg_state_main[] = L"Execute function?";
static const WCHAR g_msg_state_dump_ok[] =  L"Function executed!";
static const WCHAR g_msg_state_dump_fail[] = L"Something wrong!";

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
	{ EV_NO, HandleEventNo },
	{ EV_YES, HandleEventYes },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_dialog_hdls[] = {
	{ EV_DONE, ApplicationStop },
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls },
	{ APP_STATE_OK, HandleStateEnter, HandleStateExit, g_state_dialog_hdls },
	{ APP_STATE_FAIL, HandleStateEnter, HandleStateExit, g_state_dialog_hdls }
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
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, 1, 0);

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	APP_ConsumeEv(ev_st, app);

	DeleteDialog(app);

	status = APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;

	memclr(&content, sizeof(CONTENT_T));

	switch (app_state) {
		case APP_STATE_MAIN:
			UIS_MakeContentFromString("MCq0", &content, g_msg_state_main);
			dialog = UIS_CreateConfirmation(&port, &content);
			break;
		case APP_STATE_OK:
			UIS_MakeContentFromString("RMq0", &content, g_msg_state_dump_ok);
			dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_OK);
			break;
		case APP_STATE_FAIL:
			UIS_MakeContentFromString("RMq0", &content, g_msg_state_dump_fail);
			dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_FAIL);
			break;
		default:
			dialog = DialogType_None;
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
		DeleteDialog(app);
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

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	if (BogoMIPS() == RESULT_OK) {
		APP_UtilChangeState(APP_STATE_OK, ev_st, app);
		APP_UtilStartTimer(100, APP_TIMER_OK, app);
	} else {
		APP_UtilChangeState(APP_STATE_FAIL, ev_st, app);
		APP_UtilStartTimer(100, APP_TIMER_FAIL, app);
	}
	return RESULT_OK;
}

static UINT32 HandleEventNo(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	ApplicationStop(ev_st, app);
	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	if (timer_id == APP_TIMER_OK) {
		/* Play a normal camera shutter sound using loud speaker. */
		DL_AudPlayTone(0x03,  0xFF);
	} else if (timer_id == APP_TIMER_FAIL) {
		/* Play an error sound using quiet speaker. */
		DL_AudPlayTone(0x02,  0xFF);
	}

	return RESULT_OK;
}

#define TICKS_PER_SEC 8192

static UINT32 BogoMIPS(void) {
	UINT32 loops_per_sec = 1;
	UINT32 i;

	while ((loops_per_sec *= 2)) {
		UINT64 delta_a;
		UINT64 delta_b;
		UINT32 delta;

		delta_a = suPalReadTime();

//		delay_bmips(loops_per_sec);

		for (i = 0; i < loops_per_sec; ++i) ;

		delta_b = suPalReadTime();

		delta = (UINT32) (delta_b - delta_a);

		LOG("=> %lu %lu\n", loops_per_sec, delta);

		if (delta >= TICKS_PER_SEC) {
			UINT32 lps = loops_per_sec;
			UINT32 bmips_i;
			UINT32 bmips_f;
			lps = (lps / delta) * TICKS_PER_SEC;

			bmips_i = lps / 500000;
			bmips_f = (lps / 5000) % 100;

			LOG("CPU: Delta A ticks: %llu\n", delta_a);
			LOG("CPU: Delta A ms: %lu\n", (UINT32) suPalTicksToMsec(delta_a));
			LOG("CPU: Delta B ticks: %llu\n", delta_b);
			LOG("CPU: Delta B ms: %lu\n", (UINT32) suPalTicksToMsec(delta_b));
			LOG("CPU: Delta ticks: %lu\n", delta);
			LOG("CPU: Delta ms: %lu\n", (UINT32) suPalTicksToMsec(delta));
			LOG("CPU: Loops/s: %lu\n", loops_per_sec);
			LOG("CPU: BogoMIPS: %lu.%02lu\n", bmips_i, bmips_f);

			return RESULT_OK;
		}
	}

	LOG("CPU: Error: %s\n", "Cannot calculate BogoMIPS!");
	return RESULT_FAIL;
}
