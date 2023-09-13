/*
 * About:
 *   The "Ambilight" ELF daemon utility with GUI settings for some funlights and bias lighting functionality.
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * Application type:
 *   Deamon + GUI + ATI + Funlights.
 */

#include <loader.h>
#include <apps.h>
#include <mem.h>
#include <uis.h>
#include <res_def.h>
#include <utilities.h>
#include <filesystem.h>

#define MAX_NUMBER_LENGTH           (6)
#define MAX_NUMBER_VALUE            (999999)
#define KEY_LONG_PRESS_START_MS     (1500)
#define KEY_LONG_PRESS_STOP_MS      (3000)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_EDIT,
	APP_STATE_SELECT,
	APP_STATE_POPUP,
	APP_STATE_WARNING,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_DISPLAY_HIDE,
	APP_DISPLAY_SHOW
} APP_DISPLAY_T;

typedef enum {
	APP_TIMER_EXIT = 0xE490,
	APP_TIMER_DO_LIGHTS
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_STRING_NAME,
	APP_RESOURCE_STRING_MODE,
	APP_RESOURCE_STRING_DELAY,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_MODE = APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_DELAY,
	APP_MENU_ITEM_START_LIGHTS,
	APP_MENU_ITEM_STOP_LIGHTS,
	APP_MENU_ITEM_HELP,
	APP_MENU_ITEM_ABOUT,
	APP_MENU_ITEM_EXIT,
	APP_MENU_ITEM_MAX
} APP_MENU_ITEM_T;

typedef enum {
	APP_SELECT_ITEM_FIRST,
	APP_SELECT_ITEM_AMBILIGHT = APP_SELECT_ITEM_FIRST,
	APP_SELECT_ITEM_FLASH_50,
	APP_SELECT_ITEM_FLASH_100,
	APP_SELECT_ITEM_RAINBOW,
	APP_SELECT_ITEM_RANDOM,
	APP_SELECT_ITEM_STROBOSCOPE,
	APP_SELECT_ITEM_STROBO_FLASH,
	APP_SELECT_ITEM_MAX
} APP_SELECT_ITEM_T;

typedef enum {
	APP_POPUP_CHANGED,
	APP_POPUP_RESETED
} APP_POPUP_T;

typedef enum {
	APP_VIEW_HELP,
	APP_VIEW_ABOUT
} APP_VIEW_T;

typedef struct {
	APP_SELECT_ITEM_T mode;
	UINT32 delay;
} APP_OPTIONS_T;

typedef struct {
	APPLICATION_T app;

	APP_DISPLAY_T state;
	RESOURCE_ID resources[APP_RESOURCE_MAX];
	APP_POPUP_T popup;
	APP_VIEW_T view;
	APP_MENU_ITEM_T menu_current_item_index;
	UINT64 ms_key_press_start;
	APP_OPTIONS_T options;
	UINT32 timer_handle;
} APP_INSTANCE_T;

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
static UINT32 HandleEventMenuRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventEditData(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventEditDone(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventSelectDone(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventSelectRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleEventRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_STATE_T app_state);
static UINT32 HandleEventCancel(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 SendMenuItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count);
static UINT32 SendSelectItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count);
static const WCHAR *GetTriggerOptionString(APP_SELECT_ITEM_T item);
static UINT32 ReadFileConfig(APPLICATION_T *app, const WCHAR *file_config_path);
static UINT32 SaveFileConfig(APPLICATION_T *app, const WCHAR *file_config_path);

static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period);

