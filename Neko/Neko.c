/*
 * About:
 *   Fun animated widget for desktop screen with skin support.
 *
 * Author:
 *   baat, EXL
 *
 * License:
 *   MIT
 *
 * Application type:
 *   Deamon + GUI + ATI + Widgets.
 */

#include <loader.h>
#include <apps.h>
#include <mem.h>
#include <uis.h>
#include <res_def.h>
#include <utilities.h>
#include <filesystem.h>
#include <dl.h>
#include <dal.h>
#include <ati.h>
#include <sms.h>

#include "icons/icon_neko_48x48.h"

#define MAX_DELAY_LENGTH            (6)
#define MIN_DELAY_VALUE             (200)
#define MAX_DELAY_VALUE             (10000)
#define MAX_PATH_LENGTH             (FS_MAX_URI_NAME_LENGTH / 3) /* 88*2 bytes is enough. */
#define MAX_NAME_LENGTH             (32)
#define KEY_LONG_PRESS_START_MS     (500)
#define KEY_LONG_PRESS_STOP_MS      (1500)

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
	APP_TIMER_EXIT = 0xE680,
	APP_TIMER_DO_WIDGET_LOOP
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_STRING_NAME,
	APP_RESOURCE_ICON_NEKO,
	APP_RESOURCE_STRING_SKIN,
	APP_RESOURCE_STRING_DELAY,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_SKIN = APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_DELAY,
	APP_MENU_ITEM_RESET,
	APP_MENU_ITEM_HELP,
	APP_MENU_ITEM_ABOUT,
	APP_MENU_ITEM_EXIT,
	APP_MENU_ITEM_MAX
} APP_MENU_ITEM_T;

typedef enum {
	APP_SELECT_ITEM_FIRST,
	APP_SELECT_ITEM_NEKO = APP_SELECT_ITEM_FIRST,
	APP_SELECT_ITEM_KITTY,
	APP_SELECT_ITEM_SHEEP,
	APP_SELECT_ITEM_PEPE,
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
	APP_SELECT_ITEM_T skin;
	UINT32 delay;
} APP_OPTIONS_T;

