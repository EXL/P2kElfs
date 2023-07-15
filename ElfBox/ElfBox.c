/*
 * About:
 *   A simple ELF-application launcher.
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
#include <resources.h>

#include "icons/icon_elf_15x15.h"
#include "icons/icon_file_15x15.h"
#include "icons/icon_disk_15x15.h"
#include "icons/icon_folder_15x15.h"

#define TIMER_FAST_TRIGGER_MS             (1)
#define MAX_VOLUMES_COUNT                 (4)
#define MAX_FILENAME_SYMS                 (64)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_POPUP,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0x0001,
	APP_TIMER_EXIT_FAST,
	APP_TIMER_TO_MAIN_VIEW
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_NAME,
	APP_RESOURCE_MENU,
	APP_RESOURCE_HELP,
	APP_RESOURCE_ABOUT,
	APP_RESOURCE_EXIT,
	APP_RESOURCE_ICON_DISK,
	APP_RESOURCE_ICON_FOLDER,
	APP_RESOURCE_ICON_FILE,
	APP_RESOURCE_ICON_ELF,
	APP_RESOURCE_ACTION_BACK,
	APP_RESOURCE_ACTION_RUN,
	APP_RESOURCE_ACTION_HELP,
	APP_RESOURCE_ACTION_ABOUT,
	APP_RESOURCE_ACTION_EXIT,
	APP_RESOURCE_LIST_DESCRIPTION,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_POPUP_WAIT,
	APP_POPUP_NOT_ELF
} APP_POPUP_T;

typedef enum {
	APP_VIEW_HELP,
	APP_VIEW_ABOUT
} APP_VIEW_T;

typedef enum {
	FS_VOLUME,
	FS_FOLDER,
	FS_FOLDER_UP,
	FS_FILE,
	FS_ELF
} FS_ENTITY_T;

typedef struct {
	WCHAR name[MAX_FILENAME_SYMS + 1];
	FS_ENTITY_T type;
} FS_OBJECT_T;

typedef struct {
	BOOL root;
	UINT16 count;
	FS_OBJECT_T *list;
} FS_T;

typedef struct {
	APPLICATION_T app; /* Must be first. */

	RESOURCE_ID resources[APP_RESOURCE_MAX];
	APP_POPUP_T popup;
	APP_VIEW_T view;
	UINT32 menu_current_item_index;
	BOOL flag_from_select;

	WCHAR current_path[FS_MAX_PATH_NAME_LENGTH + 1];
	WCHAR current_title[FS_MAX_PATH_NAME_LENGTH + 1];
	FS_T fs;
} APP_INSTANCE_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */

