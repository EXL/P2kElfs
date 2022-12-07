/*
 * Application mode: GUI + Deamon.
 */

#include <apps.h>
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
	APP_TIMER_EXIT
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_STRING_GOT_IT,
	APP_RESOURCE_ACTION_GOT_IT,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_ITEM_VIBRATION_SIGNAL,
	APP_ITEM_VIBRATION_DURATION,
	APP_ITEM_VIBRATION_VOLTAGE_SIGNAL,
	APP_ITEM_VIBRATION_VOLTAGE,
	APP_ITEM_ABOUT,
	APP_ITEM_EXIT,
	APP_ITEM_MAX
} APP_ITEM_MENU_T;

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

static const char g_app_name[APP_NAME_LEN] = "VibroHaptic";

static const UINT8 g_key_app_menu = KEY_LSOFT;
static const UINT8 g_key_app_exit = KEY_STAR;

static UINT16 g_option_vibro_signal = 735; /* R3443H: 735, R3551: 721. */
static UINT16 g_option_vibro_voltage_signal = 702; /* R3443H: 702, R3551: 688. */
static UINT32 g_option_vibro_voltage_level = 0;
static UINT32 g_option_vibro_delay = 30;

static APP_DISPLAY_T g_app_state = APP_DISPLAY_HIDE;
static RESOURCE_ID g_app_resources[APP_RESOURCE_MAX];
static LIST_ENTRY_T *g_app_menu;
static UINT64 g_ms_key_press_start = 0LLU;

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
	 *  for pure daemon behavior with no GUI at run.
	 */
	/* { EV_GRANT_TOKEN, APP_HandleUITokenGranted }, */
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

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
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
	INT32 result, i, idx;
	RES_ACTION_LIST_ITEM_T action;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_GOT_IT], RES_TYPE_STRING,
		L"DEBUG", 6 * sizeof(WCHAR));

	action.softkey_label = resources[APP_RESOURCE_STRING_GOT_IT];
	action.list_label = resources[APP_RESOURCE_STRING_GOT_IT];
	action.softkey_priority = 1;
	action.list_priority = 1;
	action.isExit = FALSE;
	action.sendDlgDone = FALSE;
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ACTION_GOT_IT], RES_TYPE_ACTION, &action, sizeof(action));

	g_app_menu = suAllocMem(sizeof(LIST_ENTRY_T) * APP_ITEM_MAX, &result);
	status |= (result != 0);

	for ( i=0, idx=0; i<APP_ITEM_MAX; i++, idx++ ) {
		memclr(&g_app_menu[i], sizeof(LIST_ENTRY_T));
		switch (idx-1) {
			case APP_ITEM_VIBRATION_SIGNAL:
				g_app_menu[i].editable = TRUE;
				g_app_menu[i].content.editable_entry.descr_res = RES_FIELD_NUMBER;
				g_app_menu[i].content.editable_entry.data = g_option_vibro_signal;
				//UIS_MakeContentFromString("s0", &g_app_menu[i].content.static_entry.text, LANG_BACK );
				break;
		}
	}

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

	suFreeMem(g_app_menu);

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
	actions.action[0].event = EV_DIALOG_DONE;
	actions.action[0].action_res = g_app_resources[APP_RESOURCE_ACTION_GOT_IT];
	actions.count = 1;

//	UIS_MakeContentFromString("RMq0", &content, L"TEST");

	dialog = UIS_CreateStaticList(&port, 0, APP_ITEM_MAX, 0, g_app_menu, 0, 2, &actions, APP_RESOURCE_ACTION_GOT_IT);

	// dialog = UIS_CreateNotice(&port, &content, 0, NOTICE_TYPE_DEFAULT, FALSE, &actions);

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
	UINT8 key;
	UINT8 dialog;

	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;
	dialog = DialogType_Null;

	if (key == g_key_app_menu || key == g_key_app_exit) {
		g_ms_key_press_start = suPalTicksToMsec(suPalReadTime());
	}

	switch (key) {
		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
		case KEY_RSOFT:
		/* case KEY_LSOFT: */ /* Disable "Back" softkey. */
		case KEY_CENTER:
			UIS_GetActiveDialogType(&dialog);
			if (dialog == DialogType_Menu || dialog == DialogType_SecondLevelMenu) {
				/* Set vibration motor voltage. */
				hPortWrite(g_option_vibro_voltage_signal, g_option_vibro_voltage_level);

				/* Start vibration motor. */
				hPortWrite(g_option_vibro_signal, 1);

				/* Delay using SUAPI because APP_UtilStartTimer() is slow. */
				suSleep(g_option_vibro_delay, NULL);

				/* Stop vibration motor. */
				hPortWrite(g_option_vibro_signal, 0);

				/* Reset vibration motor voltage. */
				hPortWrite(g_option_vibro_voltage_signal, 0);
			}
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, void *app) {
	EVENT_T *event;
	UINT8 key;
	UINT32 ms_key_release_stop;

	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_app_menu || key == g_key_app_exit) {
		/*
		 * Detect long key press between 500 ms (0.5 s) and 1500 ms (1.5 s) and ignore rest.
		 */
		ms_key_release_stop = (UINT32) (suPalTicksToMsec(suPalReadTime()) - g_ms_key_press_start);
		if ((ms_key_release_stop >= 500) && (ms_key_release_stop <= 1500)) {
			if (key == g_key_app_menu) {
				APP_ConsumeEv(ev_st, app);
				return HandleEventShow(ev_st, app);
			} else if (key == g_key_app_exit) {
				APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
			}
		}
	}

	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, void *app) {
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