typedef struct {
	APPLICATION_T app;

	RESOURCE_ID resources[APP_RESOURCE_MAX];

	APP_DISPLAY_T state;
	APP_POPUP_T popup;
	APP_VIEW_T view;
	APP_MENU_ITEM_T menu_current_item_index;

	APP_OPTIONS_T options;

	UINT64 ms_key_press_start;
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
static UINT32 ResetSettingsToDefaultValues(APPLICATION_T *app);
static UINT32 ReadFileConfig(APPLICATION_T *app, const WCHAR *file_config_path);
static UINT32 SaveFileConfig(APPLICATION_T *app, const WCHAR *file_config_path);

static UINT32 SetLoopTimer(APPLICATION_T *app, UINT32 period);

static UINT32 StartWidget(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ProcessWidget(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 StopWidget(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static void AHG_Init(void);
static UINT32 paint(void);
static BOOL KeypadLock(void);
static BOOL WorkingTable(void);
static void InitBitmap(void);
static void DeinitBitmap(void);
static UINT32 add_call(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 del_call(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimeOutInactivities(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimeOutUserActivity(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static const char g_app_name[APP_NAME_LEN] = "Neko";

static const WCHAR g_str_app_name[] = L"Neko";
static const WCHAR g_str_skin[] = L"Skin:";
static const WCHAR g_str_e_skin[] = L"Skin";
static const WCHAR g_str_skin_neko[] = L"Neco-Arc";
static const WCHAR g_str_skin_kitty[] = L"Kitty";
static const WCHAR g_str_skin_sheep[] = L"Sheep";
static const WCHAR g_str_skin_pepe[] = L"PepeD";
static const WCHAR g_str_delay[] = L"Delay (in ms):";
static const WCHAR g_str_e_delay[] = L"Delay (in ms)";
static const WCHAR g_str_reset[] = L"Reset to default";
static const WCHAR g_str_help[] = L"Help...";
static const WCHAR g_str_e_help[] = L"Help";
static const WCHAR g_str_about[] = L"About...";
static const WCHAR g_str_e_about[] = L"About";
static const WCHAR g_str_exit[] = L"Exit";
static const WCHAR g_str_changed[] = L"Changed:";
static const WCHAR g_str_reseted[] = L"All settings have been reset to default values!";
static const WCHAR g_str_warn_reset[] = L"Do you want to reset settings to default?";
static const WCHAR g_str_help_content_p1[] =
	L"Fun animated widget for desktop screen with skin support.\n\n"
	L"Press and hold the right soft key for 2.5-3.5 seconds to bring up the main application menu.\n\n"
	L"The following skins are available for use:\n"
	L"1. Neco-Arc.\n"
	L"2. Kitty.\n"
	L"3. Sheep.\n"
	L"4. PepeD.\n\n"
	L"You can set refresh delay in 0.2-10 sec. of widget on desktop screen.\n";
static const WCHAR g_str_about_content_p1[] = L"Version: 1.0";
static const WCHAR g_str_about_content_p2[] = L"\x00A9 baat & EXL, 2010, 29-Oct-2023.";
static const WCHAR g_str_about_content_p3[] = L"https://github.com/EXL/P2kElfs/tree/master/Neko";
static const WCHAR g_str_about_content_p4[] = L"       "; /* HACK: gap */

static const UINT8 g_key_app_menu = KEY_SOFT_LEFT;
static const UINT8 g_key_app_exit = KEY_0;

static WCHAR g_config_file_path[FS_MAX_URI_NAME_LENGTH]; /* TODO: Can it be non-global? */

static WCHAR g_ani_files[APP_SELECT_ITEM_MAX][MAX_PATH_LENGTH];
static const WCHAR g_ani_file_names[APP_SELECT_ITEM_MAX][MAX_NAME_LENGTH] = {
	L"Neco.ani",
	L"Kitty.ani",
	L"Sheep.ani",
	L"Pepe.ani"
};

static BOOL g_user_activity = TRUE;
static BOOL g_ani_file_is_ok = FALSE;
static APP_SELECT_ITEM_T g_selected_skin = APP_SELECT_ITEM_NEKO;

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ EV_KEY_PRESS, HandleEventKeyPress },
	{ EV_KEY_RELEASE, HandleEventKeyRelease },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ EV_USER_ACTIVITY_TIMEOUT, HandleEventTimeOutUserActivity },
	{ EV_SCREENSAVER_TIMEOUT, HandleEventTimeOutInactivities },
	{ EV_DISPLAY_TIMEOUT, HandleEventTimeOutInactivities },
	/* { EV_BACKLIGHT_TIMEOUT, HandleEventTimeOutInactivities }, */
	{ EV_INACTIVITY_TIMEOUT, HandleEventTimeOutInactivities },
	{ EV_ADD_MISSED_CALL, add_call},
	{ EV_REMOVE_MISSED_CALL, del_call},
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
	UINT32 i;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	u_atou(elf_path_uri, g_config_file_path);
	g_config_file_path[u_strlen(g_config_file_path) - 3] = '\0';
	u_strcat(g_config_file_path, L"cfg");

	for (i = 0; i < APP_SELECT_ITEM_MAX; ++i) {
		u_atou(elf_path_uri, g_ani_files[i]);
		*(u_strrchr(g_ani_files[i], L'/') + 1) = '\0';
		u_strcat(g_ani_files[i], g_ani_file_names[i]);
	}

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

		ResetSettingsToDefaultValues((APPLICATION_T *) app_instance);

		if (DL_FsFFileExist(g_config_file_path)) {
			ReadFileConfig((APPLICATION_T *) app_instance, g_config_file_path);
		} else {
			SaveFileConfig((APPLICATION_T *) app_instance, g_config_file_path);
		}

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_INIT,
			g_state_table_hdls, HandleEventHide, g_app_name, 0);

		StartWidget(ev_st, (APPLICATION_T *) app_instance);
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

	status |= StopWidget(ev_st, app);
	status |= APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_NAME], RES_TYPE_STRING,
		(void *) g_str_app_name, (u_strlen(g_str_app_name) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_SKIN], RES_TYPE_STRING,
		(void *) g_str_e_skin, (u_strlen(g_str_e_skin) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_DELAY], RES_TYPE_STRING,
		(void *) g_str_e_delay, (u_strlen(g_str_e_delay) + 1) * sizeof(WCHAR));

	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_NEKO], RES_TYPE_GRAPHICS,
		(void *) neko_48x48_gif, sizeof(neko_48x48_gif));

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
	WCHAR edit_buffer[MAX_DELAY_LENGTH];

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
			u_ltou(app_instance->options.delay, edit_buffer);
			dialog = UIS_CreateCharacterEditor(&port, edit_buffer, 32 /* Numbers only. */, MAX_DELAY_VALUE,
				FALSE, NULL, edit_title);
			break;
		case APP_STATE_SELECT:
			starting_list_item = APP_MENU_ITEM_FIRST;
			dialog = UIS_CreateSelectionEditor(&port, 0, APP_SELECT_ITEM_MAX, app_instance->options.skin + 1,
				&starting_list_item, 0, NULL, app_instance->resources[APP_RESOURCE_STRING_SKIN]);
			break;
		case APP_STATE_POPUP:
			switch (app_instance->popup) {
				default:
				case APP_POPUP_CHANGED:
					UIS_MakeContentFromString("MCq0NMCq1NMCq2", &content,
						g_str_changed, g_str_e_skin, GetTriggerOptionString(app_instance->options.skin));
					break;
				case APP_POPUP_RESETED:
					UIS_MakeContentFromString("MCq0", &content, g_str_reseted);
					break;
			}
			dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_OK);
			break;
		case APP_STATE_WARNING:
			UIS_MakeContentFromString("MCq0", &content, g_str_warn_reset);
			dialog = UIS_CreateConfirmation(&port, &content);
			break;
		case APP_STATE_VIEW:
			switch (app_instance->view) {
				default:
				case APP_VIEW_HELP:
					UIS_MakeContentFromString("q0Nq1", &content, g_str_e_help, g_str_help_content_p1);
					break;
				case APP_VIEW_ABOUT:
					UIS_MakeContentFromString("q0NMCp1NMCq2NMCq3NMCq4NMCq5NMCq6", &content, g_str_app_name,
						app_instance->resources[APP_RESOURCE_ICON_NEKO],
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
		case APP_MENU_ITEM_SKIN:
			status |= APP_UtilChangeState(APP_STATE_SELECT, ev_st, app);
			break;
		case APP_MENU_ITEM_DELAY:
			status |= APP_UtilChangeState(APP_STATE_EDIT, ev_st, app);
			break;
		case APP_MENU_ITEM_RESET:
			status |= APP_UtilChangeState(APP_STATE_WARNING, ev_st, app);
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
				if (!KeypadLock()) {
					APP_ConsumeEv(ev_st, app);
					return HandleEventShow(ev_st, app);
				}
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
	} else if (timer_id == APP_TIMER_DO_WIDGET_LOOP) {
		ProcessWidget(ev_st, app);
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
		if (data > MAX_DELAY_VALUE) {
			data = MAX_DELAY_VALUE;
		}
		if (data < MIN_DELAY_VALUE) {
			data = MIN_DELAY_VALUE;
		}
		app_instance->options.delay = data;
	}

	StartWidget(ev_st, app);

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

	app_instance->options.skin = event->data.index - 1;
	app_instance->popup = APP_POPUP_CHANGED;

	g_selected_skin = app_instance->options.skin;
	g_ani_file_is_ok = DL_FsFFileExist(g_ani_files[g_selected_skin]);

	StartWidget(ev_st, app);

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
	ResetSettingsToDefaultValues(app);

	StartWidget(ev_st, app);

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
		&list[APP_MENU_ITEM_SKIN].content.static_entry.text,
		g_str_skin, GetTriggerOptionString(app_instance->options.skin));
	status |= UIS_MakeContentFromString("Mq0Si1",
		&list[APP_MENU_ITEM_DELAY].content.static_entry.text,
		g_str_delay, app_instance->options.delay);
	status |= UIS_MakeContentFromString("Mq0",
		&list[APP_MENU_ITEM_RESET].content.static_entry.text,
		g_str_reset);
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
		&list[APP_SELECT_ITEM_NEKO].content.static_entry.text,
		g_str_skin_neko);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_KITTY].content.static_entry.text,
		g_str_skin_kitty);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_SHEEP].content.static_entry.text,
		g_str_skin_sheep);
	status |= UIS_MakeContentFromString("q0",
		&list[APP_SELECT_ITEM_PEPE].content.static_entry.text,
		g_str_skin_pepe);

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
		case APP_SELECT_ITEM_NEKO:
			return g_str_skin_neko;
		case APP_SELECT_ITEM_KITTY:
			return g_str_skin_kitty;
		case APP_SELECT_ITEM_SHEEP:
			return g_str_skin_sheep;
		case APP_SELECT_ITEM_PEPE:
			return g_str_skin_pepe;
	}
}