static UINT32 LdrInitEventHandlersTbl(EVENT_HANDLER_ENTRY_T *tbl, UINT32 *base);
static UINT32 LdrFindEventHandlerTbl(EVENT_HANDLER_ENTRY_T *tbl, EVENT_HANDLER_T *hfn);

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 InitResourses(RESOURCE_ID *resources);
static UINT32 FreeResourses(RESOURCE_ID *resources);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventBackDir(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventHelp(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventAbout(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventExit(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventBack(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventSearchCompleted(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static LIST_ENTRY_T *CreateList(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 UpdateList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *directory, const WCHAR *filter);
static UINT32 UpdateVolumeList(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 UpdateFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *search_string);
static UINT32 FillFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, FS_SEARCH_COMPLETED_INDEX_T *search_index);
static int file_comparate(const void *obj_1, const void *obj_2);
static UINT32 SortFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 CreateSearchString(WCHAR *search_string, const WCHAR *search_directory, const WCHAR *search_pattern);
static UINT32 AddUriFileHeader(WCHAR *out_str, const WCHAR *in_str);
static const WCHAR *GetFileNameFromPath(const WCHAR *path);
static BOOL IsElfFile(const WCHAR *path);
static const WCHAR *DirUp(WCHAR *path);
static const WCHAR *DirDown(WCHAR *path, const WCHAR *directory_name);
static UINT32 RunElfApplication(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *elf_path);

static const char g_app_name[APP_NAME_LEN] = "ElfBox";

static const UINT32 g_ev_code_base = 0x00000FF1;

static const WCHAR g_str_app_name[] = L"ElfBox Launcher";
static const WCHAR g_str_app_menu[] = L"ElfBox Menu";
static const WCHAR g_str_menu_help[] = L"Help...";
static const WCHAR g_str_menu_about[] = L"About...";
static const WCHAR g_str_menu_exit[] = L"Exit";
static const WCHAR g_str_view_help[] = L"Help";
static const WCHAR g_str_popup_please_wait[] = L"Please wait!";
static const WCHAR g_str_popup_sorting[] = L"Sorting files and folders";
static const WCHAR g_str_popup_cannot_run[] = L"Cannot Run!";
static const WCHAR g_str_popup_not_elf[] = L"It is not an ELF file.";
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
	{ EV_DONE, HandleEventBackDir },
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_SELECT, HandleEventSelect },
	{ STATE_HANDLERS_RESERVED, HandleEventHelp },
	{ STATE_HANDLERS_RESERVED, HandleEventAbout },
	{ STATE_HANDLERS_RESERVED, HandleEventExit },
	{ STATE_HANDLERS_END, NULL }
};

static EVENT_HANDLER_ENTRY_T g_state_popup_hdls[] = {
	{ EV_DONE, HandleEventBack },
	{ EV_DIALOG_DONE, HandleEventBack },
	{ EV_FILE_SEARCH_COMPLETED, HandleEventSearchCompleted },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, NULL, g_state_main_hdls },
	{ APP_STATE_POPUP, HandleStateEnter, HandleStateExit, g_state_popup_hdls },
	{ APP_STATE_VIEW, HandleStateEnter, HandleStateExit, g_state_popup_hdls } /* Same as popups. */
};

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;

	D("Reserve 'ev_code' app event: %d 0x%X.\n", ev_code, ev_code);
	LdrInitEventHandlersTbl(g_state_main_hdls, &ev_code);

	status = APP_Register(&g_ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	return status;
}

static UINT32 LdrInitEventHandlersTbl(EVENT_HANDLER_ENTRY_T *tbl, UINT32 *base) {
	UINT32 i = 0;
	while (tbl[i].code != STATE_HANDLERS_END) {
		if (tbl[i].code == STATE_HANDLERS_RESERVED) {
			tbl[i].code = (*base)++;
			D("Added my own ev_code: %d 0x%X.\n", tbl[i].code, tbl[i].code);
		}
		i++;
	}
	return *base;
}

static UINT32 LdrFindEventHandlerTbl(EVENT_HANDLER_ENTRY_T *tbl, EVENT_HANDLER_T *hfn) {
	UINT32 i = 0;
	while (tbl[i].code != STATE_HANDLERS_END) {
		if (tbl[i].hfunc == hfn) {
			return tbl[i].code;
		}
		i++;
	}
	return 0;
}

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		appi = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, 1, 0);

		InitResourses(appi->resources);

		appi->menu_current_item_index = 0;
		appi->popup = APP_POPUP_WAIT;
		appi->view = APP_VIEW_ABOUT;
		appi->flag_from_select = FALSE;

		u_strcpy(appi->current_path, L"/a");
		u_strcpy(appi->current_title, g_str_app_name);
		appi->fs.root = FALSE;
		appi->fs.count = 0;
		appi->fs.list = NULL;

		status = APP_Start(ev_st, &appi->app, APP_STATE_POPUP,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	FreeResourses(appi->resources);

	status |= APP_Exit(ev_st, app, 0);

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;
	RES_ACTION_LIST_ITEM_T action;
	UIS_LIST_RESOURCE_CONTENTS_T list_description;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_NAME], RES_TYPE_STRING,
		(void *) g_str_app_name, (u_strlen(g_str_app_name) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_MENU], RES_TYPE_STRING,
		(void *) g_str_app_menu, (u_strlen(g_str_app_menu) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_HELP], RES_TYPE_STRING,
		(void *) g_str_menu_help, (u_strlen(g_str_menu_help) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ABOUT], RES_TYPE_STRING,
		(void *) g_str_menu_about, (u_strlen(g_str_menu_about) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_EXIT], RES_TYPE_STRING,
		(void *) g_str_menu_exit, (u_strlen(g_str_menu_exit) + 1) * sizeof(WCHAR));

	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_DISK], RES_TYPE_GRAPHICS,
		(void *) disk_15x15_gif, sizeof(disk_15x15_gif));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_FOLDER], RES_TYPE_GRAPHICS,
		(void *) folder_15x15_gif, sizeof(folder_15x15_gif));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_FILE], RES_TYPE_GRAPHICS,
		(void *) file_15x15_gif, sizeof(file_15x15_gif));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_ELF], RES_TYPE_GRAPHICS,
		(void *) elf_15x15_gif, sizeof(elf_15x15_gif));

	action.softkey_label = resources[APP_RESOURCE_HELP];
	action.list_label = resources[APP_RESOURCE_HELP];
	action.softkey_priority = 0;
	action.list_priority = APP_RESOURCE_MAX - APP_RESOURCE_HELP;
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ACTION_HELP], RES_TYPE_ACTION, &action, sizeof(action));

	action.softkey_label = resources[APP_RESOURCE_ABOUT];
	action.list_label = resources[APP_RESOURCE_ABOUT];
	action.softkey_priority = 0;
	action.list_priority = APP_RESOURCE_MAX - APP_RESOURCE_ABOUT;
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ACTION_ABOUT], RES_TYPE_ACTION, &action, sizeof(action));

	action.softkey_label = resources[APP_RESOURCE_EXIT];
	action.list_label = resources[APP_RESOURCE_EXIT];
	action.softkey_priority = 0;
	action.list_priority = APP_RESOURCE_MAX - APP_RESOURCE_EXIT;
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ACTION_EXIT], RES_TYPE_ACTION, &action, sizeof(action));

	memclr(&list_description, sizeof(list_description));
	list_description.list_title = resources[APP_RESOURCE_NAME];
	list_description.menu_title = resources[APP_RESOURCE_MENU];
	status |= DRM_CreateResource(&resources[APP_RESOURCE_LIST_DESCRIPTION], RES_TYPE_LIST_DESCR,
		&list_description, sizeof(list_description));

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
	APP_INSTANCE_T *appi;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	UINT8 notice_type;
	LIST_ENTRY_T *list;
	ACTIONS_T actions;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	appi = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;
	dialog = DialogType_None;

	memclr(&content, sizeof(CONTENT_T));

	switch (app_state) {
		case APP_STATE_MAIN:
			list = CreateList(ev_st, app);
			if (list != NULL) {
				DRM_SetResource(appi->resources[APP_RESOURCE_NAME],
					(void *) appi->current_title, (u_strlen(appi->current_title) + 1) * sizeof(WCHAR));

				actions.action[0].operation = ACTION_OP_ADD;
				actions.action[0].event = LdrFindEventHandlerTbl(g_state_main_hdls, HandleEventHelp);
				actions.action[0].action_res = appi->resources[APP_RESOURCE_ACTION_HELP];
				actions.action[1].operation = ACTION_OP_ADD;
				actions.action[1].event = LdrFindEventHandlerTbl(g_state_main_hdls, HandleEventAbout);
				actions.action[1].action_res = appi->resources[APP_RESOURCE_ACTION_ABOUT];
				actions.action[2].operation = ACTION_OP_ADD;
				actions.action[2].event = LdrFindEventHandlerTbl(g_state_main_hdls, HandleEventExit);
				actions.action[2].action_res = appi->resources[APP_RESOURCE_ACTION_EXIT];
				actions.count = 3;

				dialog = UIS_CreateStaticList(&port, 0, appi->fs.count, 0, list, FALSE, 2, &actions,
					appi->resources[APP_RESOURCE_LIST_DESCRIPTION]);
				suFreeMem(list);

				/* Insert cursor to proper position. */
				if (appi->flag_from_select) {
					if (appi->menu_current_item_index != 0) {
						APP_UtilAddEvChangeListPosition(ev_st, app, appi->menu_current_item_index + 1,
							NULL, NULL, NULL);

						UIS_HandleEvent(dialog, ev_st);
					}
					appi->flag_from_select = FALSE;
				}
			}
			break;
		case APP_STATE_POPUP:
			switch (appi->popup) {
				default:
				case APP_POPUP_WAIT:
					UpdateList(ev_st, app, appi->current_path, L"*");
					notice_type = NOTICE_TYPE_WAIT_NO_KEY;
					UIS_MakeContentFromString("MCq0NMCq1", &content, g_str_popup_please_wait, g_str_popup_sorting);
					break;
				case APP_POPUP_NOT_ELF:
					notice_type = NOTICE_TYPE_FAIL;
					UIS_MakeContentFromString("MCq0NMCq1", &content, g_str_popup_cannot_run, g_str_popup_not_elf);
					break;
			}
			dialog = UIS_CreateTransientNotice(&port, &content, notice_type);
			/* Reset it to default POPUP_WAIT */
			appi->popup = APP_POPUP_WAIT;
			break;
		case APP_STATE_VIEW:
			switch (appi->view) {
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
			/* Play an exit sound using quiet speaker. */
			DL_AudPlayTone(0x00,  0xFF);
			/* Fall through */
		case APP_TIMER_EXIT_FAST:
			return ApplicationStop(ev_st, app);
			break;
		case APP_TIMER_TO_MAIN_VIEW:
			return APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;
	EVENT_T *event;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	appi->menu_current_item_index = event->data.index - 1;

	switch (appi->fs.list[appi->menu_current_item_index].type) {
		case FS_VOLUME:
			u_strcpy(appi->current_path, appi->fs.list[appi->menu_current_item_index].name);
			appi->current_path[u_strlen(appi->current_path) - 1] = 0;
			u_strcpy(appi->current_title, appi->current_path);
			break;
		case FS_FOLDER:
			u_strcpy(appi->current_path,
				DirDown(appi->current_path, appi->fs.list[appi->menu_current_item_index].name));
			u_strcpy(appi->current_title, appi->current_path);
			break;
		case FS_FOLDER_UP:
			u_strcpy(appi->current_path,
				DirUp(appi->current_path));
			u_strcpy(appi->current_title, appi->current_path);
			break;
		case FS_FILE:
			appi->flag_from_select = TRUE;
			appi->popup = APP_POPUP_NOT_ELF;
			break;
		case FS_ELF:
			u_strcat(appi->current_path, L"/");
			u_strcat(appi->current_path, appi->fs.list[appi->menu_current_item_index].name);
			status |= RunElfApplication(ev_st, app, appi->current_path);
			break;
		default:
			break;
	}

#if defined(DEBUG)
	{
		char file_path[FS_MAX_PATH_NAME_LENGTH + 1];

		D("Selected: %d %d.\n", appi->menu_current_item_index, appi->fs.list[appi->menu_current_item_index].type);

		u_utoa(appi->current_path, file_path);
		D("Current Dir: %s\n", file_path);

		u_utoa(appi->current_title, file_path);
		D("Current Title: %s\n", file_path);
	}
#endif

	status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);

	return status;
}

