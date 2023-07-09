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
#include <resources.h>

#include "icons/icon_elf_gif.h"

#if !defined(DEBUG)
#define D(format, ...)
#else
#define D(format, ...) UtilLogStringData(format, ##__VA_ARGS__); PFprintf(format, ##__VA_ARGS__)
#endif

#define MAX_VOLUMES_COUNT     (4)
#define MAX_FILENAME_SYMS     (64)

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
	APP_TIMER_EXIT_FAST
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_NAME,
	APP_RESOURCE_ICON_ELF,
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
	FS_UP_FOLDER,
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
static UINT32 HandleEventSearchCompleted(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static LIST_ENTRY_T *CreateList(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 UpdateList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *directory, const WCHAR *filter);
static UINT32 UpdateVolumeList(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 UpdateFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *search_string);
static UINT32 FillFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, FS_SEARCH_COMPLETED_INDEX_T *search_index);

static UINT32 CreateSearchString(WCHAR *search_string, const WCHAR *search_directory, const WCHAR *search_pattern);
static const WCHAR *GetFileNameFromPath(WCHAR *path);

static const char g_app_name[APP_NAME_LEN] = "ElfBox";
static const UINT32 g_ev_code_base = 0x00000FF1;

static const WCHAR g_str_app_name[] = L"ElfBox";

static const WCHAR g_str_menu_help[] = L"Help...";
static const WCHAR g_str_menu_about[] = L"About...";
static const WCHAR g_str_view_help[] = L"Help";
static const WCHAR g_str_popup_please_wait[] = L"Please wait";
static const WCHAR g_str_popup_error[] = L"Error!";
static const WCHAR g_str_popup_not_elf[] = L"It is not ELF file.";
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
	{ 0x8213D, HandleEventSearchCompleted },
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

	status = APP_Register(&g_ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	return status;
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

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_NAME], RES_TYPE_STRING,
		(void *) g_str_app_name, (u_strlen(g_str_app_name) + 1) * sizeof(WCHAR));
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_ELF], RES_TYPE_GRAPHICS,
		(void *) icon_elf_gif, sizeof(icon_elf_gif));

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

	if (state != ENTER_STATE_ENTER) {
//		if (app->state != APP_STATE_MAIN) {
			return RESULT_OK;
//		}
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
				dialog = UIS_CreateStaticList(&port, 0, appi->fs.count, 0, list, FALSE, 2, NULL,
					appi->resources[APP_RESOURCE_NAME]);
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
					UpdateList(ev_st, app, L"/a/mobile", L"*");
					notice_type = NOTICE_TYPE_WAIT_NO_KEY;
					UIS_MakeContentFromString("MCq0", &content, g_str_popup_please_wait);
					break;
				case APP_POPUP_NOT_ELF:
					notice_type = NOTICE_TYPE_FAIL;
					UIS_MakeContentFromString("MCq0NMCq1", &content, g_str_popup_error, g_str_popup_not_elf);
					break;
			}
			dialog = UIS_CreateTransientNotice(&port, &content, notice_type);
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
	APP_INSTANCE_T *appi;
	EVENT_T *event;

	status = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);

	appi->flag_from_select = TRUE;
	appi->menu_current_item_index = event->data.index - 1;

	switch (appi->menu_current_item_index) {
		case 0:
			appi->view = APP_VIEW_HELP;
			status |= APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);
			break;
		case 1:
			appi->view = APP_VIEW_ABOUT;
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

		status |= UIS_MakeContentFromString("q1",
			&list_elements[i].content.static_entry.text, appi->fs.list[i].name);

		list_elements[i].content.static_entry.marks[0] = appi->resources[APP_RESOURCE_ICON_ELF];
		list_elements[i].content.static_entry.marks[1] = appi->resources[APP_RESOURCE_ICON_ELF];
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
	WCHAR *result;

	appi = (APP_INSTANCE_T *) app;
	result = DL_FsVolumeEnum(volumes);
	if (!result) {
		return RESULT_FAIL;
	}

	appi->fs.root = TRUE;
	appi->fs.count = 0;
	if (appi->fs.list) {
		suFreeMem(appi->fs.list);
	}
	appi->fs.list = suAllocMem(sizeof(FS_OBJECT_T) * MAX_VOLUMES_COUNT, &error);
	if (error != RESULT_OK) {
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
}

WCHAR* e(UINT16 a, W_CHAR*s) {

	D(":AAAAA: %d\n", a);

	return NULL;
}

