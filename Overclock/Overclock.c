/*
 * About:
 *   An application for overclocking ARM7TDMI core in Neptune SoC phones from stock 52 MHz to 65 MHz.
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

#include "Overclock.h"

#include "icons/icon_overclock_48x48.h"

#define TIMER_FAST_TRIGGER_MS             (1)
#define TIMER_POPUP_DELAY_MS             (50)
#define TIMER_EXIT_DELAY_MS             (100)
#define STRING_CURRENT_CLOCK_MAX         (64)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_POPUP,
	APP_STATE_VIEW,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0xE398,
	APP_TIMER_EXIT_FAST,
	APP_TIMER_DO_OVERCLOCK
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_NAME,
	APP_RESOURCE_ICON_OVERCLOCK,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_13MHZ_13MHZ = APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_13MHZ_26MHZ,
	APP_MENU_ITEM_26MHZ_21MHZ,
	APP_MENU_ITEM_26MHZ_26MHZ,
	APP_MENU_ITEM_39MHZ_26MHZ,
	APP_MENU_ITEM_52MHZ_26MHZ,
	APP_MENU_ITEM_65MHZ_26MHZ,
	APP_MENU_ITEM_65MHZ_32MHZ,
	APP_MENU_ITEM_65MHZ_43MHZ,
	APP_MENU_ITEM_69MHZ_26MHZ,
	APP_MENU_ITEM_78MHZ_26MHZ,
	APP_MENU_ITEM_86MHZ_26MHZ,
	APP_MENU_ITEM_104MHZ_26MHZ,
	APP_MENU_ITEM_130MHZ_26MHZ,
	APP_MENU_ITEM_HELP,
	APP_MENU_ITEM_ABOUT,
	APP_MENU_ITEM_EXIT,
	APP_MENU_ITEM_MAX
} APP_MENU_ITEM_T;

typedef enum {
	APP_POPUP_OK,
	APP_POPUP_ERROR
} APP_POPUP_T;

typedef enum {
	APP_VIEW_HELP,
	APP_VIEW_ABOUT
} APP_VIEW_T;

typedef struct {
	APPLICATION_T app; /* Must be first. */

	RESOURCE_ID resources[APP_RESOURCE_MAX];
	APP_POPUP_T popup;
	APP_VIEW_T view;
	APP_MENU_ITEM_T menu_current_item_index;
	BOOL flag_from_select;
} APP_INSTANCE_T;

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments);                                /* ElfPack 2.x entry point. */
#elif defined(EM1)
int _main(ElfLoaderApp ela);                                                 /* ElfPack 1.x M*CORE entry point. */
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments);                             /* ElfPack 2.x M*CORE entry point. */
#endif

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
static const WCHAR *GetSelectedClocks(APP_MENU_ITEM_T menu_item);
static NEPTUNE_CLOCKS_T SaveConvertMenuItemToClocks(APP_MENU_ITEM_T menu_item);

const char g_app_name[APP_NAME_LEN] = "Overclock";

static const WCHAR g_str_app_name[] = L"Overclock";
static const WCHAR g_str_menu_13_13[] = L"13 MHz | 13 MHz";
static const WCHAR g_str_menu_13_26[] = L"13 MHz | 26 MHz";
static const WCHAR g_str_menu_26_21[] = L"26 MHz | 21 MHz";
static const WCHAR g_str_menu_26_26[] = L"26 MHz | 26 MHz";
static const WCHAR g_str_menu_39_26[] = L"39 MHz | 26 MHz";
static const WCHAR g_str_menu_52_26[] = L"52 MHz | 26 MHz | stock";
static const WCHAR g_str_menu_65_26[] = L"65 MHz | 26 MHz";
static const WCHAR g_str_menu_65_32[] = L"65 MHz | 32 MHz";
static const WCHAR g_str_menu_65_43[] = L"65 MHz | 43 MHz";
static const WCHAR g_str_menu_69_26[] = L"69 MHz | 26 MHz";
static const WCHAR g_str_menu_78_26[] = L"78 MHz | 26 MHz";
static const WCHAR g_str_menu_86_26[] = L"86 MHz | 26 MHz";
static const WCHAR g_str_menu_104_26[] = L"104 MHz | 26 MHz";
static const WCHAR g_str_menu_130_26[] = L"130 MHz | 26 MHz";
static const WCHAR g_str_menu_help[] = L"Help...";
static const WCHAR g_str_menu_about[] = L"About...";
static const WCHAR g_str_menu_exit[] = L"Exit";
static const WCHAR g_str_popup_ok_p1[] = L"Success!";
static const WCHAR g_str_popup_ok_p2[] = L"Neptune SoC clocks: ";
static const WCHAR g_str_popup_error_p1[] = L"Error!";
static const WCHAR g_str_popup_error_p2[] = L"Cannot set Neptune SoC clocks to: ";
static const WCHAR g_str_view_help[] = L"Help";
static const WCHAR g_str_help_content_p1[] =
	L"An application for overclocking ARM7TDMI core in Neptune SoC phones from stock 52 MHz to 65 MHz.\n\n"
	L"Be careful with experimenting, overclocking can be dangerous and can brick your phone!\n\n"
	L"First digit is the clock value of the MCU (CPU), it was tested on 65 MHz.\n\n"
	L"Second digit is the reference clock for DSP and various peripherals.\n\n"
	L"Base frequency of Clock Control Module (CCM) is 260 MHz.\n\n"
	L"Dividers are 1, 2, 3, 4, 5, 6, 8, 10 or 12.";
