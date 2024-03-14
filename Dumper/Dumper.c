/*
 * About:
 *   The "Dumper" ELF utility for dumping various memory regions of Motorola P2K phones.
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
#include <time_date.h>
#include <mme.h>

#include "icons/icon_dumper_48x48.h"

#define TIMER_FAST_TRIGGER_MS             (1)
#define TIMER_POPUP_DELAY_MS             (50)
#define TIMER_EXIT_DELAY_MS             (100)
#define DISK_DRIVER_NAME_SIZE             (8)
#define DEBUG_OUTPUT_MAX_LENGTH         (256)

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
	APP_TIMER_DO_DUMP,
	APP_TIMER_DUMP_OK,
	APP_TIMER_GO_VIEW
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_NAME,
	APP_RESOURCE_ICON_DUMPER,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef enum {
	APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_BOOT_HWCFG = APP_MENU_ITEM_FIRST,
	APP_MENU_ITEM_PDS,
	APP_MENU_ITEM_RAM,
	APP_MENU_ITEM_BATTERY_ROM,
	APP_MENU_ITEM_IROM,
	APP_MENU_ITEM_IRAM,
	APP_MENU_ITEM_PANIC,
	APP_MENU_ITEM_SOC,
	APP_MENU_ITEM_HELP,
	APP_MENU_ITEM_ABOUT,
	APP_MENU_ITEM_EXIT,
	APP_MENU_ITEM_MAX
} APP_MENU_ITEM_T;

typedef enum {
	APP_POPUP_DUMP_OK,
	APP_POPUP_DUMP_FAIL,
	APP_POPUP_DUMP_WAIT
} APP_POPUP_T;

typedef enum {
	APP_VIEW_HELP,
	APP_VIEW_SOC,
	APP_VIEW_ABOUT
} APP_VIEW_T;

typedef enum {
	SOC_LTE,           /* E398, SLVR L6, ROKR E1, V3. */
	SOC_LTE_OLD,       /* V300, V500, V600, V80, A630. */
	SOC_LTE2,          /* V360, V235, SLVR L7. */
	SOC_LTE2_LAST,     /* SLVR L7e, SLVR L9, RIZR Z3, KRZR K1. */
	SOC_LTE2_EZX       /* ROKR E2 (BaseBand Processor), A1200 (BaseBand Processor). */
} PHONE_SOC_T;

typedef struct {
	UINT32 free_size;
	PHONE_SOC_T soc;
} PHONE_PARAMETERS_T;

typedef struct {
	APPLICATION_T app; /* Must be first. */

	RESOURCE_ID resources[APP_RESOURCE_MAX];
	APP_POPUP_T popup;
	APP_VIEW_T view;
	APP_MENU_ITEM_T menu_current_item_index;
	BOOL flag_from_select;
	BOOL flag_show_view_after_dumping;

	PHONE_PARAMETERS_T phone_parameters;
} APP_INSTANCE_T;

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments);                                /* ElfPack 2.x entry point. */
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments);                             /* ElfPack M*CORE entry point. */
#endif

static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 SetPhoneParameters(APP_INSTANCE_T *app_instance);
static const char *GetSoCName(PHONE_SOC_T SoC);