static UINT32 HandleEventBackDir(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	if (!appi->fs.root) {
		u_strcpy(appi->current_path,
			DirUp(appi->current_path));
		u_strcpy(appi->current_title, appi->current_path);

		status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
	} else {
		status |= ApplicationStop(ev_st, app);
	}

	return status;
}

static UINT32 HandleEventHelp(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	appi->view = APP_VIEW_HELP;
	status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);

	return status;
}

static UINT32 HandleEventAbout(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	APP_INSTANCE_T *appi;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	appi->view = APP_VIEW_ABOUT;
	status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);

	return status;
}

static UINT32 HandleEventExit(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	status |= ApplicationStop(ev_st, app);

	return status;
}

static UINT32 HandleEventBack(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);

	return status;
}

static LIST_ENTRY_T *CreateList(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	INT32 result;
	UINT32 i;
	LIST_ENTRY_T *list_elements;
	APP_INSTANCE_T *appi;
	RESOURCE_ID list_icon;

	status = RESULT_OK;
	result = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	if (appi->fs.count == 0) {
		return NULL;
	}

	list_elements = (LIST_ENTRY_T *) suAllocMem(sizeof(LIST_ENTRY_T) * appi->fs.count, &result);
	if (result != RESULT_OK) {
		return NULL;
	}

	for (i = 0; i < appi->fs.count; ++i) {
		memclr(&list_elements[i], sizeof(LIST_ENTRY_T));
		list_elements[i].editable = FALSE;
		list_elements[i].content.static_entry.formatting = TRUE;

		switch (appi->fs.list[i].type) {
			case FS_VOLUME:
				list_icon = appi->resources[APP_RESOURCE_ICON_DISK];
				break;
			case FS_FOLDER_UP:
			case FS_FOLDER:
				list_icon = appi->resources[APP_RESOURCE_ICON_FOLDER];
				break;
			default:
			case FS_FILE:
				list_icon = appi->resources[APP_RESOURCE_ICON_FILE];
				break;
			case FS_ELF:
				list_icon = appi->resources[APP_RESOURCE_ICON_ELF];
				break;
		}

		status |= UIS_MakeContentFromString("Mp0 Mq1",
			&list_elements[i].content.static_entry.text, list_icon, appi->fs.list[i].name);
	}

	if (status != RESULT_OK) {
		suFreeMem(list_elements);
		return NULL;
	}

	return list_elements;
}