static UINT32 StartLights(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ProcessLights(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 StopLights(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "Ambilight";

static const WCHAR g_str_app_name[] = L"Ambilight";
static const WCHAR g_str_mode[] = L"Mode:";
static const WCHAR g_str_e_mode[] = L"Mode";
static const WCHAR g_str_mode_ambilight[] = L"Ambilight";
static const WCHAR g_str_mode_flash50[] = L"Flash 50%";
static const WCHAR g_str_mode_flash100[] = L"Flash 100%";
static const WCHAR g_str_mode_rainbow[] = L"Rainbow";
static const WCHAR g_str_mode_random[] = L"Random";
static const WCHAR g_str_mode_stroboscope[] = L"Stroboscope";
static const WCHAR g_str_mode_strobo_flash[] = L"Strobo Flash";
static const WCHAR g_str_delay[] = L"Delay (in ms):";
static const WCHAR g_str_e_delay[] = L"Delay (in ms)";
static const WCHAR g_str_start_ligths[] = L"Start Lights!";
static const WCHAR g_str_stop_ligths[] = L"Stop Lights!";
static const WCHAR g_str_help[] = L"Help...";
static const WCHAR g_str_e_help[] = L"Help";
static const WCHAR g_str_about[] = L"About...";
static const WCHAR g_str_e_about[] = L"About";
static const WCHAR g_str_exit[] = L"Exit";
static const WCHAR g_str_changed[] = L"Changed:";
static const WCHAR g_str_warn_activated[] = L"Flash 100% activated, don't use this mode for too long!";
static const WCHAR g_str_warn_question[] = L"This can be dangerous to flashlight diode, are you sure?";
static const WCHAR g_str_help_content_p1[] = L"Bias lighting daemon program for Motorola phones.";
static const WCHAR g_str_about_content_p1[] = L"Version: 1.0";
static const WCHAR g_str_about_content_p2[] = L"\x00A9 EXL, 13-Sep-2023.";
static const WCHAR g_str_about_content_p3[] = L"https://github.com/EXL/P2kElfs/tree/master/Ambilight";
static const WCHAR g_str_about_content_p4[] = L"       "; /* HACK: gap */

static const UINT8 g_key_app_menu = KEY_SOFT_LEFT;
static const UINT8 g_key_app_exit = KEY_0;

static WCHAR g_config_file_path[FS_MAX_URI_NAME_LENGTH]; /* TODO: Can it be non-global? */

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
	 *  then change `app_instance->state` variable to `APP_DISPLAY_HIDE` value
	 *  then change `routing_stack` argument in `APP_InitAppData()` function to `APP_DISPLAY_HIDE` value
	 *  for pure daemon behavior with no GUI at start.
	 */
	/* { EV_GRANT_TOKEN, APP_HandleUITokenGranted }, */
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_REQUEST_LIST_ITEMS, HandleEventMenuRequestListItems },
	{ EV_DONE, HandleEventHide },
	{ EV_DIALOG_DONE, HandleEventHide },
	{ EV_SELECT, HandleEventSelect },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_edit_hdls[] = {
	{ EV_DATA, HandleEventEditData },
	{ EV_DONE, HandleEventEditDone },
	{ EV_DIALOG_DONE, HandleEventCancel },
	{ EV_CANCEL, HandleEventCancel },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_select_hdls[] = {
	{ EV_DONE, HandleEventSelectDone },
	{ EV_DIALOG_DONE, HandleEventCancel },
	{ EV_CANCEL, HandleEventCancel },
	{ EV_REQUEST_LIST_ITEMS, HandleEventSelectRequestListItems },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_popup_hdls[] = {
	{ EV_DONE, HandleEventCancel },
	{ EV_DIALOG_DONE, HandleEventCancel },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_warn_hdls[] = {
	{ EV_DONE, HandleEventCancel },
	{ EV_DIALOG_DONE, HandleEventCancel },
	{ EV_NO, HandleEventCancel },
	{ EV_YES, HandleEventYes },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls },
	{ APP_STATE_EDIT, HandleStateEnter, HandleStateExit, g_state_edit_hdls },
	{ APP_STATE_SELECT, HandleStateEnter, HandleStateExit, g_state_select_hdls },
	{ APP_STATE_POPUP, HandleStateEnter, HandleStateExit, g_state_popup_hdls },
	{ APP_STATE_WARNING, HandleStateEnter, HandleStateExit, g_state_warn_hdls },
	{ APP_STATE_VIEW, HandleStateEnter, HandleStateExit, g_state_popup_hdls } /* Same as popups. */
};

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	u_atou(elf_path_uri, g_config_file_path);
	g_config_file_path[u_strlen(g_config_file_path) - 3] = '\0';
	u_strcat(g_config_file_path, L"cfg");

	LdrStartApp(ev_code_base);

	return status;
}

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) HandleEventMain, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, APP_DISPLAY_HIDE, 0);

		InitResourses(app_instance->resources);
		app_instance->state = APP_DISPLAY_HIDE;
		app_instance->popup = APP_POPUP_CHANGED;
		app_instance->view = APP_VIEW_HELP;
		app_instance->menu_current_item_index = APP_MENU_ITEM_FIRST;
		app_instance->ms_key_press_start = 0LLU;
		app_instance->options.mode = APP_SELECT_ITEM_AMBILIGHT;
		app_instance->options.delay = 100; /* 100 ms. */

		if (DL_FsFFileExist(g_config_file_path)) {
			ReadFileConfig((APPLICATION_T *) app_instance, g_config_file_path);
		} else {
			SaveFileConfig((APPLICATION_T *) app_instance, g_config_file_path);
		}

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_INIT,
			g_state_table_hdls, HandleEventHide, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	FreeResourses(app_instance->resources);

	status |= StopLights(ev_st, app);

	status |= APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_NAME], RES_TYPE_STRING,
		(void *) g_str_app_name, (u_strlen(g_str_app_name) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_MODE], RES_TYPE_STRING,
		(void *) g_str_e_mode, (u_strlen(g_str_e_mode) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_DELAY], RES_TYPE_STRING,
		(void *) g_str_e_delay, (u_strlen(g_str_e_delay) + 1) * sizeof(WCHAR));

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
	APP_INSTANCE_T *app_instance;
	void *hdl;
	UINT32 routing_stack;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->state != display) {
		app_instance->state = display;
		hdl = (void *) ((app_instance->state == APP_DISPLAY_SHOW) ? APP_HandleEvent : APP_HandleEventPrepost);
		routing_stack = (app_instance->state == APP_DISPLAY_SHOW);
		status = APP_ChangeRoutingStack(app, ev_st, hdl, routing_stack, 0, 1, 1);
	}

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	APP_INSTANCE_T *app_instance;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	UINT32 starting_list_item;
	RESOURCE_ID edit_title;
	WCHAR edit_number[MAX_NUMBER_LENGTH + 1];

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;

	memclr(&content, sizeof(CONTENT_T));

	switch (app_state) {
		case APP_STATE_MAIN:
			starting_list_item = APP_MENU_ITEM_FIRST;
			dialog = UIS_CreateList(&port, 0, APP_MENU_ITEM_MAX, 0, &starting_list_item, 0, 2, NULL,
				app_instance->resources[APP_RESOURCE_STRING_NAME]);

			/* Insert cursor to proper position. */
			if (app_instance->menu_current_item_index != APP_MENU_ITEM_FIRST) {
				APP_UtilAddEvUISListChange(ev_st, app, 0, app_instance->menu_current_item_index + 1, APP_MENU_ITEM_MAX,
					FALSE, 2, NULL, NULL, NULL);
				UIS_HandleEvent(dialog, ev_st);
			}
			break;
		case APP_STATE_EDIT:
			edit_title = app_instance->resources[APP_RESOURCE_STRING_DELAY];
			u_ltou(app_instance->options.delay, edit_number);
			dialog = UIS_CreateCharacterEditor(&port, edit_number, 32 /* Numbers only. */, MAX_NUMBER_LENGTH,
				FALSE, NULL, edit_title);
			break;
		case APP_STATE_SELECT:
			starting_list_item = APP_MENU_ITEM_FIRST;
			dialog = UIS_CreateSelectionEditor(&port, 0, APP_SELECT_ITEM_MAX, app_instance->options.mode + 1,
				&starting_list_item, 0, NULL, app_instance->resources[APP_RESOURCE_STRING_MODE]);
			break;
		case APP_STATE_POPUP:
			switch (app_instance->popup) {
				default:
				case APP_POPUP_CHANGED:
					UIS_MakeContentFromString("MCq0NMCq1NMCq2", &content,
						g_str_changed, g_str_e_mode, GetTriggerOptionString(app_instance->options.mode));
					break;
				case APP_POPUP_RESETED:
					UIS_MakeContentFromString("MCq0", &content, g_str_warn_activated);
					break;
			}
			dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_OK);
			break;
		case APP_STATE_WARNING:
			UIS_MakeContentFromString("MCq0", &content, g_str_warn_question);
			dialog = UIS_CreateConfirmation(&port, &content);
			break;
		case APP_STATE_VIEW:
			switch (app_instance->view) {
				default:
				case APP_VIEW_HELP:
					UIS_MakeContentFromString("q0Nq1", &content, g_str_e_help,
						g_str_help_content_p1
					);
					break;
				case APP_VIEW_ABOUT:
					UIS_MakeContentFromString("q0NMCp1NMCq2NMCq3NMCq4NMCq5NMCq6", &content, g_str_app_name,
//						app_instance->resources[APP_RESOURCE_ICON_OVERCLOCK],
						NULL,
						g_str_about_content_p1, g_str_about_content_p2, g_str_about_content_p3,
						g_str_about_content_p4, g_str_about_content_p4);
					break;
			}
			dialog = UIS_CreateViewer(&port, &content, NULL);
			break;
		default:
			dialog = DialogType_None;
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	switch (app_state) {
		case APP_STATE_MAIN:
			SendMenuItemsToList(ev_st, app, 1, APP_MENU_ITEM_MAX);
			break;
		case APP_STATE_SELECT:
			SendSelectItemsToList(ev_st, app, 1, APP_SELECT_ITEM_MAX);
			break;
		default:
			break;
	}

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
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->state == APP_DISPLAY_SHOW) {
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
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	app_instance->menu_current_item_index = event->data.index - 1;

	switch (app_instance->menu_current_item_index) {
		case APP_MENU_ITEM_MODE:
			status |= APP_UtilChangeState(APP_STATE_SELECT, ev_st, app);
			break;
		case APP_MENU_ITEM_DELAY:
			status |= APP_UtilChangeState(APP_STATE_EDIT, ev_st, app);
			break;
		case APP_MENU_ITEM_START_LIGHTS:
			if (app_instance->options.mode == APP_SELECT_ITEM_FLASH_100) {
				status |= APP_UtilChangeState(APP_STATE_WARNING, ev_st, app);
			} else {
				status |= StartLights(ev_st, app);
			}
			break;
		case APP_MENU_ITEM_STOP_LIGHTS:
			status |= StopLights(ev_st, app);
			break;
		case APP_MENU_ITEM_HELP:
			app_instance->view = APP_VIEW_HELP;
			status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);
			break;
		case APP_MENU_ITEM_ABOUT:
			app_instance->view = APP_VIEW_ABOUT;
			status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);
			break;
		case APP_MENU_ITEM_EXIT:
			status |= APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
			break;
		default:
			break;
	}

	status |= APP_ConsumeEv(ev_st, app);

	return status;
}