static UINT32 ResetSettingsToDefaultValues(APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	app_instance->options.skin = APP_SELECT_ITEM_NEKO;
	app_instance->options.delay = 200; /* 200 ms. */

	g_ani_file_is_ok = DL_FsFFileExist(g_ani_files[g_selected_skin]);

	return status;
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

	g_selected_skin = app_instance->options.skin;
	g_ani_file_is_ok = DL_FsFFileExist(g_ani_files[g_selected_skin]);

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
		DL_ClkStartCyclicalTimer(&iface_data, period, APP_TIMER_DO_WIDGET_LOOP);
		status |= app_instance->timer_handle = iface_data.handle;
	}

	return status;
}

static UINT32 StartWidget(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	status |= StopWidget(ev_st, app);

	InitBitmap();
	randomize();
	AHG_Init();

	status |= SetLoopTimer(app, app_instance->options.delay);

	return status;
}

static UINT32 ProcessWidget(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	if (WorkingTable() && !KeypadLock() && g_user_activity && g_ani_file_is_ok) {
		paint();
		D("%s\n", "Paint!");
	}

	return status;
}

static UINT32 StopWidget(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	status |= SetLoopTimer(app, 0);
	status |= APP_UtilStopTimer(app);

	DeinitBitmap();

	return status;
}