static UINT32 UpdateFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, const WCHAR *search_string) {
	/*
	 * List of directories and files.
	 */
	FS_SEARCH_PARAMS_T search_params;
	FS_SEARCH_RESULT_T search_result;
	FS_SEARCH_HANDLE_T search_handle;

	INT32 error;
	APP_INSTANCE_T *appi;

	IFACE_DATA_T iface;


	INT32 i;
	INT32 j;
	UINT8 mode;
	UINT16 res;

	DL_FS_SEARCH_INFO_T *search_info = NULL;

	search_params.flags = FS_SEARCH_START_PATH | FS_SEARCH_FOLDERS;
	search_params.attrib = FS_ATTR_DEFAULT;
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
//		DL_FsSearchClose(search_info->shandle);
}

static UINT32 FillFileList(EVENT_STACK_T *ev_st, APPLICATION_T *app, FS_SEARCH_COMPLETED_INDEX_T *search_index) {
	UINT32 status;
	INT32 error;
	APP_INSTANCE_T *appi;
	UINT16 i;
	UINT16 count, mode;
#if defined(DEBUG)
	char file_path[FS_MAX_FILE_NAME_LENGTH + 1];
#endif

	status = RESULT_OK;
	error = RESULT_OK;
	appi = (APP_INSTANCE_T *) app;
	count = 1;

	D("%s:%d: Cleaning previous file list.\n", __func__, __LINE__);
	suFreeMem(appi->fs.list);
	appi->fs.list = NULL;

	appi->fs.count = search_index->search_total;
	appi->fs.count += 1;
	appi->fs.list = suAllocMem(sizeof(FS_OBJECT_T) * appi->fs.count, &error);
	if (error) {
		D("%s:%d: Error: Cannot allocate %d bytes.\n", __func__, __LINE__, sizeof(FS_OBJECT_T) * appi->fs.count);
		return RESULT_FAIL;
	}
	u_strcpy(appi->fs.list[0].name, L"..");
	appi->fs.list[0].type = FS_UP_FOLDER;

	for (i = 0; i < search_index->search_total; ++i) {
		status = DL_FsSearchResults(search_index->search_handle, i, &count, &search_index->search_result);
		if (status == RESULT_OK) {
#if defined(DEBUG)
			u_utoa(search_index->search_result.name, file_path);
			D("%s:%d: %d Added: %s, attrib 0x%X, owner %d.\n",
				__func__, __LINE__, i + 1, file_path,
				search_index->search_result.attrib, search_index->search_result.owner);
#endif
			u_strcpy(appi->fs.list[i + 1].name, GetFileNameFromPath(search_index->search_result.name));

			if (search_index->search_result.attrib & FS_ATTR_DIRECTORY) {
				appi->fs.list[i + 1].type = FS_FOLDER;
			} else {
//				if (!u_stristr(appi->fs.list[i + 1].name, L".elf")) {
//					appi->fs.list[i + 1].type = FS_ELF;
//				} else {
//					appi->fs.list[i + 1].type = FS_FILE;
//				}
				appi->fs.list[i + 1].type = FS_FILE;
			}
		}
	}

	status |= DL_FsSearchClose(search_index->search_handle);

	return status;
}

static UINT32 HandleEventSearchCompleted(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;
	FS_SEARCH_COMPLETED_INDEX_T *search_index;

	status = RESULT_OK;
	event = AFW_GetEv(ev_st);
	search_index = (FS_SEARCH_COMPLETED_INDEX_T *) event->attachment;

	D("%s:%d: %d elements found.\n", __func__, __LINE__, search_index->search_total);

	status |= FillFileList(ev_st, app, search_index);
	status |= APP_ConsumeEv(ev_st, app);
	status |= APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);

	return status;
}

static UINT32 CreateSearchString(WCHAR *search_string, const WCHAR *search_directory, const WCHAR *search_pattern) {
	if (!u_strncmp(search_directory, L"file:/", 6)) {
		u_strcpy(search_string, search_directory);
	} else {
		u_strcpy(search_string, L"file:/");
		u_strcat(search_string, search_directory);
	}

	if (search_directory[u_strlen(search_directory) - 1] != L'/') {
		u_strcat(search_string, L"/");
	}

	u_strcat(search_string, L"\xFFFE");
	u_strcat(search_string, search_pattern);
}

static const WCHAR *GetFileNameFromPath(WCHAR *path) {
	return u_strrchr(path, L'/') + 1;
}