static UINT32 InitResourses(RESOURCE_ID *resources);
static UINT32 FreeResourses(RESOURCE_ID *resources);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventSelect(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventBack(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static LIST_ENTRY_T *CreateList(EVENT_STACK_T *ev_st, APPLICATION_T *app, UINT32 start, UINT32 count);

static UINT32 DumpBootAndHwcfg(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 DumpPds(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 DumpRam(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 DumpBatteryRom(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 DumpIROM(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 DumpIRAM(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 DumpPanic(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 DumpMemoryRegionToFile(
	EVENT_STACK_T *ev_st, APPLICATION_T *app,
	UINT32 start, UINT32 size, const WCHAR *filename,
	UINT32 chunk_size
);
static UINT32 DumpSoC(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static WCHAR *GetUniqueIdentifierSoc(void);
static WCHAR *GetProductRevisionSoc(void);
static WCHAR *GetProductVendorSoc(void);
static WCHAR *GetSiliconRevisionSoc(void);

static UINT32 ClearDataArrays(UINT8 *data_arr, UINT32 size);

static INT32 ReadBit(UINT16 value, UINT8 bitIndex);

static const char g_app_name[APP_NAME_LEN] = "Dumper";

static const WCHAR g_str_app_name[] = L"Dumper";
static const WCHAR g_str_menu_boot_hwcfg[] = L"Boot, hwcfg CG5";
static const WCHAR g_str_menu_pds[] = L"PDS CG6";
static const WCHAR g_str_menu_ram[] = L"RAM";
static const WCHAR g_str_menu_battery_rom[] = L"Battery ROM";
static const WCHAR g_str_menu_irom[] = L"IROM (integrated)";
static const WCHAR g_str_menu_iram[] = L"IRAM (integrated)";
static const WCHAR g_str_menu_panic[] = L"Panics Data";
static const WCHAR g_str_menu_soc[] = L"Neptune IC";
static const WCHAR g_str_menu_help[] = L"Help...";
static const WCHAR g_str_menu_about[] = L"About...";
static const WCHAR g_str_menu_exit[] = L"Exit";
static const WCHAR g_str_dump_ok[] = L"Memory region dumped to:";
static const WCHAR g_str_dump_fail[] = L"Check free space. Cannot dump memory region to:";
static const WCHAR g_str_dump_wait_p1[] = L"Dumping memory region to file";
static const WCHAR g_str_dump_wait_p2[] = L"Please wait...";
static const WCHAR g_str_view_help[] = L"Help";
static WCHAR *g_str_help_content_p1 = NULL;
static const WCHAR g_str_help_content_p2[] = L"ELF utility for dumping various memory regions of Motorola P2K phones."
	L" RAM and IROM dumps can take a long time, please be patient.";
static const WCHAR g_str_about_content_p1[] = L"Version: 1.0";
static const WCHAR g_str_about_content_p2[] = L"\x00A9 EXL, 02-Jun-2023.";
static const WCHAR g_str_about_content_p3[] = L"https://github.com/EXL/P2kElfs/tree/master/Dumper";
static const WCHAR g_str_about_content_p4[] = L"       "; /* HACK: gap */
static const WCHAR g_str_view_iim[] = L"Neptune IC Info";
static const WCHAR g_str_soc_uid[] = L"Unique Identifier:";
static const WCHAR g_str_soc_uid_u[] = L"0000 Unknown";
static const WCHAR g_str_soc_uid_d[] = L"0001 Development Part";
static const WCHAR g_str_soc_uid_p[] = L"0010 Production Part";
static const WCHAR g_str_soc_uid_s[] = L"0011 Non-Secure Part";
static const WCHAR g_str_soc_rev_product_revision[] = L"Product Revision:";
static const WCHAR g_str_soc_rev_pr_unk[] = L"00000 Reserved";
static const WCHAR g_str_soc_rev_pr_lts[] = L"10001 Neptune LTS 0000 ROM";
static const WCHAR g_str_soc_rev_pr_lte[] = L"10010 Neptune LTE 0200 ROM";
static const WCHAR g_str_soc_rev_pr_uls[] = L"10011 Neptune ULS 0000 ROM";
static const WCHAR g_str_soc_rev_pr_lte2[] = L"10100 Neptune LTE2 0300 ROM";
static const WCHAR g_str_soc_rev_pr_lte2_irom0400[] = L"101?? Neptune LTE2 0400 ROM?";
static const WCHAR g_str_soc_rev_product_vendor[] = L"Product Vendor / Tech:";
static const WCHAR g_str_soc_rev_pr_unk0[] = L"000 Unknown 000";
static const WCHAR g_str_soc_rev_pr_sps6[] = L"001 SPS HIP6W? 001";
static const WCHAR g_str_soc_rev_pr_sps7[] = L"010 SPS HIP7 010";
static const WCHAR g_str_soc_rev_pr_sps8[] = L"011 SPS HIP8 011";
static const WCHAR g_str_soc_rev_silicon_revision[] = L"Silicon Revision:";
static const WCHAR g_str_soc_rev_si_0[] = L"0000 (Initial Revision) Pass 1";
static const WCHAR g_str_soc_rev_si_1[] = L"0001 Pass 2";

static const WCHAR g_file_dump_boot[]        = L"D_BOOT_HWCFG.bin";
static const WCHAR g_file_dump_pds[]         = L"D_PDS.bin";
static const WCHAR g_file_dump_ram[]         = L"D_RAM.bin";
static const WCHAR g_file_dump_battery_rom[] = L"D_BATT_ROM.bin";
static const WCHAR g_file_dump_irom[]        = L"D_IROM.bin";
static const WCHAR g_file_dump_iram[]        = L"D_IRAM.bin";
static const WCHAR g_file_dump_panic[]       = L"D_PANIC.bin";
static const WCHAR g_file_dump_soc[]         = L"D_SOC.bin";

static WCHAR g_res_file_path[FS_MAX_URI_NAME_LENGTH];
static WCHAR g_cur_file_path[FS_MAX_URI_NAME_LENGTH];

#if defined(EP2)
static ldrElf g_app_elf;
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

	u_atou(elf_path_uri, g_res_file_path);

	LdrStartApp(ev_code_base);

	return status;
}
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments) {
	UINT32 status;
	UINT32 ev_code_base;
	UINT32 reserve;

	if (ldrIsLoaded(g_app_name)) {
		cprint("Dumper: Error! Application has already been loaded!\n");
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

	u_strcpy(g_res_file_path, uri);

	status |= ldrSendEvent(ev_code_base);
	g_app_elf.name = (char *) g_app_name;

	return (status == RESULT_OK) ? &g_app_elf : NULL;
}
#elif defined(EM2)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments) {
	UINT32 status;
	UINT32 reserve;
	WCHAR *ptr;

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

		ptr = NULL;
		u_strcpy(g_res_file_path, L"file:/");
		ptr = g_res_file_path + u_strlen(g_res_file_path);
		DL_FsGetURIFromID(&g_app_elf->id, ptr);

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
		app_instance->popup = APP_POPUP_DUMP_OK;
		app_instance->view = APP_VIEW_ABOUT;
		app_instance->flag_from_select = FALSE;
		app_instance->flag_show_view_after_dumping = FALSE;
		SetPhoneParameters(app_instance);

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

	APP_ConsumeEv(ev_st, app);

	DeleteDialog(app);

	FreeResourses(app_instance->resources);

	status |= APP_Exit(ev_st, app, 0);

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#elif defined(EM2)
	ldrUnloadElf(g_app_elf);
#endif

	return status;
}

static UINT32 SetPhoneParameters(APP_INSTANCE_T *app_instance) {
	INT32 result;
	char buffer_a[DEBUG_OUTPUT_MAX_LENGTH + 1];
	WCHAR buffer_u[DEBUG_OUTPUT_MAX_LENGTH + 1];
	char disk_a[DISK_DRIVER_NAME_SIZE]; /* /a/, /b/, /c/ */
	WCHAR disk_u[DISK_DRIVER_NAME_SIZE]; /* /a/, /b/, /c/ */
	VOLUME_DESCR_T volume_description;

	result = RESULT_OK;

	g_str_help_content_p1 = suAllocMem(sizeof(WCHAR) * (DEBUG_OUTPUT_MAX_LENGTH + 1), &result);
	if (result != RESULT_OK) {
		return RESULT_FAIL;
	}
	memclr(g_str_help_content_p1, sizeof(WCHAR) * (DEBUG_OUTPUT_MAX_LENGTH + 1));

	if (strncmp("LTE2", LdrGetPlatformName(), sizeof("LTE2") - 1) == 0) {
		if (strncmp("L7e", LdrGetPhoneName(), sizeof("L7e") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_LAST;
		} else if (strncmp("K1", LdrGetPhoneName(), sizeof("K1") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_LAST;
		} else if (strncmp("Z3", LdrGetPhoneName(), sizeof("Z3") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_LAST;
		} else if (strncmp("L9", LdrGetPhoneName(), sizeof("L9") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_LAST;
		} else if (strncmp("W490", LdrGetPhoneName(), sizeof("W490") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_LAST;
		} else if (strncmp("W510", LdrGetPhoneName(), sizeof("W510") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_LAST;
		} else if (strncmp("E2", LdrGetPhoneName(), sizeof("E2") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_EZX;
		} else if (strncmp("E6", LdrGetPhoneName(), sizeof("E6") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_EZX;
		} else if (strncmp("A1200", LdrGetPhoneName(), sizeof("A1200") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_EZX;
		} else if (strncmp("A1600", LdrGetPhoneName(), sizeof("A1600") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_EZX;
		} else if (strncmp("E895", LdrGetPhoneName(), sizeof("E895") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_EZX;
		} else if (strncmp("A910", LdrGetPhoneName(), sizeof("A910") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE2_EZX;
		} else {
			app_instance->phone_parameters.soc = SOC_LTE2;
		}
	} else if (strncmp("LTE", LdrGetPlatformName(), sizeof("LTE") - 1) == 0) {
		if (strncmp("V300", LdrGetPhoneName(), sizeof("V300") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE_OLD;
		} else if (strncmp("V500", LdrGetPhoneName(), sizeof("V500") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE_OLD;
		} else if (strncmp("V600", LdrGetPhoneName(), sizeof("V600") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE_OLD;
		} else if (strncmp("V80", LdrGetPhoneName(), sizeof("V80") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE_OLD;
		} else if (strncmp("A630", LdrGetPhoneName(), sizeof("A630") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE_OLD;
		} else if (strncmp("C650", LdrGetPhoneName(), sizeof("C650") - 1) == 0) {
			app_instance->phone_parameters.soc = SOC_LTE_OLD;
		} else {
			app_instance->phone_parameters.soc = SOC_LTE;
		}
	}

	sprintf(buffer_a, "Phone: %s\n", LdrGetPhoneName());
	u_atou(buffer_a, buffer_u);
	u_strcat(g_str_help_content_p1, buffer_u);

	sprintf(buffer_a, "Firmware: %s_%s\n", LdrGetFirmwareMajorVersion(), LdrGetFirmwareMinorVersion());
	u_atou(buffer_a, buffer_u);
	u_strcat(g_str_help_content_p1, buffer_u);

	PFprintf("SoC: %d\n", app_instance->phone_parameters.soc);
	sprintf(buffer_a, "SoC: %s\n", GetSoCName(app_instance->phone_parameters.soc));
	u_atou(buffer_a, buffer_u);
	u_strcat(g_str_help_content_p1, buffer_u);

	u_strncpy(disk_u, g_res_file_path + 6, 3); /* file://c/... */
	u_utoa(disk_u, disk_a);
	PFprintf("Disk: %s\n", disk_a);
	sprintf(buffer_a, "Disk: %s\n", disk_a);
	u_atou(buffer_a, buffer_u);
	u_strcat(g_str_help_content_p1, buffer_u);

	if (!DL_FsGetVolumeDescr(disk_u, &volume_description)) {
		return RESULT_FAIL;
	}
	app_instance->phone_parameters.free_size = volume_description.free;
	PFprintf("Free Space: %lu\n", app_instance->phone_parameters.free_size);
	sprintf(buffer_a, "Free Space: %lu\n\n", app_instance->phone_parameters.free_size);
	u_atou(buffer_a, buffer_u);
	u_strcat(g_str_help_content_p1, buffer_u);

	return RESULT_OK;
}

static const char *GetSoCName(PHONE_SOC_T SoC) {
	switch (SoC) {
		default:
		case SOC_LTE:
			return "LTE";
		case SOC_LTE_OLD:
			return "LTE (old)";
		case SOC_LTE2:
			return "LTE2";
		case SOC_LTE2_EZX:
			return "LTE2 (ezx)";
		case SOC_LTE2_LAST:
			return "LTE2 (last)";
	}
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_NAME], RES_TYPE_STRING,
		(void *) g_str_app_name, (u_strlen(g_str_app_name) + 1) * sizeof(WCHAR));

	status |= DRM_CreateResource(&resources[APP_RESOURCE_ICON_DUMPER], RES_TYPE_GRAPHICS,
		(void *) dumper_48x48_gif, sizeof(dumper_48x48_gif));

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

	suFreeMem(g_str_help_content_p1);

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
				case APP_POPUP_DUMP_OK:
					notice_type = NOTICE_TYPE_OK;
					UIS_MakeContentFromString("MCq0NMCq1", &content, g_str_dump_ok, g_cur_file_path);
					APP_UtilStartTimer(TIMER_POPUP_DELAY_MS, APP_TIMER_DUMP_OK, app);
					break;
				case APP_POPUP_DUMP_FAIL:
					notice_type = NOTICE_TYPE_FAIL;
					UIS_MakeContentFromString("MCq0NMCq1", &content, g_str_dump_fail, g_cur_file_path);
					break;
				case APP_POPUP_DUMP_WAIT:
					notice_type = NOTICE_TYPE_WAIT;
					UIS_MakeContentFromString("MCq0NMCq1", &content, g_str_dump_wait_p1, g_str_dump_wait_p2);
					APP_UtilStartTimer(TIMER_POPUP_DELAY_MS, APP_TIMER_DO_DUMP, app);
					break;
			}
			dialog = UIS_CreateTransientNotice(&port, &content, notice_type);
			break;
		case APP_STATE_VIEW:
			switch (app_instance->view) {
				default:
				case APP_VIEW_HELP:
					UIS_MakeContentFromString("q0Nq1Nq2", &content, g_str_view_help,
						g_str_help_content_p1, g_str_help_content_p2);
					break;
				case APP_VIEW_ABOUT:
					UIS_MakeContentFromString("q0NMCp1NMCq2NMCq3NMCq4NMCq5NMCq6", &content, g_str_app_name,
						app_instance->resources[APP_RESOURCE_ICON_DUMPER],
						g_str_about_content_p1, g_str_about_content_p2, g_str_about_content_p3,
						g_str_about_content_p4, g_str_about_content_p4);
					break;
				case APP_VIEW_SOC:
					UIS_MakeContentFromString("q0Nq1Nq2Nq3Nq4Nq5Nq6Nq7Nq8", &content, g_str_view_iim,
						g_str_soc_uid, GetUniqueIdentifierSoc(),
						g_str_soc_rev_product_revision, GetProductRevisionSoc(),
						g_str_soc_rev_product_vendor, GetProductVendorSoc(),
						g_str_soc_rev_silicon_revision, GetSiliconRevisionSoc());
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
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (state == EXIT_STATE_EXIT) {
		DeleteDialog(app);

		if (app_instance->flag_show_view_after_dumping) {
			app_instance->flag_show_view_after_dumping = FALSE;

			APP_UtilStartTimer(TIMER_FAST_TRIGGER_MS, APP_TIMER_GO_VIEW, app);
		}

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
		case APP_TIMER_EXIT_FAST:
			/* Play an exit sound using quiet speaker. */
			DL_AudPlayTone(0x00,  0xFF);
			return ApplicationStop(ev_st, app);
			break;
		case APP_TIMER_DO_DUMP:
			switch (app_instance->menu_current_item_index) {
				case APP_MENU_ITEM_BOOT_HWCFG:
					app_instance->popup =
						(DumpBootAndHwcfg(ev_st, app) == RESULT_OK) ? APP_POPUP_DUMP_OK : APP_POPUP_DUMP_FAIL;
					APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
					break;
				case APP_MENU_ITEM_PDS:
					app_instance->popup =
						(DumpPds(ev_st, app) == RESULT_OK) ? APP_POPUP_DUMP_OK : APP_POPUP_DUMP_FAIL;
					APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
					break;
				case APP_MENU_ITEM_RAM:
					app_instance->popup =
						(DumpRam(ev_st, app) == RESULT_OK) ? APP_POPUP_DUMP_OK : APP_POPUP_DUMP_FAIL;
					APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
					break;
				case APP_MENU_ITEM_BATTERY_ROM:
					app_instance->popup =
						(DumpBatteryRom(ev_st, app) == RESULT_OK) ? APP_POPUP_DUMP_OK : APP_POPUP_DUMP_FAIL;
					APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
					break;
				case APP_MENU_ITEM_IROM:
					app_instance->popup =
						(DumpIROM(ev_st, app) == RESULT_OK) ? APP_POPUP_DUMP_OK : APP_POPUP_DUMP_FAIL;
					APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
					break;
				case APP_MENU_ITEM_IRAM:
					app_instance->popup =
						(DumpIRAM(ev_st, app) == RESULT_OK) ? APP_POPUP_DUMP_OK : APP_POPUP_DUMP_FAIL;
					APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
					break;
				case APP_MENU_ITEM_PANIC:
					app_instance->popup =
						(DumpPanic(ev_st, app) == RESULT_OK) ? APP_POPUP_DUMP_OK : APP_POPUP_DUMP_FAIL;
					APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
					break;
				case APP_MENU_ITEM_SOC:
					app_instance->flag_show_view_after_dumping = TRUE;
					app_instance->popup =
						(DumpSoC(ev_st, app) == RESULT_OK) ? APP_POPUP_DUMP_OK : APP_POPUP_DUMP_FAIL;
					APP_UtilChangeState(APP_STATE_POPUP, ev_st, app);
					break;
			}
			break;
		case APP_TIMER_DUMP_OK:
#if defined(USE_MME)
			/* Play a normal camera shutter sound using loud speaker. */
			/* NOTE: Function `MME_GC_playback_open_audio_play_forget()` may not be available on most libraries. */
#if !defined(FTR_V600)
			MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter5.amr");
#else
			MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter5.wav");
#endif /* !defined(FTR_V600) */
#endif /* !defined(USE_MME) */
			break;
		case APP_TIMER_GO_VIEW:
			app_instance->view = APP_VIEW_SOC;
			APP_UtilChangeState(APP_STATE_VIEW, ev_st, app);
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
		case APP_MENU_ITEM_BOOT_HWCFG:
		case APP_MENU_ITEM_PDS:
		case APP_MENU_ITEM_RAM:
		case APP_MENU_ITEM_BATTERY_ROM:
		case APP_MENU_ITEM_IROM:
		case APP_MENU_ITEM_IRAM:
		case APP_MENU_ITEM_PANIC:
		case APP_MENU_ITEM_SOC:
			app_instance->popup = APP_POPUP_DUMP_WAIT;
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
		&list_elements[APP_MENU_ITEM_BOOT_HWCFG].content.static_entry.text,
		g_str_menu_boot_hwcfg);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_PDS].content.static_entry.text,
		g_str_menu_pds);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_RAM].content.static_entry.text,
		g_str_menu_ram);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_BATTERY_ROM].content.static_entry.text,
		g_str_menu_battery_rom);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_IROM].content.static_entry.text,
		g_str_menu_irom);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_IRAM].content.static_entry.text,
		g_str_menu_iram);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_PANIC].content.static_entry.text,
		g_str_menu_panic);
	status |= UIS_MakeContentFromString("Mq0",
		&list_elements[APP_MENU_ITEM_SOC].content.static_entry.text,
		g_str_menu_soc);
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

static UINT32 DumpBootAndHwcfg(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	return DumpMemoryRegionToFile(ev_st, app, 0x10000000, 0x10000, g_file_dump_boot, 0);
}

static UINT32 DumpPds(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->phone_parameters.soc != SOC_LTE2_EZX) {
		return DumpMemoryRegionToFile(ev_st, app, 0x10010000, 0x10000, g_file_dump_pds, 0);
	} else {
		return DumpMemoryRegionToFile(ev_st, app, 0x10010000, 0x20000, g_file_dump_pds, 0);
	}
}

static UINT32 DumpRam(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->phone_parameters.soc == SOC_LTE_OLD) {
		return DumpMemoryRegionToFile(ev_st, app, 0x12000000, 0x400000, g_file_dump_ram, 8192);
	} else if (app_instance->phone_parameters.soc == SOC_LTE || app_instance->phone_parameters.soc == SOC_LTE2) {
		return DumpMemoryRegionToFile(ev_st, app, 0x12000000, 0x800000, g_file_dump_ram, 8192);
	} else if (app_instance->phone_parameters.soc == SOC_LTE2_LAST) {
		return DumpMemoryRegionToFile(ev_st, app, 0x14000000, 0x1000000, g_file_dump_ram, 8192);
	} else { /* Unknown?! Dump atleast 2MB. */
		return DumpMemoryRegionToFile(ev_st, app, 0x12000000, 0x200000, g_file_dump_ram, 8192);
	}
}

static UINT32 DumpBatteryRom(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	UINT32 written;
	FILE rom;
	UINT8 battery_id[HAPI_BATTERY_ROM_UNIQUE_ID_SIZE];
	UINT8 battery_rom[HAPI_BATTERY_ROM_BYTE_SIZE];
	HAPI_BATTERY_ROM_T battery_status;

	status = RESULT_OK;

	ClearDataArrays(battery_id, HAPI_BATTERY_ROM_UNIQUE_ID_SIZE);
	ClearDataArrays(battery_rom, HAPI_BATTERY_ROM_BYTE_SIZE);

	battery_status = HAPI_BATTERY_ROM_NONE;
#if defined(FTR_V600)
	memset(&battery_id, 0xFF, HAPI_BATTERY_ROM_UNIQUE_ID_SIZE);
#else
	HAPI_BATTERY_ROM_get_unique_id(battery_id);
#endif
	battery_status = HAPI_BATTERY_ROM_read(battery_rom);

	u_strcpy(g_cur_file_path, g_res_file_path);
	g_cur_file_path[u_strlen(g_res_file_path) - 10] = '\0'; /* file://c/Elf/Dumper.elf */
	u_strcat(g_cur_file_path, g_file_dump_battery_rom);

	if (DL_FsFFileExist(g_cur_file_path)) {
		status = DL_FsDeleteFile(g_cur_file_path, 0);
		if (status != RESULT_OK) {
			return RESULT_FAIL;
		}
	}

	rom = DL_FsOpenFile(g_cur_file_path, FILE_WRITE_MODE, 0);

	/*
	 * Binary battery file format:
	 *    4 bytes = Battery status.
	 *    6 bytes = Battery id.
	 *  128 bytes = Battery ROM.
	 *  138 bytes = Total.
	 */
	status |= DL_FsWriteFile((void *) &battery_status, sizeof(UINT32), 1, rom, &written);
	status |= DL_FsWriteFile((void *) &battery_id, HAPI_BATTERY_ROM_UNIQUE_ID_SIZE, 1, rom, &written);
	status |= DL_FsWriteFile((void *) &battery_rom, HAPI_BATTERY_ROM_BYTE_SIZE, 1, rom, &written);

	DL_FsCloseFile(rom);

	return status;
}

static UINT32 DumpIROM(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->phone_parameters.soc == SOC_LTE2 || app_instance->phone_parameters.soc == SOC_LTE2_EZX) {
		return DumpMemoryRegionToFile(ev_st, app, 0x0, 0x20000, g_file_dump_irom, 0);
	} else if (app_instance->phone_parameters.soc == SOC_LTE2_LAST) {
		return DumpMemoryRegionToFile(ev_st, app, 0x0, 0x40000, g_file_dump_irom, 0);
	} else {
		return DumpMemoryRegionToFile(ev_st, app, 0x0, 0x1C0000, g_file_dump_irom, 0);
	}
}

static UINT32 DumpIRAM(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->phone_parameters.soc == SOC_LTE || app_instance->phone_parameters.soc == SOC_LTE_OLD) {
		return DumpMemoryRegionToFile(ev_st, app, 0x03FC0000, 0x40000, g_file_dump_iram, 0);
	} else {
		return DumpMemoryRegionToFile(ev_st, app, 0x03F80000, 0x80000, g_file_dump_iram, 0);
	}
}

static UINT32 DumpPanic(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	return DumpMemoryRegionToFile(ev_st, app, 0x10020000, 0x20000, g_file_dump_panic, 0);
}

static UINT32 DumpSoC(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	return DumpMemoryRegionToFile(ev_st, app, 0x24850000, 0x12, g_file_dump_soc, 0);
}


static UINT32 DumpMemoryRegionToFile(
	EVENT_STACK_T *ev_st, APPLICATION_T *app,
	UINT32 start, UINT32 size, const WCHAR *filename,
	UINT32 chunk_size
) {
	UINT32 status;
	UINT32 written;
	UINT32 real_start;
	UINT32 real_size;
	FILE_HANDLE_T file;
	APP_INSTANCE_T *app_instance;

	status = RESULT_OK;
	written = 0;
	app_instance = (APP_INSTANCE_T *) app;

	u_strcpy(g_cur_file_path, g_res_file_path);
	g_cur_file_path[u_strlen(g_res_file_path) - 10] = '\0'; /* file://c/Elf/Dumper.elf */
	u_strcat(g_cur_file_path, filename);

	if (app_instance->phone_parameters.free_size <= size) { /* 65536 bytes, 65 kB */
		return RESULT_FAIL;
	}

	if (DL_FsFFileExist(g_cur_file_path)) {
		status = DL_FsDeleteFile(g_cur_file_path, 0);
		if (status != RESULT_OK) {
			return RESULT_FAIL;
		}
	}

	file = DL_FsOpenFile(g_cur_file_path, FILE_WRITE_MODE, NULL);

	if (start == 0x0) { /* IROM dumping hack. */
		UINT8 first_byte = 0xE5;
		real_start = 0x01;
		real_size = size - 1;
		status |= DL_FsWriteFile((void *) &first_byte, sizeof(UINT8), 1, file, &written);
		if (written == 0) {
			status = RESULT_FAIL;
		}
	} else {
		real_start = start;
		real_size = size;
	}

	if (chunk_size != 0) {
		int i;
		for (i = real_start; i < real_start + real_size; i += chunk_size) {
			status |= DL_FsWriteFile((void *) i, chunk_size, 1, file, &written);
			if (written == 0) {
				status = RESULT_FAIL;
			}
		}
	} else {
		status |= DL_FsWriteFile((void *) real_start, real_size, 1, file, &written);
		if (written == 0) {
			status = RESULT_FAIL;
		}
	}
	status |= DL_FsCloseFile(file);

	return status;
}

/*
 * U800_Neptune LTE IC Baseband Specification.pdf:
 * https://firmware.center/firmware/Motorola/E398/Service%20Docs/Data%20Sheets/U800_Neptune%20LTE%20IC%20Baseband%20Specification.pdf
 */

static WCHAR *GetUniqueIdentifierSoc(void) {
	BOOL bit0;
	BOOL bit1;
	BOOL bit3;
	UINT16 uid0;

	uid0 = *((UINT16 *) 0x24850000);

	bit0 = ReadBit(uid0, 0);
	bit1 = ReadBit(uid0, 1);
	bit3 = ReadBit(uid0, 3);

	if (bit0) {
		return (WCHAR *) g_str_soc_uid_d;
	} else if (bit1) {
		return (WCHAR *) g_str_soc_uid_p;
	} else if (bit3) {
		return (WCHAR *) g_str_soc_uid_s;
	}
	return (WCHAR *) g_str_soc_uid_u;
}

static WCHAR *GetProductRevisionSoc(void) {
	BOOL bit11;
	BOOL bit12;
	BOOL bit13;
	BOOL bit15;
	UINT16 rev;

	rev = *((UINT16 *) 0x24850010);

	bit11 = ReadBit(rev, 11);
	bit12 = ReadBit(rev, 12);
	bit13 = ReadBit(rev, 13);
	bit15 = ReadBit(rev, 15);

	if (bit15) {
		if (bit13) {
			if (bit11 || bit12) {
				return (WCHAR *) g_str_soc_rev_pr_lte2_irom0400;
			} else {
				return (WCHAR *) g_str_soc_rev_pr_lte2;
			}
		} else {
			if (bit11 && bit12) {
				return (WCHAR *) g_str_soc_rev_pr_uls;
			} else if (bit11) {
				return (WCHAR *) g_str_soc_rev_pr_lts;
			} else if (bit12) {
				return (WCHAR *) g_str_soc_rev_pr_lte;
			}
		}
	}
	return (WCHAR *) g_str_soc_rev_pr_unk;
}

static WCHAR *GetProductVendorSoc(void) {
	BOOL bit8;
	BOOL bit9;
	UINT16 rev;

	rev = *((UINT16 *) 0x24850010);

	bit8 = ReadBit(rev, 8);
	bit9 = ReadBit(rev, 9);

	if (bit8 && bit9) {
		return (WCHAR *) g_str_soc_rev_pr_sps8;
	} else if (bit8) {
		return (WCHAR *) g_str_soc_rev_pr_sps6;
	} else if (bit9) {
		return (WCHAR *) g_str_soc_rev_pr_sps7;
	}
	return (WCHAR *) g_str_soc_rev_pr_unk0;
}

static WCHAR *GetSiliconRevisionSoc(void) {
	BOOL bit0;
	UINT16 rev;

	rev = *((UINT16 *) 0x24850010);

	bit0 = ReadBit(rev, 0);

	if (bit0) {
		return (WCHAR *) g_str_soc_rev_si_1;
	}
	return (WCHAR *) g_str_soc_rev_si_0;
}

static UINT32 ClearDataArrays(UINT8 *data_arr, UINT32 size) {
	UINT32 i;
	for (i = 0; i < size; ++i) {
		data_arr[i] = 0;
	}
	return RESULT_OK;
}

static INT32 ReadBit(UINT16 value, UINT8 bitIndex) {
	if (bitIndex > 15) {
		return -1;
	}
	return (value >> bitIndex) & 0x01;
}