static UINT32 UpdateList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *directory, const WCHAR *filter) {
	UINT32 status;

	status = RESULT_OK;

	if (!directory || !u_strcmp(directory, L"/")) {
		status |= UpdateVolumeList(ev_st, app);
		status |= APP_UtilStartTimer(TIMER_FAST_TRIGGER_MS, APP_TIMER_TO_MAIN_VIEW, app);
	} else {
		WCHAR search_string[FS_MAX_URI_NAME_LENGTH + 16]; /* 280 */
		status |= CreateSearchString(search_string, directory, filter);
		status |= UpdateFileList(ev_st, app, search_string);
	}

	return RESULT_OK;
}

static UINT32 UpdateVolumeList(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	/*
	 * List of volumes.
	 *   /a/
	 *   /b/
	 *   /c/
	 */

	INT32 i;
	INT32 error;
	APP_INSTANCE_T *appi;
	WCHAR volumes[MAX_VOLUMES_COUNT * 3];
//	= {
//		L'/', L'a', 0xFFEE,
//		L'/', L'b', 0xFFEE,
//		L'/', L'c', 0xFFEE,
//		L'/', L'd', 0xFFEE
//	};
	WCHAR *result;

	appi = (APP_INSTANCE_T *) app;

	appi->fs.root = TRUE;

	result = DL_FsVolumeEnum(volumes);
	if (!result) {
		return RESULT_FAIL;
	}

	appi->fs.root = TRUE;
	appi->fs.count = 0;
	if (appi->fs.list) {
		D("%s\n", "Cleaning previous file list.");
		suFreeMem(appi->fs.list);
	}
	appi->fs.list = suAllocMem(sizeof(FS_OBJECT_T) * MAX_VOLUMES_COUNT, &error);
	if (error != RESULT_OK) {
		D("Error: Cannot allocate %d bytes.\n", sizeof(FS_OBJECT_T) * MAX_VOLUMES_COUNT);
		return RESULT_FAIL;
	}

	for (i = 0; i < MAX_VOLUMES_COUNT; ++i) {
		appi->fs.list[i].name[0] = volumes[i * 3];
		appi->fs.list[i].name[1] = volumes[i * 3 + 1];
		appi->fs.list[i].name[2] = volumes[i * 3];
		appi->fs.list[i].name[3] = 0;
		appi->fs.list[i].name[4] = 0;
		appi->fs.list[i].type = FS_VOLUME;
		appi->fs.count += 1;
		if (!volumes[i * 3 + 2]) {
			break;
		}
	}

	return RESULT_OK;
}