static const WCHAR g_str_about_content_p1[] = L"Version: 1.0";
static const WCHAR g_str_about_content_p2[] = L"\x00A9 EXL, 08-Sep-2023.";
static const WCHAR g_str_about_content_p3[] = L"https://github.com/EXL/P2kElfs/tree/master/Overclock";
static const WCHAR g_str_about_content_p4[] = L"       "; /* HACK: gap */

static WCHAR g_str_current_clock[STRING_CURRENT_CLOCK_MAX];

#if defined(EP2)
static ldrElf g_app_elf;
#elif defined(EM1)
static ElfLoaderApp g_app_elf = { 0 };
#elif defined(EM2)
static ldrElf *g_app_elf = NULL;
#endif

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
	{ APP_STATE_POPUP, HandleStateEnter, HandleStateExit, g_state_popup_hdls },
	{ APP_STATE_VIEW, HandleStateEnter, HandleStateExit, g_state_popup_hdls } /* Same as popups. */
};

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code) {
	UINT32 status;
	UINT32 ev_code_base;

	ev_code_base = ev_code;

	status = APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	LdrStartApp(ev_code_base);

	return status;
}
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments) {
	UINT32 status;
	UINT32 ev_code_base;
	UINT32 reserve;

	if (ldrIsLoaded(g_app_name)) {
		cprint("Overclock: Error! Application has already been loaded!\n");
		return NULL;
	}

	status = RESULT_OK;
	ev_code_base = ldrRequestEventBase();
	reserve = ev_code_base + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_popup_hdls, reserve);

	status |= APP_Register(&ev_code_base, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	status |= ldrSendEvent(ev_code_base);
	g_app_elf.name = (char *) g_app_name;

	return (status == RESULT_OK) ? &g_app_elf : NULL;
}
#elif defined(EM1)
int _main(ElfLoaderApp ela) {
	UINT32 status;

	status = RESULT_OK;

	memcpy((void *) &g_app_elf, (void *) &ela, sizeof(ElfLoaderApp));

	status = APP_Register(&g_app_elf.evcode, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);

	LoaderShowApp(&g_app_elf);

	return RESULT_FAIL;
}
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments) {
	UINT32 status;
	UINT32 reserve;

	status = RESULT_OK;
	g_app_elf = elf;
	g_app_elf->name = (char *) g_app_name;

	if (ldrIsLoaded(g_app_elf->name)) {
		PFprintf("%s: Application already loaded.\n", g_app_elf->name);
		return RESULT_FAIL;
	}

	reserve = g_app_elf->evbase + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_popup_hdls, reserve);

	status |= APP_Register(&g_app_elf->evbase, 1, g_state_table_hdls, APP_STATE_MAX, (void *) ApplicationStart);
	if (status == RESULT_OK) {
		PFprintf("%s: Application has been registered successfully.\n", g_app_elf->name);

		status |= ldrSendEvent(g_app_elf->evbase);
	} else {
		PFprintf("%s: Cannot register application.\n", g_app_elf->name);
	}

	return status;
}
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, 1, 0);

		InitResourses(app_instance->resources);
		app_instance->menu_current_item_index = APP_MENU_ITEM_FIRST;
		app_instance->popup = APP_POPUP_OK;
		app_instance->view = APP_VIEW_ABOUT;
		app_instance->flag_from_select = FALSE;

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, ApplicationStop, g_app_name, 0);

#if defined(EP2)
		g_app_elf.app = (APPLICATION_T *) app_instance;
#elif defined(EM2)
		g_app_elf->app = &app_instance->app;
#endif
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

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#elif defined(EM1)
	LoaderEndApp(&g_app_elf);
#elif defined(EM2)
	ldrUnloadElf(g_app_elf);
