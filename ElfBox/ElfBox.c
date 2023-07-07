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

#define MAX_VOLUMES_COUNT     (4)

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

typedef enum {
	FS_VOLUME,
	FS_FOLDER,
	FS_FILE,
	FS_ELF
} FS_ENTITY_T;

typedef struct {
	WCHAR name[FS_MAX_FILE_NAME_LENGTH + 1];
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
	APP_VIEW_T view;
	UINT32 menu_current_item_index;
	BOOL flag_from_select;

	FS_T fs;
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
static UINT32 HandleEventSearchCompleated(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static LIST_ENTRY_T *CreateList(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 UpdateFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *directory, const WCHAR *filter);

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
	{ 0x8213D, HandleEventSearchCompleated },
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
		app_instance->fs.root = FALSE;
		app_instance->fs.count = 0;
		app_instance->fs.list = NULL;

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
//		if (app->state != APP_STATE_MAIN) {
			return RESULT_OK;
//		}
	}

	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;
	dialog = DialogType_None;

	memclr(&content, sizeof(CONTENT_T));

	switch (app_state) {
		case APP_STATE_MAIN:
			list = CreateList(ev_st, app);
			if (list != NULL) {
				dialog = UIS_CreateStaticList(&port, 0, app_instance->fs.count, 0, list, FALSE, 2, NULL,
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

static LIST_ENTRY_T *CreateList(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	INT32 result;
	UINT32 i;
	LIST_ENTRY_T *list_elements;
	APP_INSTANCE_T *appi;
	char debug_str[FS_MAX_URI_NAME_LENGTH + 16];

	status = RESULT_OK;
	result = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	if (appi->fs.list) {
		suFreeMem(appi->fs.list);
		appi->fs.list = NULL;
	}
	UpdateFileList(ev_st, app, L"/a/mobile/audio", L"*");

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

		status |= UIS_MakeContentFromString("Mq0",
			&list_elements[i].content.static_entry.text,
			appi->fs.list[i].name);

		u_utoa(appi->fs.list[i].name, debug_str);
		PFprintf("ELF: %d %s\n", i, debug_str);
	}

	if (status != RESULT_OK) {
		suFreeMem(list_elements);
		return NULL;
	}

	return list_elements;
}

WCHAR* e(UINT16 a, W_CHAR*s) {

	PFprintf(":AAAAA: %d\n", a);

	return NULL;
}

static UINT32 UpdateFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *directory, const WCHAR *filter) {
	INT32 error;
	APP_INSTANCE_T *appi;
	FS_SEARCH_PARAMS_T search_params;
	FS_SEARCH_RESULT_T search_result;
	FS_SEARCH_HANDLE_T search_handle;
	IFACE_DATA_T iface;
	char debug_str[FS_MAX_URI_NAME_LENGTH + 16];

	error = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;

	if (!directory || !u_strcmp(directory, L"/")) {
		/*
		 * List of volumes.
		 *   /a/
		 *   /b/
		 *   /c/
		 */

		INT32 i;
		WCHAR volumes[MAX_VOLUMES_COUNT * 3];
		WCHAR *result = DL_FsVolumeEnum(volumes);
		if (!result) {
			return RESULT_FAIL;
		}

		appi->fs.root = TRUE;
		appi->fs.count = 0;
		if (appi->fs.list) {
			suFreeMem(appi->fs.list);
		}
		appi->fs.list = suAllocMem(sizeof(FS_OBJECT_T) * MAX_VOLUMES_COUNT, &error);
		if (error) {
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
	} else {
		/*
		 * List of directories and files.
		 */
		INT32 i;
		INT32 j;
		UINT8 mode;
		UINT16 res;
		WCHAR search_string[FS_MAX_URI_NAME_LENGTH + 16]; /* 280 */

		DL_FS_SEARCH_INFO_T *search_info = NULL;

		if (!u_strncmp(directory, L"file:/", 6)) {
			u_strcpy(search_string, directory);
		} else {
			u_strcpy(search_string, L"file:/");
			u_strcat(search_string, directory);
		}

		if (directory[u_strlen(directory) - 1] != L'/') {
			u_strcat(search_string, L"/");
		}

		u_strcat(search_string, L"\xFFFE");
		u_strcat(search_string, filter);

		search_params.flags = 0x1C;
		search_params.attrib = 0;
		search_params.mask = 0;

		iface.port = app->port;
		search_handle = DL_FsSSearch(&iface, search_params, search_string, 0);
		PFprintf("LOL0: %d\n", search_handle);

//		if ((DL_FsSSearch(search_params, search_string, &search_handle, &appi->fs.count, 0)) != RESULT_OK) {
//			DL_FsSearchClose(search_handle);
//			return RESULT_FAIL;
//		}
/*
		search_handle = DL_FsSSearch(
			search_params,
			search_string,
			&search_info,
			(DL_FS_URI_FNCT_PTR *)e,
			0);

		u_utoa(search_string, debug_str);
		PFprintf("SH: %d\n", search_handle);
		PFprintf("LOL: %s\n", debug_str);

		PFprintf("SN: %d\n", search_info->num);

		appi->fs.count = search_info->num;
		appi->fs.count += 1;

		if (appi->fs.list) {
			suFreeMem(appi->fs.list);
		}
		appi->fs.list = suAllocMem(sizeof(FS_OBJECT_T) * appi->fs.count, &error);
		if (error) {
			PFprintf("ELF: err %d\n", appi->fs.count);
			return RESULT_FAIL;
		}

		u_strcpy(appi->fs.list[0].name, L"..");
		appi->fs.list[0].type = FS_FOLDER;

		res = 1;
		mode = 0;
		for (i = 0, j = 1; j <= appi->fs.count; ++i) {
			if (i >= appi->fs.count) {
				if (mode > 0) {
					break;
				} else {
					i = -1;
					mode = 1;
					continue;
				}
			}
			if (DL_FsSearchResults(search_info->shandle, i, &res, &search_result) == RESULT_OK) {
				if ((mode == 0) && !(search_result.attrib & FS_ATTR_DIRECTORY)) {
					continue;
				}
				if ((mode == 1) && (search_result.attrib & FS_ATTR_DIRECTORY)) {
					continue;
				}
				u_strcpy(appi->fs.list[j].name, search_result.name);
				appi->fs.list[j].type = FS_FILE;
				j++;
			}
		}
*/
		DL_FsSearchClose(search_info->shandle);
	}

	return RESULT_OK;
}

static UINT32 HandleEventSearchCompleated(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	FS_SEARCH_COMPLETED_INDEX_T *search_index;
	char name[256];
	UINT16 res;
	int i;

	event = AFW_GetEv(ev_st);
	search_index = (FS_SEARCH_COMPLETED_INDEX_T *) event->attachment;

	PFprintf("Search Completed: %d %d\n", event->att_size, search_index->search_total);

	for (i = 0; i < search_index->search_total; ++i) {
		if (DL_FsSearchResults(search_index->search_handle, i, &res, &search_index->search_result) == RESULT_OK) {
			u_utoa(search_index->search_result.name, name);
			PFprintf("Search Completed: %s\n", name);
		}
	}

	return RESULT_OK;
}