static UINT32 HandleEventMenuRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	return HandleEventRequestListItems(ev_st, app, APP_STATE_MAIN);
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT8 key;

	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_app_menu || key == g_key_app_exit) {
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

	if (key == g_key_app_menu || key == g_key_app_exit) {
		/*
		 * Detect long key press between 500 ms (0.5 s) and 1500 ms (1.5 s) and ignore rest.
		 */
		ms_key_release_stop = (UINT32) (suPalTicksToMsec(suPalReadTime()) - app_instance->ms_key_press_start);
		if ((ms_key_release_stop >= KEY_LONG_PRESS_START_MS) && (ms_key_release_stop <= KEY_LONG_PRESS_STOP_MS)) {
			if (key == g_key_app_menu) {
				APP_ConsumeEv(ev_st, app);
				return HandleEventShow(ev_st, app);
			} else if (key == g_key_app_exit) {
#ifdef EXIT_BY_KEY
				APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
#endif
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
	} else if (timer_id == APP_TIMER_DO_LIGHTS) {
		ProcessLights(ev_st, app);
	}

	return RESULT_OK;
}

static UINT32 HandleEventEditData(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT32 data;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	if (event->attachment != NULL) {
		data = u_atol(event->attachment);
		if (data > MAX_NUMBER_VALUE) {
			data = MAX_NUMBER_VALUE;
		}
		switch (app_instance->menu_current_item_index) {
			case APP_MENU_ITEM_DELAY:
				app_instance->options.delay = data;
				break;
			default:
				break;
		}
	}

	status |= SaveFileConfig(app, g_config_file_path);
	status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);

	return status;
}

static UINT32 HandleEventEditDone(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	ADD_EVENT_DATA_T *event_data;

	status = RESULT_OK;

	/*
	 * EXL, 05-Jan-2023:
	 *  I don't know how, but this trick with uninitialised pointer just works.
	 *  Structure `ADD_EVENT_DATA_T &event_data;` on stack leads to a strange problem
	 *  when editing the list with deleting one symbol, only one character appears after editing.
	 */
	status |= AFW_AddEvEvD(ev_st, EV_REQUEST_DATA, event_data);
	status |= UIS_HandleEvent(app->dialog, ev_st);

	return status;
}

static UINT32 HandleEventSelectDone(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	app_instance->options.mode = event->data.index - 1;
	app_instance->popup = APP_POPUP_CHANGED;

	status |= SaveFileConfig(app, g_config_file_path);
	status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);

	return status;
}

