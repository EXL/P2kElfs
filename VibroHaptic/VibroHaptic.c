/*
 * Application mode: Deamon + GUI.
 */

#include <loader.h>
#include <apps.h>
#include <mem.h>
#include <uis.h>
#include <res_def.h>
#include <tasks.h>
#include <utilities.h>

typedef struct {
	APPLICATION_T app;
} ELF_T;

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_EDIT,
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
	APP_RESOURCE_STRING_NAME,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_MENU_ITEM_VIBRATION_SIGNAL,
	APP_MENU_ITEM_VIBRATION_DURATION,
	APP_MENU_ITEM_VIBRATION_VOLTAGE_SIGNAL,
	APP_MENU_ITEM_VIBRATION_VOLTAGE,
	APP_MENU_ITEM_HELP,
	APP_MENU_ITEM_ABOUT,
	APP_MENU_ITEM_EXIT,
	APP_MENU_ITEM_MAX
} APP_MENU_ITEM_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ApplicationDisplay(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_DISPLAY_T display);

static UINT32 InitResourses(RESOURCE_ID *resources);
static UINT32 FreeResourses(RESOURCE_ID *resources);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static void HandleEventMain(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id);
static UINT32 HandleEventHide(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventShow(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventEditData(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventEditCancel(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 SendMenuItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count);

static const char g_app_name[APP_NAME_LEN] = "VibroHaptic";

static const WCHAR g_str_app_name[] = L"Vibro Haptic";
static const WCHAR g_str_vibro_signal[] = L"Motor Signal:";
static const WCHAR g_str_vibro_delay[] = L"Delay (in ms):";
static const WCHAR g_str_vibro_voltage_signal[] = L"Voltage Signal:";
static const WCHAR g_str_vibro_voltage_level[] = L"Voltage Level:";
static const WCHAR g_str_help[] = L"Help...";
static const WCHAR g_str_about[] = L"About...";
static const WCHAR g_str_exit[] = L"Exit";

static const UINT8 g_key_app_menu = KEY_SOFT_LEFT;
static const UINT8 g_key_app_exit = KEY_STAR;

static UINT16 g_option_vibro_motor_signal = 735; /* R3443H: 735, R3551: 721. */
static UINT16 g_option_vibro_motor_send_on = 1;
static UINT16 g_option_vibro_motor_send_off = 0;
static UINT16 g_option_vibro_voltage_signal = 702; /* R3443H: 702, R3551: 688. */
static UINT32 g_option_vibro_voltage_level_on = 0;
static UINT32 g_option_vibro_voltage_level_off = 0;
static UINT32 g_option_vibro_delay = 30;

static APP_DISPLAY_T g_app_state = APP_DISPLAY_HIDE;
static RESOURCE_ID g_app_resources[APP_RESOURCE_MAX];
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
	 *  and change `g_app_state` global variable to `APP_DISPLAY_HIDE` value
	 *  for pure daemon behavior with no GUI at run.
	 */
	/* { EV_GRANT_TOKEN, APP_HandleUITokenGranted }, */
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_REQUEST_LIST_ITEMS, HandleEventRequestListItems },
	{ EV_DONE, HandleEventHide },
	{ EV_DIALOG_DONE, HandleEventHide },
	{ EV_SELECT, HandleEventSelect },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_edit_hdls[] = {
	{ EV_DONE, HandleEventEditCancel },
	{ EV_DIALOG_DONE, HandleEventEditCancel },
	{ EV_DATA, HandleEventEditData },
	{ EV_CANCEL, HandleEventEditCancel },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls },
	{ APP_STATE_EDIT, HandleStateEnter, HandleStateExit, g_state_edit_hdls }
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
		status = APP_Start(ev_st, &elf->app, APP_STATE_INIT, g_state_table_hdls, HandleEventHide, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	DeleteDialog(app);

	FreeResourses(g_app_resources);

	status = APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_NAME], RES_TYPE_STRING,
		(void *) g_str_app_name, (u_strlen(g_str_app_name) + 1) * sizeof(WCHAR));

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

static UINT32 ApplicationDisplay(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_DISPLAY_T display) {
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

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	UINT32 starting_list_items;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;

	switch (app_state) {
		case APP_STATE_MAIN:
			starting_list_items = APP_MENU_ITEM_MAX;
			dialog = UIS_CreateList(&port, 0, APP_MENU_ITEM_MAX, 0, &starting_list_items, 0, 2, NULL, g_app_resources[APP_RESOURCE_STRING_NAME]);
			break;
		case APP_STATE_EDIT:

			break;
		default:
			dialog = DialogType_None;
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	SendMenuItemsToList(ev_st, app, 1, APP_MENU_ITEM_MAX);

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

static void HandleEventMain(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id) {
	if (g_app_state == APP_DISPLAY_SHOW) {
		APP_HandleEvent(ev_st, app, app_id, reg_id);
	} else {
		APP_HandleEventPrepost(ev_st, app, app_id, reg_id);
	}
}

static UINT32 HandleEventHide(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	ApplicationDisplay(ev_st, app, APP_DISPLAY_HIDE);
	return RESULT_OK;
}

static UINT32 HandleEventShow(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	if (app->state != APP_STATE_MAIN) {
		APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);
	}

	ApplicationDisplay(ev_st, app, APP_DISPLAY_SHOW);

	return RESULT_OK;
}

static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;

	event = AFW_GetEv(ev_st);

	UtilLogStringData("SELECT");

	APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
	return RESULT_OK;
}

static UINT32 HandleEventRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;
	UINT32 start;
	UINT32 count;

	status = RESULT_OK;

	if (!app->focused) {
		return status;
	}

	event = AFW_GetEv(ev_st);
	start = event->data.list_items_req.begin_idx;
	count = event->data.list_items_req.count;

	status |= APP_ConsumeEv(ev_st, app);
	status |= SendMenuItemsToList(ev_st, app, start, count);

	return status;
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	UINT8 key;
	UINT8 dialog;

	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;
	dialog = DialogType_None;

	if (key == g_key_app_menu || key == g_key_app_exit) {
		g_ms_key_press_start = suPalTicksToMsec(suPalReadTime());
	}

	switch (key) {
		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
		case KEY_SOFT_RIGHT:
		/* case KEY_SOFT_LEFT: */ /* Disable "Back" softkey. */
		case KEY_JOY_OK:
			UIS_GetActiveDialogType(&dialog);
			if (dialog == DialogType_Menu || dialog == DialogType_SecondLevelMenu) {
				/* Set vibration motor voltage. */
				hPortWrite(g_option_vibro_voltage_signal, g_option_vibro_voltage_level_on);

				/* Start vibration motor. */
				hPortWrite(g_option_vibro_motor_signal, g_option_vibro_motor_send_on);

				/* Delay using SUAPI because APP_UtilStartTimer() is slow. */
				suSleep(g_option_vibro_delay, NULL);

				/* Stop vibration motor. */
				hPortWrite(g_option_vibro_motor_signal, g_option_vibro_motor_send_off);

				/* Reset vibration motor voltage. */
				hPortWrite(g_option_vibro_voltage_signal, g_option_vibro_voltage_level_off);
			}
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
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

static UINT32 HandleEventEditData(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	return RESULT_OK;
}

static UINT32 HandleEventEditCancel(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	return RESULT_OK;
}

static UINT32 SendMenuItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count) {
	UINT32 status;
	INT32 result;
	UINT32 i;
	LIST_ENTRY_T *list;

	status = RESULT_OK;
	result = RESULT_OK;

	if (count == 0) {
		return RESULT_FAIL;
	}
	list = (LIST_ENTRY_T *) suAllocMem(sizeof(LIST_ENTRY_T) * APP_MENU_ITEM_MAX, &result);
	if (result != RESULT_OK) {
		return RESULT_FAIL;
	}

	for (i = 0; i < APP_MENU_ITEM_MAX; ++i) {
		list[i].editable = FALSE;
		list[i].content.static_entry.formatting = TRUE;
	}

	status |= UIS_MakeContentFromString("Mq0Si1",
		&list[APP_MENU_ITEM_VIBRATION_SIGNAL].content.static_entry.text,
		g_str_vibro_signal, g_option_vibro_motor_signal);
	status |= UIS_MakeContentFromString("Mq0Si1",
		&list[APP_MENU_ITEM_VIBRATION_DURATION].content.static_entry.text,
		g_str_vibro_delay, g_option_vibro_delay);
	status |= UIS_MakeContentFromString("Mq0Si1",
		&list[APP_MENU_ITEM_VIBRATION_VOLTAGE_SIGNAL].content.static_entry.text,
		g_str_vibro_voltage_signal, g_option_vibro_voltage_signal);
	status |= UIS_MakeContentFromString("Mq0Si1",
		&list[APP_MENU_ITEM_VIBRATION_VOLTAGE].content.static_entry.text,
		g_str_vibro_voltage_level, g_option_vibro_voltage_level_on);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_HELP].content.static_entry.text,
		g_str_help);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_ABOUT].content.static_entry.text,
		g_str_about);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_EXIT].content.static_entry.text,
		g_str_exit);

	status |= APP_UtilAddEvUISListData(ev_st, app, 0, start, APP_MENU_ITEM_MAX, FBF_LEAVE,
		sizeof(LIST_ENTRY_T) * APP_MENU_ITEM_MAX, list);
	if (status != RESULT_FAIL) {
		UIS_HandleEvent(app->dialog, ev_st);
	}

	suFreeMem(list);

	return status;
}
