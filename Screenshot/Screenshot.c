#include <apps.h>

typedef struct {
	APPLICATION_T app;
} ELF_T;

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATES_T;

typedef enum {
	APP_HIDE,
	APP_SHOW
} APP_STATE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 AppStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 AppStop(EVENT_STACK_T *ev_st, void *app);
static UINT32 AppShow(EVENT_STACK_T *ev_st, void *app, APP_STATE_T state);

static void HandleAppState(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id);
static UINT32 HandleAppHide(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleAppShow(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleMainStateEnter(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleKeyPress(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleKeyRelease(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleTimerExpired(EVENT_STACK_T *ev_st, void *app);

static const char g_app_name[APP_NAME_LEN] = "Screenshot";
static APP_STATE_T g_app_state = APP_HIDE;

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	// FIXME: Check this one.
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ EV_KEY_PRESS, HandleKeyPress },
	{ EV_KEY_RELEASE, HandleKeyRelease },
	{ EV_TIMER_EXPIRED, HandleTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	// FIXME: Check this.
	/* { EV_GRANT_TOKEN, APP_HandleUITokenGranted }, */
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	// FIXME: Only one please?
	{ EV_DONE, HandleAppHide },
	{ EV_DIALOG_DONE, HandleAppHide },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleMainStateEnter, NULL, g_state_main_hdls }
};

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) AppStart);

	LdrStartApp(ev_code_base);

	return status;
}

static UINT32 AppStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	UINT32 state;
	ELF_T *elf;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		state = (g_app_state == APP_SHOW);
		elf = (ELF_T *) APP_InitAppData((void *) HandleAppState, sizeof(ELF_T), reg_id, 0, 1, 1, 1, state, 0);
		status = APP_Start(ev_st, &elf->app, APP_STATE_MAIN, g_state_table_hdls, HandleAppHide, g_app_name, 0);
	}

	return status;
}

static UINT32 AppStop(EVENT_STACK_T *ev_st, void *app) {
	UINT32 status;
	APPLICATION_T *application;

	application = (APPLICATION_T *) app;
	APP_UtilUISDialogDelete(&application->dialog);

	status = APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 AppShow(EVENT_STACK_T *ev_st, void *app, APP_STATE_T state) {
	UINT32 status;
	void *hdl;
	UINT32 routing_stack;

	status = RESULT_OK;

	if (g_app_state != state) {
		g_app_state = state;
		hdl = (void *) ((g_app_state == APP_SHOW) ? APP_HandleEvent : APP_HandleEventPrepost);
		routing_stack = (g_app_state == APP_SHOW);
		status = APP_ChangeRoutingStack(app, ev_st, hdl, routing_stack, 0, 1, 1);
	}

	return status;
}

static void HandleAppState(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id) {
	if (g_app_state == APP_SHOW) {
		APP_HandleEvent(ev_st, app, app_id, reg_id);
	} else {
		APP_HandleEventPrepost(ev_st, app, app_id, reg_id);
	}
}

static UINT32 HandleAppHide(EVENT_STACK_T *ev_st, void *app) {
	AppShow(ev_st, app, FALSE);
	return RESULT_OK;
}

static UINT32 HandleAppShow(EVENT_STACK_T *ev_st, void *app) {
	APPLICATION_T *application;

	application = (APPLICATION_T *) app;

	if (application->state != APP_STATE_MAIN) {
		APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);
	}

	AppShow(ev_st, app, TRUE);

	return RESULT_OK;
}

static UINT32 HandleMainStateEnter(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state) {
	APPLICATION_T *application;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;

	WCHAR msg[] = L"Hold '#' key to Screenshot!\nPush '0' to Exit."; // TODO: GLobal?

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	application = (APPLICATION_T *) app;
	port = application->port;
	UIS_MakeContentFromString("MCq0", &content, msg);

	dialog = UIS_CreateConfirmation(&port, &content);

	if (dialog == 0) {
		return RESULT_FAIL;
	}

	application->dialog = dialog;

	return RESULT_OK;
}

static UINT32 HandleKeyPress(EVENT_STACK_T *ev_st, void *app) {
	EVENT_T *event;

	event = AFW_GetEv(ev_st);

	switch (event->data.key_pressed) {
		case KEY_POUND:
			break;
		case KEY_0:
			return AppStop(ev_st, app);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleKeyRelease(EVENT_STACK_T *ev_st, void *app) {
	EVENT_T *event;

	event = AFW_GetEv(ev_st);

	switch (event->data.key_pressed) {
		case KEY_POUND:
			HandleAppShow(ev_st, app);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleTimerExpired(EVENT_STACK_T *ev_st, void *app) {
	return RESULT_OK;
}