static UINT32 HandleEventSelectRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	return HandleEventRequestListItems(ev_st, app, APP_STATE_SELECT);
}

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	app_instance->popup = APP_POPUP_RESETED;

	status |= StartLights(ev_st, app);

	status |= SaveFileConfig(app, g_config_file_path);
	status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);

	return status;
}

static UINT32 HandleEventRequestListItems(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_STATE_T app_state) {
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

	switch (app_state) {
		default:
		case APP_STATE_MAIN:
			status |= SendMenuItemsToList(ev_st, app, start, count);
			break;
		case APP_STATE_SELECT:
			SendSelectItemsToList(ev_st, app, start, count);
			break;
	}

	return status;
}

static UINT32 HandleEventCancel(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);

	return status;
}

static UINT32 SendMenuItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count) {
	UINT32 status;
	INT32 result;
	APP_INSTANCE_T *app_instance;
	UINT32 i;
	LIST_ENTRY_T *list;

	status = RESULT_OK;
	result = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	if (count == 0) {
		return RESULT_FAIL;
	}
	list = (LIST_ENTRY_T *) suAllocMem(sizeof(LIST_ENTRY_T) * APP_MENU_ITEM_MAX, &result);
	if (result != RESULT_OK) {
		return RESULT_FAIL;
	}

	for (i = 0; i < APP_MENU_ITEM_MAX; ++i) {
		memclr(&list[i], sizeof(LIST_ENTRY_T));
		list[i].editable = FALSE;
		list[i].content.static_entry.formatting = TRUE;
	}

	status |= UIS_MakeContentFromString("Mq0Sq1",
		&list[APP_MENU_ITEM_MODE].content.static_entry.text,
		g_str_mode, GetTriggerOptionString(app_instance->options.mode));
	status |= UIS_MakeContentFromString("Mq0Si1",
		&list[APP_MENU_ITEM_DELAY].content.static_entry.text,
		g_str_delay, app_instance->options.delay);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_START_LIGHTS].content.static_entry.text,
		g_str_start_ligths);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_STOP_LIGHTS].content.static_entry.text,
		g_str_stop_ligths);
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