static UINT8 RECTSURF_W = DISPLAY_WIDTH;
static UINT8 RECTSURF_H = DISPLAY_HEIGHT;
static UINT8 x_t = 100;
static UINT8 y_t = 157;
static BOOL is_cSTN_128p_DISPLAY = FALSE;

#define a(b, c) ((UINT8)  ((b == c) ? (1) : (0)))
#define b(t1, t2, t3, t4) ((UINT8)((t1 << 3) + (t2 << 2) + (t3 << 1) + t4))

static UINT8 two2bin(UINT8 b1, UINT8 b2, UINT8 b3, UINT8 b4, UINT8 c, UINT8 n);
static void FreeBin(void);
static void OpenBin(UINT8 l);
static void WriteBin(UINT8 x, UINT8 y, UINT8 u);
static void AHG_Flush(void);

static BOOL call(void);
static BOOL sms(void);
static BOOL sleep(void);
static void copy(UINT8 x, UINT8 y, UINT8 bk);
static void _time(void);

typedef struct {
	UINT8 r, g, b;
} TColor;

typedef struct {
	INT8 x, y;
} TPoint;

static UINT8 ccall = 0, _count[1] = {0}, s_d = 0, pred_d = 0;
static INT8 d = 3;
static INT8 x = 4;
static TColor *color = NULL;
static char *arr = NULL, *bin = NULL;
static FILE file = NULL;
static UINT32 r = 0;
static BOOL new = 1;
static TPoint s[1] = {0};
static AHIBITMAP_T bm, bmp;
static AHIDEVCONTEXT_T dCtx = 0;
static AHISURFACE_T sDraw = 0;
static AHISURFACE_T sDisp = 0;
static AHIPOINT_T pSurf = {0, 0};
static AHIRECT_T rectSurf = {0, 0, 0, 0};