#endif

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_NAME], RES_TYPE_STRING,
		(void *) g_str_app_name, u_strlen(g_str_app_name) * sizeof(WCHAR));

	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_OVERCLOCK], RES_TYPE_GRAPHICS,
		(void *) overclock_48x48_gif, sizeof(overclock_48x48_gif));

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
	UINT8 notice_type;
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
			list = CreateList(ev_st, app, 1, APP_MENU_ITEM_MAX);
			if (list != NULL) {
				u_strcpy(g_str_current_clock, L"Overclock: ");
				u_strcpy(g_str_current_clock + u_strlen(g_str_current_clock), DetermineNeptuneMcuClock());
				DRM_SetResource(app_instance->resources[APP_RESOURCE_NAME],
					(void *) g_str_current_clock, (u_strlen(g_str_current_clock) + 1) * sizeof(WCHAR));
				dialog = UIS_CreateStaticList(&port, 0, APP_MENU_ITEM_MAX, 0, list, FALSE, 2, NULL,
					app_instance->resources[APP_RESOURCE_NAME]);
				suFreeMem(list);

				/* Insert cursor to proper position. */
				if (app_instance->flag_from_select) {
					if (app_instance->menu_current_item_index != APP_MENU_ITEM_FIRST) {
						APP_UtilAddEvChangeListPosition(ev_st, app, app_instance->menu_current_item_index + 1,
							NULL, NULL, NULL);
						UIS_HandleEvent(dialog, ev_st);
					}
					app_instance->flag_from_select = FALSE;
				}
			}
			break;
		case APP_STATE_POPUP:
			switch (app_instance->popup) {
				default:
				case APP_POPUP_OK:
					notice_type = NOTICE_TYPE_OK;
					UIS_MakeContentFromString("MCq0NMCq1NMCq2", &content, g_str_popup_ok_p1, g_str_popup_ok_p2,
						GetSelectedClocks(app_instance->menu_current_item_index));
					APP_UtilStartTimer(TIMER_POPUP_DELAY_MS, APP_TIMER_DO_OVERCLOCK, app);
					break;
				case APP_POPUP_ERROR:
					notice_type = NOTICE_TYPE_OK;
					UIS_MakeContentFromString("MCq0NMCq1NMCq2", &content, g_str_popup_error_p1, g_str_popup_error_p2,
						GetSelectedClocks(app_instance->menu_current_item_index));
					break;
			}
			dialog = UIS_CreateTransientNotice(&port, &content, notice_type);
			break;
		case APP_STATE_VIEW:
			switch (app_instance->view) {
				default:
				case APP_VIEW_HELP:
					UIS_MakeContentFromString("q0Nq1", &content, g_str_view_help, g_str_help_content_p1);
					break;
				case APP_VIEW_ABOUT:
					UIS_MakeContentFromString("q0NMCp1NMCq2NMCq3NMCq4NMCq5NMCq6", &content, g_str_app_name,
						app_instance->resources[APP_RESOURCE_ICON_OVERCLOCK],
						g_str_about_content_p1, g_str_about_content_p2, g_str_about_content_p3,
						g_str_about_content_p4, g_str_about_content_p4);
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
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	switch (timer_id) {
		case APP_TIMER_EXIT:
			/* No break here. */
		case APP_TIMER_EXIT_FAST:
			/* Play an exit sound using quiet speaker. */
			DL_AudPlayTone(0x00, 0xFF);
			return ApplicationStop(ev_st, app);
			break;
		case APP_TIMER_DO_OVERCLOCK:
			SetNeptuneClocks(SaveConvertMenuItemToClocks(app_instance->menu_current_item_index));
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
		case APP_MENU_ITEM_13MHZ_13MHZ:
		case APP_MENU_ITEM_13MHZ_26MHZ:
		case APP_MENU_ITEM_26MHZ_21MHZ:
		case APP_MENU_ITEM_26MHZ_26MHZ:
		case APP_MENU_ITEM_39MHZ_26MHZ:
		case APP_MENU_ITEM_52MHZ_26MHZ:
		case APP_MENU_ITEM_65MHZ_26MHZ:
		case APP_MENU_ITEM_65MHZ_32MHZ:
		case APP_MENU_ITEM_65MHZ_43MHZ:
		case APP_MENU_ITEM_69MHZ_26MHZ:
		case APP_MENU_ITEM_78MHZ_26MHZ:
		case APP_MENU_ITEM_86MHZ_26MHZ:
		case APP_MENU_ITEM_104MHZ_26MHZ:
		case APP_MENU_ITEM_130MHZ_26MHZ:
			app_instance->popup = APP_POPUP_OK;
			status |= APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
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
			status |= APP_UtilStartTimer(TIMER_EXIT_DELAY_MS, APP_TIMER_EXIT, app);
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
		&list_elements[APP_MENU_ITEM_13MHZ_13MHZ].content.static_entry.text,
		g_str_menu_13_13);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_13MHZ_26MHZ].content.static_entry.text,
		g_str_menu_13_26);
		status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_26MHZ_21MHZ].content.static_entry.text,
		g_str_menu_26_21);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_26MHZ_26MHZ].content.static_entry.text,
		g_str_menu_26_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_39MHZ_26MHZ].content.static_entry.text,
		g_str_menu_39_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_52MHZ_26MHZ].content.static_entry.text,
		g_str_menu_52_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_65MHZ_26MHZ].content.static_entry.text,
		g_str_menu_65_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_65MHZ_32MHZ].content.static_entry.text,
		g_str_menu_65_32);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_65MHZ_43MHZ].content.static_entry.text,
		g_str_menu_65_43);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_69MHZ_26MHZ].content.static_entry.text,
		g_str_menu_69_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_78MHZ_26MHZ].content.static_entry.text,
		g_str_menu_78_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_86MHZ_26MHZ].content.static_entry.text,
		g_str_menu_86_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_104MHZ_26MHZ].content.static_entry.text,
		g_str_menu_104_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_130MHZ_26MHZ].content.static_entry.text,
		g_str_menu_130_26);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_HELP].content.static_entry.text,
		g_str_menu_help);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_ABOUT].content.static_entry.text,
		g_str_menu_about);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_EXIT].content.static_entry.text,
		g_str_menu_exit);

	if (status != RESULT_OK) {
		suFreeMem(list_elements);
		return NULL;
	}

	return list_elements;
}