static UINT32 SendSelectItemsToList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count) {
	UINT32 status;
	INT32 result;
	UINT32 i;
	LIST_ENTRY_T *list;

	status = RESULT_OK;
	result = RESULT_OK;

	if (count == 0) {
		return RESULT_FAIL;
	}
	list = (LIST_ENTRY_T *) suAllocMem(sizeof(LIST_ENTRY_T) * APP_SELECT_ITEM_MAX, &result);
	if (result != RESULT_OK) {
		return RESULT_FAIL;
	}

	for (i = 0; i < APP_SELECT_ITEM_MAX; ++i) {
		memclr(&list[i], sizeof(LIST_ENTRY_T));
		list[i].editable = FALSE;
		list[i].content.static_entry.formatting = TRUE;
	}

	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_AMBILIGHT].content.static_entry.text,
		g_str_mode_ambilight);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_FLASH_50].content.static_entry.text,
		g_str_mode_flash50);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_FLASH_100].content.static_entry.text,
		g_str_mode_flash100);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_RAINBOW].content.static_entry.text,
		g_str_mode_rainbow);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_RANDOM].content.static_entry.text,
		g_str_mode_random);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_STROBOSCOPE].content.static_entry.text,
		g_str_mode_stroboscope);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_STROBO_FLASH].content.static_entry.text,
		g_str_mode_strobo_flash);

	status |= APP_UtilAddEvUISListData(ev_st, app, 0, start, APP_SELECT_ITEM_MAX, FBF_LEAVE,
		sizeof(LIST_ENTRY_T) * APP_SELECT_ITEM_MAX, list);
	if (status != RESULT_FAIL) {
		UIS_HandleEvent(app->dialog, ev_st);
	}

	suFreeMem(list);

	return status;
}