static UINT32 UpdateFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *search_string) {
	/*
	 * List of directories and files.
	 */
	UINT32 status;
	APP_INSTANCE_T *appi;

	FS_SEARCH_COMPLETED_INDEX_T search_index;
	FS_SEARCH_PARAMS_T search_params;
	FS_SEARCH_INFO_T *search_info;
	FS_URI_FNCT_PTR complete_function;
//	FS_SEARCH_RESULT_T search_result;
//	FS_SEARCH_HANDLE_T search_handle;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	appi->fs.root = FALSE;

	search_params.flags = FS_SEARCH_FOLDERS | FS_SEARCH_SORT_BY_NAME | FS_SEARCH_START_PATH;
	search_params.attrib = FS_ATTR_DEFAULT;
	search_params.mask = FS_ATTR_DEFAULT;

//	IFACE_DATA_T iface;
//	iface.port = app->port;
//	search_handle = DL_FsSSearch(&iface, search_params, search_string, 0);
//	D("LOL0: %d\n", search_handle);

//		if ((DL_FsSSearch(search_params, search_string, &search_handle, &appi->fs.count, 0)) != RESULT_OK) {
//			DL_FsSearchClose(search_handle);
//			return RESULT_FAIL;
//		}

	status |= DL_FsSSearch(search_params, search_string, &search_info, &complete_function, 0);
	D("Status: %d.\n", status);
	D("Info Num: %d.\n", search_info->num);
	if (status != RESULT_OK) {
		D("%s\n", "Trying to fix search.");
		if (search_info->shandle) {
			D("%s\n", "Search Handle is not 0.");
			status |= DL_FsSearchClose(search_info->shandle);
		}

		u_strcpy(appi->current_path, L"/");
		UpdateVolumeList(ev_st, app);

		status |= APP_UtilStartTimer(TIMER_FAST_TRIGGER_MS, APP_TIMER_TO_MAIN_VIEW, app);
		return status;
	}

	search_index.search_handle = search_info->shandle;
	search_index.search_total = search_info->num;

	status |= FillFileList(ev_st, app, &search_index);

	status |= APP_UtilStartTimer(TIMER_FAST_TRIGGER_MS, APP_TIMER_TO_MAIN_VIEW, app);

	return status;
}

