#include <apps.h>
#include <ati.h>
#include <mme.h>
#include <uis.h>

typedef struct {
	APPLICATION_T app;
} ELF_T;

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_DISPLAY_HIDE,
	APP_DISPLAY_SHOW
} APP_DISPLAY_T;

typedef enum {
	APP_TIMER_LONG_PRESS
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_STRING_GOT_IT,
	APP_RESOURCE_ACTION_GOT_IT,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, void *app);
static UINT32 ApplicationDisplay(EVENT_STACK_T *ev_st, void *app, APP_DISPLAY_T display);

static UINT32 InitResourses(RESOURCE_ID *resources);
static UINT32 FreeResourses(RESOURCE_ID *resources);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, void *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(void *app);

static void HandleEventMain(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id);
static UINT32 HandleEventHide(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventShow(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, void *app);

static const char g_app_name[APP_NAME_LEN] = "Screenshot";

static const WCHAR g_msg_state_main[] = L"Hold \"#\" to Screenshot!\nPush \"0\" to Help.\nPush \"*\" to Exit.";
static const WCHAR g_msg_softkey_got_it[] = L"Got it!";

static APP_DISPLAY_T g_app_state = APP_DISPLAY_SHOW;
static RESOURCE_ID g_app_resources[APP_RESOURCE_MAX];

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ EV_KEY_PRESS, HandleEventKeyPress },
	{ EV_KEY_RELEASE, HandleEventKeyRelease },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	/*
	 * EXL, 01-Dec-2022:
	 *  Please comment out the `{ EV_GRANT_TOKEN, APP_HandleUITokenGranted }` construction
	 *  and change `g_app_state` global variable to `APP_HIDE` value
	 *  for pure daemon behavior with no GUI.
	 */
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DONE, HandleEventHide },
	{ EV_DIALOG_DONE, HandleEventHide },
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
	UINT32 routing_stack;
	ELF_T *elf;

	status = RESULT_FAIL;

	UtilLogStringData("1 %s", __func__);

	// TODO: Why fails?!
	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		UtilLogStringData("2 %s", __func__);

		InitResourses(g_app_resources);

		routing_stack = (g_app_state == APP_DISPLAY_SHOW);
		elf = (ELF_T *) APP_InitAppData((void *) HandleEventMain, sizeof(ELF_T), reg_id, 0, 1, 1, 1, routing_stack, 0);
		status = APP_Start(ev_st, &elf->app, APP_STATE_MAIN, g_state_table_hdls, HandleEventHide, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, void *app) {
	UINT32 status;

	DeleteDialog(app);

	FreeResourses(g_app_resources);

	status = APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;
	RES_ACTION_LIST_ITEM_T action;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_GOT_IT], RES_TYPE_STRING,
		(WCHAR *) g_msg_softkey_got_it, (u_strlen(g_msg_softkey_got_it) + 1) * sizeof(WCHAR));

	action.softkey_label = resources[APP_RESOURCE_STRING_GOT_IT];
	action.list_label = resources[APP_RESOURCE_STRING_GOT_IT];
	action.softkey_priority = 1;
	action.list_priority = 1;
	action.isExit = FALSE;
	action.sendDlgDone = FALSE;
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ACTION_GOT_IT], RES_TYPE_ACTION, &action, sizeof(action));

	return status;
}

static UINT32 FreeResourses(RESOURCE_ID *resources) {
	UINT32 status;
	UINT32 i;

	status = RESULT_OK;

	for (i = 0; i < APP_RESOURCE_MAX; ++i) {
		if (resources[i]) {
			status |= DRM_ClearResource(resources[i]);
		}
	}

	return status;
}

static UINT32 ApplicationDisplay(EVENT_STACK_T *ev_st, void *app, APP_DISPLAY_T display) {
	UINT32 status;
	void *hdl;
	UINT32 routing_stack;

	status = RESULT_OK;

	if (g_app_state != display) {
		g_app_state = display;
		hdl = (void *) ((g_app_state == APP_DISPLAY_SHOW) ? APP_HandleEvent : APP_HandleEventPrepost);
		routing_stack = (g_app_state == APP_DISPLAY_SHOW);
		status = APP_ChangeRoutingStack(app, ev_st, hdl, routing_stack, 0, 1, 1);
	}

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state) {
	APPLICATION_T *application;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	ACTIONS_T actions;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	DeleteDialog(app);

	application = (APPLICATION_T *) app;
	port = application->port;

	actions.action[0].operation = ACTION_OP_ADD;
	actions.action[0].event = EV_DONE;
	actions.action[0].action_res = g_app_resources[APP_RESOURCE_ACTION_GOT_IT];
	actions.count = 1;

	UIS_MakeContentFromString("RMq0", &content, g_msg_state_main);

	dialog = UIS_CreateNotice(&port, &content, 0, NOTICE_TYPE_DEFAULT, TRUE, &actions);

	if (dialog == DialogType_Null) {
		return RESULT_FAIL;
	}

	application->dialog = dialog;

	return RESULT_OK;
}

static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, void *app, EXIT_STATE_TYPE_T state) {
	if (state == EXIT_STATE_EXIT) {
		DeleteDialog(app);
		return RESULT_OK;
	}
	return RESULT_FAIL;
}

static UINT32 DeleteDialog(void *app) {
	APPLICATION_T *application;

	application = (APPLICATION_T *) app;

	if (application->dialog != DialogType_Null) {
		UIS_Delete(application->dialog);
		application->dialog = DialogType_Null;
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static void HandleEventMain(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id) {
	if (g_app_state == APP_DISPLAY_SHOW) {
		APP_HandleEvent(ev_st, app, app_id, reg_id);
	} else {
		APP_HandleEventPrepost(ev_st, app, app_id, reg_id);
	}
}

static UINT32 HandleEventHide(EVENT_STACK_T *ev_st, void *app) {
	ApplicationDisplay(ev_st, app, APP_DISPLAY_HIDE);
	return RESULT_OK;
}

static UINT32 HandleEventShow(EVENT_STACK_T *ev_st, void *app) {
	APPLICATION_T *application;

	application = (APPLICATION_T *) app;

	if (application->state != APP_STATE_MAIN) {
		APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);
	}

	ApplicationDisplay(ev_st, app, APP_DISPLAY_SHOW);

	return RESULT_OK;
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, void *app) {
	EVENT_T *event;

	event = AFW_GetEv(ev_st);

	switch (event->data.key_pressed) {
		case KEY_STAR:
			return ApplicationStop(ev_st, app);
			break;
		case KEY_0:
			return HandleEventShow(ev_st, app);
			break;
		case KEY_POUND:
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, void *app) {
	EVENT_T *event;

	event = AFW_GetEv(ev_st);

	switch (event->data.key_pressed) {
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, void *app) {
	return RESULT_OK;
}