static const WCHAR *GetTriggerOptionString(APP_SELECT_ITEM_T item) {
	switch (item) {
		default:
		case APP_SELECT_ITEM_AMBILIGHT:
			return g_str_mode_ambilight;
		case APP_SELECT_ITEM_FLASH_50:
			return g_str_mode_flash50;
		case APP_SELECT_ITEM_FLASH_100:
			return g_str_mode_flash100;
		case APP_SELECT_ITEM_RAINBOW:
			return g_str_mode_rainbow;
		case APP_SELECT_ITEM_RANDOM:
			return g_str_mode_random;
		case APP_SELECT_ITEM_STROBOSCOPE:
			return g_str_mode_stroboscope;
		case APP_SELECT_ITEM_STROBO_FLASH:
			return g_str_mode_strobo_flash;
	}
}

static UINT32 ReadFileConfig(APPLICATION_T *app, const WCHAR *file_config_path) {
	UINT32 readen;
	APP_INSTANCE_T *app_instance;
	FILE_HANDLE_T file_config;

	readen = 0;
	app_instance = (APP_INSTANCE_T *) app;

	file_config = DL_FsOpenFile(file_config_path, FILE_READ_MODE, 0);
	DL_FsReadFile(&app_instance->options, sizeof(APP_OPTIONS_T), 1, file_config, &readen);
	DL_FsCloseFile(file_config);

	return (readen == 0);
}

static UINT32 SaveFileConfig(APPLICATION_T *app, const WCHAR *file_config_path) {
	UINT32 written;
	APP_INSTANCE_T *app_instance;
	FILE_HANDLE_T file_config;

	written = 0;
	app_instance = (APP_INSTANCE_T *) app;

	file_config = DL_FsOpenFile(file_config_path, FILE_WRITE_MODE, 0);
	DL_FsWriteFile(&app_instance->options, sizeof(APP_OPTIONS_T), 1, file_config, &written);
	DL_FsCloseFile(file_config);

	return (written == 0);
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
		DL_ClkStartCyclicalTimer(&iface_data, period, APP_TIMER_DO_LIGHTS);
		status |= app_instance->timer_handle = iface_data.handle;
	}

	return status;
}

static UINT32 StartLights(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	status |= SetLoopTimer(app, app_instance->options.delay);

	return status;
}

static UINT32 ProcessLights(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	return status;
}

static UINT32 StopLights(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	status |= SetLoopTimer(app, 0);

	return status;
}