static UINT32 FillFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, FS_SEARCH_COMPLETED_INDEX_T *search_index) {
	UINT32 status;
	INT32 error;
	APP_INSTANCE_T *appi;
	UINT16 i;
	UINT16 count;

	status = RESULT_OK;
	error = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;
	count = 1;

	D("%s\n", "Cleaning previous file list.");
	suFreeMem(appi->fs.list);
	appi->fs.list = NULL;

	appi->fs.count = search_index->search_total;
	appi->fs.count += 1;
	appi->fs.list = suAllocMem(sizeof(FS_OBJECT_T) * appi->fs.count, &error);
	if (error) {
		D("Error: Cannot allocate %d bytes.\n", sizeof(FS_OBJECT_T) * appi->fs.count);
		return RESULT_FAIL;
	}
	u_strcpy(appi->fs.list[0].name, L"..");
	appi->fs.list[0].type = FS_FOLDER_UP;

	for (i = 0; i < search_index->search_total; ++i) {
		status = DL_FsSearchResults(search_index->search_handle, i, &count, &search_index->search_result);
		if (status == RESULT_OK) {
#if defined(DEBUG)
			char file_path[FS_MAX_FILE_NAME_LENGTH + 1];
			u_utoa(search_index->search_result.name, file_path);
			D("%d Added: %s, attrib 0x%X, owner %d.\n",
				i + 1, file_path, search_index->search_result.attrib, search_index->search_result.owner);
#endif
			if (search_index->search_result.attrib == DIRECTORY_FILTER_ATTRIBUTE) {
				/*
				 * HACK Stage 1: Use first dot to proper files and folder sorting:
				 *  - ..
				 *  - .folder1
				 *  - .folder2
				 *  - File
				 *  - file
				 */
				u_strcpy(appi->fs.list[i + 1].name, L".");
				u_strcpy(appi->fs.list[i + 1].name + 1, GetFileNameFromPath(search_index->search_result.name));
				appi->fs.list[i + 1].type = FS_FOLDER;
			} else {
				u_strcpy(appi->fs.list[i + 1].name, GetFileNameFromPath(search_index->search_result.name));
				appi->fs.list[i + 1].type = (IsElfFile(appi->fs.list[i + 1].name)) ? FS_ELF : FS_FILE;
			}
		}
	}

	status |= SortFileList(ev_st, app);

	status |= DL_FsSearchClose(search_index->search_handle);

	return status;
}

