/*
 * About:
 *   A simple "ElfBox" ELF-applications launcher.
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
#include <uis.h>
#include <mem.h>
#include <utilities.h>
#include <dl.h>
#include <filesystem.h>

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0x0001,
	APP_TIMER_EXIT_FAST
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_NAME,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_VIEW_HELP,
	APP_VIEW_ABOUT
} APP_VIEW_T;

typedef struct {
	APPLICATION_T app; /* Must be first. */

	RESOURCE_ID resources[APP_RESOURCE_MAX];
	APP_VIEW_T view;
	UINT32 menu_current_item_index;
	BOOL flag_from_select;
} APP_INSTANCE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 InitResourses(RESOURCE_ID *resources);
static UINT32 FreeResourses(RESOURCE_ID *resources);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventBack(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static LIST_ENTRY_T *CreateList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count);

static const char g_app_name[APP_NAME_LEN] = "ElfBox";
static const UINT32 g_ev_code_base = 0x00000FF1;

static const WCHAR g_str_app_name[] = L"ElfBox";

static const WCHAR g_str_menu_help[] = L"Help...";
static const WCHAR g_str_menu_about[] = L"About...";
static const WCHAR g_str_view_help[] = L"Help";
static const WCHAR g_str_help_content_p1[] = L"A simple ELF-applications launcher.";
static const WCHAR g_str_about_content_p1[] = L"Version: 1.0";
static const WCHAR g_str_about_content_p2[] = L"\x00A9 EXL, 02-Jul-2023.";
static const WCHAR g_str_about_content_p3[] = L"https://github.com/EXL/P2kElfs/tree/master/ElfBox";

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
	{ EV_SELECT, HandleEventSelect },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_popup_hdls[] = {
	{ EV_DONE, HandleEventBack },
	{ EV_DIALOG_DONE, HandleEventBack },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, NULL, g_state_main_hdls },
	{ APP_STATE_VIEW, HandleStateEnter, HandleStateExit, g_state_popup_hdls }
};

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;

	status = APP_Register(&g_ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	return status;
}

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, 1, 0);

		InitResourses(app_instance->resources);

		app_instance->menu_current_item_index = 0;
		app_instance->view = APP_VIEW_ABOUT;
		app_instance->flag_from_select = FALSE;

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);
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

	status |= APP_Exit(ev_st, app, 0);

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_NAME], RES_TYPE_STRING,
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

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	APP_INSTANCE_T *app_instance;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	LIST_ENTRY_T *list;

	if (state != ENTER_STATE_ENTER) {
		if (app->state != APP_STATE_MAIN) {
			return RESULT_OK;
		}
	}

	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;
	dialog = DialogType_None;

	memclr(&content, sizeof(CONTENT_T));

	switch (app_state) {
		case APP_STATE_MAIN:
			list = CreateList(ev_st, app, 1, 2);
			if (list != NULL) {
				dialog = UIS_CreateStaticList(&port, 0, 2, 0, list, FALSE, 2, NULL,
					app_instance->resources[APP_RESOURCE_NAME]);
				suFreeMem(list);

				/* Insert cursor to proper position. */
				if (app_instance->flag_from_select) {
					if (app_instance->menu_current_item_index != 0) {
						APP_UtilAddEvChangeListPosition(ev_st, app, app_instance->menu_current_item_index + 1,
							NULL, NULL, NULL);

						UIS_HandleEvent(dialog, ev_st);
					}
					app_instance->flag_from_select = FALSE;
				}
			}
			break;
		case APP_STATE_VIEW:
			switch (app_instance->view) {
				default:
				case APP_VIEW_HELP:
					UIS_MakeContentFromString("q0Nq1", &content, g_str_view_help,
						g_str_help_content_p1);
					break;
				case APP_VIEW_ABOUT:
					UIS_MakeContentFromString("q0NMCq1NMCq2NMCq3", &content, g_str_app_name,
						g_str_about_content_p1, g_str_about_content_p2, g_str_about_content_p3);
					break;
			}
			dialog = UIS_CreateViewer(&port, &content, NULL);
			break;
		default:
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

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	switch (timer_id) {
		case APP_TIMER_EXIT:
		case APP_TIMER_EXIT_FAST:
			/* Play an exit sound using quiet speaker. */
			DL_AudPlayTone(0x00,  0xFF);
			return ApplicationStop(ev_st, app);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	app_instance->flag_from_select = TRUE;
	app_instance->menu_current_item_index = event->data.index - 1;

	switch (app_instance->menu_current_item_index) {
		case 0:
			app_instance->view = APP_VIEW_HELP;
			status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);
			break;
		case 1:
			app_instance->view = APP_VIEW_ABOUT;
			status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);
			break;
		default:
			break;
	}

	return status;
}

static UINT32 HandleEventBack(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);

	return status;
}

static LIST_ENTRY_T *CreateList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count) {
	UINT32 status;
	INT32 result;
	UINT32 i;
	LIST_ENTRY_T *list_elements;

	status = RESULT_OK;
	result = RESULT_OK;

	if (count == 0) {
		return NULL;
	}
	list_elements = (LIST_ENTRY_T *) suAllocMem(sizeof(LIST_ENTRY_T) * count, &result);
	if (result != RESULT_OK) {
		return NULL;
	}

	for (i = 0; i < count; ++i) {
		memclr(&list_elements[i], sizeof(LIST_ENTRY_T));
		list_elements[i].editable = FALSE;
		list_elements[i].content.static_entry.formatting = TRUE;
	}

	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[0].content.static_entry.text,
		g_str_menu_help);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[1].content.static_entry.text,
		g_str_menu_about);

	if (status != RESULT_OK) {
		suFreeMem(list_elements);
		return NULL;
	}

	return list_elements;
}