static UINT8 two2bin(UINT8 b1, UINT8 b2, UINT8 b3, UINT8 b4, UINT8 c, UINT8 n) {
	UINT8 t1, t2, t3, t4, t5, t6, t7, t8;
	t1 = (b1 >> 4);
	t2 = b1 - (t1 << 4);
	t3 = (b2 >> 4);
	t4 = b2 - (t3 << 4);
	t5 = (b3 >> 4);
	t6 = b3 - (t5 << 4);
	t7 = (b4 >> 4);
	t8 = b4 - (t7 << 4);
	return (n == 0) ?
		((b(a(t1, c), a(t2, c), a(t3, c), a(t4, c)) << 4) + b(a(t5, c), a(t6, c), a(t7, c), a(t8, c))) :
		((b(a(t8, c), a(t7, c), a(t6, c), a(t5, c)) << 4) + b(a(t4, c), a(t3, c), a(t2, c), a(t1, c)));
}

static void FreeBin(void) {
	if (arr) {
		suFreeMem(arr);
		arr = NULL;
	}
	if (bin) {
		suFreeMem(bin);
		bin = NULL;
	}
	if (color) {
		suFreeMem(color);
		color = NULL;
	}
}

static void OpenBin(UINT8 l) {
	file = DL_FsOpenFile(g_ani_files[g_selected_skin], FILE_READ_MODE, 0);
	if (_count[0] < 1) {
		FreeBin();
		DL_FsReadFile(_count, 1, 1, file, &r);
		color = malloc(3 * _count[0]);
		DL_FsReadFile(color, 1, 3 * _count[0], file, &r);
		DL_FsReadFile(s, 1, 2, file, &r);
		arr = malloc(s[0].y * s[0].x);
		bin = malloc(s[0].y * s[0].x / 4);
	}
	DL_FsFSeekFile(file, 3 * (_count[0] + 1) + l * s[0].x * s[0].y, 0);
	DL_FsReadFile(arr, 1, s[0].y * s[0].x, file, &r);
	DL_FsCloseFile(file);
	bmp.stride = s[0].x >> 2;
	bmp.width = (UINT32)(s[0].x * 2);
	bmp.height = (UINT32)(s[0].y);
}

static void WriteBin(UINT8 x, UINT8 y, UINT8 u) {
	UINT16 i, i2;
	UINT8 c, _x, _y;
	AHIPOINT_T p;
	AHIRECT_T r;
	p.x = 0;
	p.y = 0;
	r.x1 = x;
	r.y1 = y;
	r.x2 = x + bmp.width;
	r.y2 = y + bmp.height;
	pred_d = u;
	AhiDrawSurfDstSet(dCtx, sDraw, 0);
	for (c = 0; c < _count[0]; c++) {
		for (_y = 0; _y < s[0].y; _y++) {
			for (_x = 0; _x < (s[0].x) >> 2; _x++) {
				if (u == 0) {
					i = _y * (s[0].x >> 2) + _x;
					bin[i] = two2bin(arr[i << 2], arr[(i << 2) + 1], arr[(i << 2) + 2], arr[(i << 2) + 3], c, 0);
				} else if (u == 1) {
					i = _y * (s[0].x >> 2) + _x;
					i2 = (_y * s[0].x >> 2) + ((s[0].x >> 2) - _x - 1);
					bin[i] = two2bin(arr[i2 << 2], arr[(i2 << 2) + 1], arr[(i2 << 2) + 2], arr[(i2 << 2) + 3], c, 1);
				}
			}
		}
		bmp.image = (void *) (bin);
		if ((ATI_565RGB(color[c].r, color[c].g, color[c].b)) != (ATI_565RGB(0, 0, 255))) {
			AhiDrawFgColorSet(dCtx, ATI_565RGB(color[c].r, color[c].g, color[c].b));
			AhiDrawBitmapBlt(dCtx, &r, &p, &bmp, NULL, 1);
		}
	}
}