static int file_comparate(const void *obj_1, const void *obj_2) {
	FS_OBJECT_T *fs_obj_1;
	FS_OBJECT_T *fs_obj_2;
	WCHAR fs_name_1[MAX_FILENAME_SYMS + 1];
	WCHAR fs_name_2[MAX_FILENAME_SYMS + 1];

	fs_obj_1 = (FS_OBJECT_T *) obj_1;
	fs_obj_2 = (FS_OBJECT_T *) obj_2;

	u_strcpy(fs_name_1, fs_obj_1->name);
	u_strcpy(fs_name_2, fs_obj_2->name);

	u_strmakeupper(fs_name_1);
	u_strmakeupper(fs_name_2);

	return u_strcmp(fs_name_1, fs_name_2);
}

static UINT32 SortFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	INT32 i;
	UINT32 status;
	APP_INSTANCE_T *appi;
	FS_OBJECT_T *objects;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;
	objects = appi->fs.list;

	qsort(objects, appi->fs.count, sizeof(FS_OBJECT_T), &file_comparate);

	/* HACK Stage 2: Strip additional sorting helper dots. */
	for (i = 0; i < appi->fs.count; ++i) {
		if (appi->fs.list[i].type == FS_FOLDER) {
			u_strcpy(appi->fs.list[i].name, appi->fs.list[i].name + 1);
		}
	}

	return status;
}

static UINT32 HandleEventSearchCompleted(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;
	FS_SEARCH_COMPLETED_INDEX_T *search_index;

	status = RESULT_OK;
	event = AFW_GetEv(ev_st);
	search_index = (FS_SEARCH_COMPLETED_INDEX_T *) event->attachment;

	D("%d elements found.\n", search_index->search_total);

	status |= FillFileList(ev_st, app, search_index);
	status |= APP_ConsumeEv(ev_st, app);
	status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);

	return status;
}

static UINT32 CreateSearchString(WCHAR *search_string, const WCHAR *search_directory, const WCHAR *search_pattern) {
	AddUriFileHeader(search_string, search_directory);

	if (search_directory[u_strlen(search_directory) - 1] != L'/') {
		u_strcat(search_string, L"/");
	}

	u_strcat(search_string, L"\xFFFE");
	u_strcat(search_string, search_pattern);

	return RESULT_OK;
}

static UINT32 AddUriFileHeader(WCHAR *out_str, const WCHAR *in_str) {
	if (!u_strncmp(in_str, L"file:/", 6)) {
		u_strcpy(out_str, in_str);
	} else {
		u_strcpy(out_str, L"file:/");
		u_strcat(out_str, in_str);
	}
	return RESULT_OK;
}

static const WCHAR *GetFileNameFromPath(const WCHAR *path) {
	if (u_strrchr(path, L'/')) {
		return u_strrchr(path, L'/') + 1;
	} else {
		return path;
	}
}

static BOOL IsElfFile(const WCHAR *path) {
	WCHAR *last_dot;

	last_dot = u_strrchr(path, L'.');

	if (last_dot) {
		last_dot += 1;

		if (last_dot[0] == L'e' &&
			last_dot[1] == L'l' &&
			last_dot[2] == L'f') {
			return TRUE;
		}

	}

	return FALSE;
}

static const WCHAR *DirUp(WCHAR *path) {
	u_strrchr(path, L'/')[0] = 0;
	if (!u_strlen(path)) {
		u_strcpy(path, L"/");
	}
	return path;
}

static const WCHAR *DirDown(WCHAR *path, const WCHAR *directory_name) {
	u_strcat(path, L"/");
	u_strcat(path, directory_name);
	return path;
}

static UINT32 RunElfApplication(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *elf_path) {
	UINT32 status;
	WCHAR elf_uri[FS_MAX_URI_NAME_LENGTH + 1]; /* 265 */

	status = RESULT_OK;

	status |= AddUriFileHeader(elf_uri, elf_path);

#if defined(DEBUG)
	{
		char elf_uri_string[FS_MAX_URI_NAME_LENGTH + 1]; /* 265 */
		u_utoa(elf_uri, elf_uri_string);
		D("Trying to run %s elf application.\n", elf_uri_string);
	}
#endif

	status |= LdrLoadELF(elf_uri, NULL);
	status |= APP_ConsumeEv(ev_st, app);
	status |= ApplicationStop(ev_st, app);

	return status;
}