static const WCHAR *GetSelectedClocks(APP_MENU_ITEM_T menu_item) {
	switch (menu_item) {
		case APP_MENU_ITEM_13MHZ_13MHZ:
			return g_str_menu_13_13;
		case APP_MENU_ITEM_13MHZ_26MHZ:
			return g_str_menu_13_26;
		case APP_MENU_ITEM_26MHZ_21MHZ:
			return g_str_menu_26_21;
		case APP_MENU_ITEM_26MHZ_26MHZ:
			return g_str_menu_26_26;
		case APP_MENU_ITEM_39MHZ_26MHZ:
			return g_str_menu_39_26;
		case APP_MENU_ITEM_52MHZ_26MHZ:
			return g_str_menu_52_26;
		case APP_MENU_ITEM_65MHZ_26MHZ:
			return g_str_menu_65_26;
		case APP_MENU_ITEM_65MHZ_32MHZ:
			return g_str_menu_65_32;
		case APP_MENU_ITEM_65MHZ_43MHZ:
			return g_str_menu_65_43;
		case APP_MENU_ITEM_69MHZ_26MHZ:
			return g_str_menu_69_26;
		case APP_MENU_ITEM_78MHZ_26MHZ:
			return g_str_menu_78_26;
		case APP_MENU_ITEM_86MHZ_26MHZ:
			return g_str_menu_86_26;
		case APP_MENU_ITEM_104MHZ_26MHZ:
			return g_str_menu_104_26;
		case APP_MENU_ITEM_130MHZ_26MHZ:
			return g_str_menu_130_26;
		default:
			break;
	}
	return NULL;
}

static NEPTUNE_CLOCKS_T SaveConvertMenuItemToClocks(APP_MENU_ITEM_T menu_item) {
	switch (menu_item) {
		case APP_MENU_ITEM_13MHZ_13MHZ:
			return CLOCKS_13MHZ_13MHZ;
		case APP_MENU_ITEM_13MHZ_26MHZ:
			return CLOCKS_13MHZ_26MHZ;
		case APP_MENU_ITEM_26MHZ_21MHZ:
			return CLOCKS_26MHZ_21MHZ;
		case APP_MENU_ITEM_26MHZ_26MHZ:
			return CLOCKS_26MHZ_26MHZ;
		case APP_MENU_ITEM_39MHZ_26MHZ:
			return CLOCKS_39MHZ_26MHZ;
		case APP_MENU_ITEM_52MHZ_26MHZ:
			return CLOCKS_52MHZ_26MHZ;
		case APP_MENU_ITEM_65MHZ_26MHZ:
			return CLOCKS_65MHZ_26MHZ;
		case APP_MENU_ITEM_65MHZ_32MHZ:
			return CLOCKS_65MHZ_32MHZ;
		case APP_MENU_ITEM_65MHZ_43MHZ:
			return CLOCKS_65MHZ_43MHZ;
		case APP_MENU_ITEM_69MHZ_26MHZ:
			return CLOCKS_69MHZ_26MHZ;
		case APP_MENU_ITEM_78MHZ_26MHZ:
			return CLOCKS_78MHZ_26MHZ;
		case APP_MENU_ITEM_86MHZ_26MHZ:
			return CLOCKS_86MHZ_26MHZ;
		case APP_MENU_ITEM_104MHZ_26MHZ:
			return CLOCKS_104MHZ_26MHZ;
		case APP_MENU_ITEM_130MHZ_26MHZ:
			return CLOCKS_130MHZ_26MHZ;
		default:
			break;
	}
	return CLOCKS_52MHZ_26MHZ;
}