static void AHG_Init(void) {
	AHIDISPMODE_T mode;
	dCtx = DAL_GetDeviceContext(DISPLAY_MAIN);
	sDraw = DAL_GetDrawingSurface(DISPLAY_MAIN);

	AhiDispModeGet(dCtx, &mode);
	is_cSTN_128p_DISPLAY = (mode.size.x == 128);
	if (is_cSTN_128p_DISPLAY) {
		RECTSURF_W = 128;
		RECTSURF_H = 160;
		x_t = 60;
		y_t = 102;
	} else {
		RECTSURF_W = DISPLAY_WIDTH;
		RECTSURF_H = DISPLAY_HEIGHT;
		x_t = 100;
		y_t = 158;
	}
	rectSurf.x1 = 0;
	rectSurf.y1 = 0;
	rectSurf.x2 = RECTSURF_W;
	rectSurf.y2 = RECTSURF_H;

	AhiDispSurfGet(dCtx, &sDisp);
	AhiDrawSurfDstSet(dCtx, sDisp, 0);
	AhiDrawSurfSrcSet(dCtx, sDraw, 0);
	AhiDrawClipSrcSet(dCtx, NULL);
	AhiDrawClipDstSet(dCtx, NULL);
}

static void AHG_Flush(void) {
	AhiDrawSurfSrcSet(dCtx, sDraw, 0);
	AhiDrawSurfDstSet(dCtx, sDisp, 0);
	AhiDrawClipSrcSet(dCtx, NULL);
	AhiDrawClipDstSet(dCtx, NULL);
	AhiDrawRopSet(dCtx, AHIROP3(AHIROP_SRCCOPY));
	AhiDrawBitBlt(dCtx, &rectSurf, &pSurf);

	/* TODO: Is this even necessary? */
	/* AhiDispWaitVBlank(dCtx, 0); */

	if (is_cSTN_128p_DISPLAY) {
		AHIUPDATEPARAMS_T update_params;
		update_params.size = sizeof(AHIUPDATEPARAMS_T);
		update_params.sync = FALSE;
		update_params.rect.x1 = 0;
		update_params.rect.y1 = 0;
		update_params.rect.x2 = 0 + RECTSURF_W;
		update_params.rect.y2 = 0 + RECTSURF_H;
		AhiDispUpdate(dCtx, &update_params);
	}
}

static BOOL KeypadLock(void) {
	BOOL keypad_statate;
	DL_DbFeatureGetCurrentState(*KEYPAD_STATE, &keypad_statate);
	return keypad_statate;
}

static BOOL WorkingTable(void) {
	UINT8 res;
	UIS_GetActiveDialogType(&res);
	return (res == DialogType_Homescreen) ? (true) : (false); /* Desktop window. */
}

static BOOL sms(void) {
	UINT16 t = 0;
	MsgUtilGetUnreadMsgsInAllFolders(&t);
	return (t > 0) ? (true) : (false);
}

static BOOL call(void) {
	return (ccall > 0) ? (true) : (false);
}

static BOOL sleep(void) {
	CLK_TIME_T time;
	DL_ClkGetTime(&time);
	return (time.hour > 22 || time.hour < 6) ? (true) : (false);
}

static void copy(UINT8 x, UINT8 y, UINT8 bk) {
	AHIPOINT_T pt;
	AHIRECT_T rt;
	if (bk == 1) {
		pt.x = x;
		pt.y = y;
		rt.x1 = 0;
		rt.y1 = 0;
		rt.x2 = 40;
		rt.y2 = 40;
		AhiSurfCopy(dCtx, sDraw, &bm, &rt, &pt, NULL, AHIFLAG_COPYFROM);
	} else {
		pt.x = 0;
		pt.y = 0;
		rt.x1 = x;
		rt.y1 = y;
		rt.x2 = x + 40;
		rt.y2 = y + 40;
		AhiSurfCopy(dCtx, sDraw, &bm, &rt, &pt, NULL, AHIFLAG_COPYTO);
	}
}

static void _time(void) {
	UINT8 count[12] = {1, 3, 3, 3, 3, 3, 3, 5, 3, 3, 5, 1},
	      start[12] = {0, 1, 1, 4, 4, 6, 9, 12, 9, 17, 20, 0},
	          u[12] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
	INT8 px[12] = {0, -2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	if (!bm.image) {
		bm.image = (void *)malloc(40 * 40 * 2);
		copy((UINT8)(x), y_t, 1);
	} else {
		copy((UINT8)(x), y_t, 0);
	}
	if (sms()) {
		d = 10;
	} else if (call()) {
		d = 9;
	} else if (sleep()) {
		d = 0;
	}
	if (new) {
		if (s_d == count[d] - 1) {
			new = false;
			s_d = 0;
		} else {
			s_d++;
			x = x + count[d] * px[d];
		}
	} else {
		if (count[d] * px[d] + x < 5) {
			d = 2;
		} else if (count[d] * px[d] + x > x_t) {
			d = 1;
		} else {
			d = random(20);
			if (d > 8) {
				while (d > 2) {
					d -= 5;
				}
			}
			if (d < 0) {
				d = 0;
			}
		}
		new = true;
	}
	if (sms()) {
		d = 10;
	} else if (call()) {
		d = 9;
	} else if (sleep()) {
		d = 0;
	}
	copy((UINT8)(x), y_t, 1);
	OpenBin(start[d] + s_d);
	WriteBin((UINT8)(x), y_t, (d != 0) ? (u[d]) : (pred_d));
}

static void InitBitmap(void) {
	bm.format = AHIFMT_16BPP_565;
	bm.width = 40;
	bm.height = 40;
	bm.stride = 40 * 2;
	bmp.format = AHIFMT_1BPP;
	bm.image = NULL;
	arr = NULL;
	bin = NULL;
	color = NULL;
}

static void DeinitBitmap(void) {
	if (bm.image) {
		suFreeMem(bm.image);
		bm.image = NULL;
	}
	_count[0] = 0;
}

static UINT32 paint(void) {
	_time();
	AHG_Flush();
	return RESULT_OK;
}

static UINT32 add_call(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	ccall = 1;
	return RESULT_OK;
}

static UINT32 del_call(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	ccall = 0;
	return RESULT_OK;
}

/*
 *  APP_EV_INACTIVITYTIMERMANAGER_UIS_TIMEOUT, /7EC
 *  APP_EV_INACTIVITYTIMERMANAGER_UIS_ACTIVITY, /7ED
 *  APP_EV_USER_ACTIVITY, / 7EE
 *  APP_EV_ACTIVATE_BACKLIGHT, / 7EF
 *
 *  APP_EV_SS_TIMEOUT, / 7F0
 *  APP_EV_DISPLAY_TIMEOUT, / 7F1
 *  APP_EV_BACKLIGHT_TIMEOUT, / 7F2
 *  APP_EV_INACTIVITY_TIMEOUT, / 7F3
 *
 * Screensaver enter.
 * e7ec:1364: Debug Line!
 * e7f0:1392: Debug Line!
 * e7f2:1406: Debug Line!
 * e7ed:1371: Debug Line!
 *
 * Inactivity screen with time enter.
 * e7f1:1399: Debug Line!
 * e7f2:1406: Debug Line!
 * e7ed:1371: Debug Line!
 * DATAFLOW ERROR: 20, 64
 *
 * Inactivity screen with time exit.
 * e7ed:1371: Debug Line!
 * e7ee:1378: Debug Line!
 */

/*
 * APP_EV_USER_ACTIVITY, / 7EE
 */
static UINT32 HandleEventTimeOutUserActivity(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	g_user_activity = TRUE;

	P();

	return RESULT_OK;
}

/*
 * APP_EV_SS_TIMEOUT, / 7F0
 * APP_EV_DISPLAY_TIMEOUT, / 7F1
 * APP_EV_BACKLIGHT_TIMEOUT, / 7F2
 * APP_EV_INACTIVITY_TIMEOUT, / 7F3
 */
static UINT32 HandleEventTimeOutInactivities(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	g_user_activity = FALSE;

	P();

	return RESULT_OK;
}
