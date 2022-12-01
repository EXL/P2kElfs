#include <apps.h>
#include <uis.h>

typedef struct {
	APPLICATION_T app;
} ELF_T;

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_DUMP_OK,
	APP_STATE_DUMP_FAIL,
	APP_STATE_MAX
} APP_STATES_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, void *app);

static UINT32 HandleStateEnterMain(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, void *app, EXIT_STATE_TYPE_T state);
static UINT32 HandleStateEnterDumpOk(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateEnterDumpFail(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state);

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventNo(EVENT_STACK_T *ev_st, void *app);

static UINT32 DumpBatteryRom(void);

static const char g_app_name[APP_NAME_LEN] = "BattDump";
static const WCHAR g_uri_battery_rom_dump[] = L"/a/battery.rom";
static const WCHAR g_msg_state_main[] = L"Dump battery ROM to the \"/a/battery.rom\" file?";
static const WCHAR g_msg_state_dump_ok[] =  L"The battery ROM has been dumped to \"/a/battery.rom\" file!";
static const WCHAR g_msg_state_dump_fail[] = L"Error while dumping battery ROM to \"/a/battery.rom\" file!";

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_NO, HandleEventNo },
	{ EV_YES, HandleEventYes },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_dump_hdls[] = {
	{ EV_DIALOG_DONE, ApplicationStop },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnterMain, HandleStateExit, g_state_main_hdls },
	{ APP_STATE_DUMP_OK, HandleStateEnterDumpOk, HandleStateExit, g_state_dump_hdls },
	{ APP_STATE_DUMP_FAIL, HandleStateEnterDumpFail, HandleStateExit, g_state_dump_hdls }
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
	ELF_T *elf;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		elf = (ELF_T *) APP_InitAppData((void *) APP_HandleEvent, sizeof(ELF_T), reg_id, 0, 1, 1, 1, 1, 0);
		status = APP_Start(ev_st, &elf->app, APP_STATE_MAIN, g_state_table_hdls, ApplicationStop, g_app_name, 0);
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, void *app) {
	UINT32 status;

	status = APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 HandleStateEnterMain(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state) {
	APPLICATION_T *application;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	application = (APPLICATION_T *) app;
	port = application->port;
	UIS_MakeContentFromString("MCq0", &content, g_msg_state_main);

	dialog = UIS_CreateConfirmation(&port, &content);

	if (dialog == 0) {
		return RESULT_FAIL;
	}

	application->dialog = dialog;

	return RESULT_OK;
}

static UINT32 HandleStateEnterDumpOk(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state) {
	APPLICATION_T *application;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	application = (APPLICATION_T *) app;
	port = application->port;
	UIS_MakeContentFromString("MCq0", &content, g_msg_state_dump_ok);

	dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_OK);

	if (dialog == 0) {
		return RESULT_FAIL;
	}

	application->dialog = dialog;

	return RESULT_OK;
}

static UINT32 HandleStateEnterDumpFail(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state) {
	APPLICATION_T *application;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	application = (APPLICATION_T *) app;
	port = application->port;
	UIS_MakeContentFromString("MCq0", &content, g_msg_state_dump_fail);

	dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_FAIL);

	if (dialog == 0) {
		return RESULT_FAIL;
	}

	application->dialog = dialog;

	return RESULT_OK;
}

static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, void *app, EXIT_STATE_TYPE_T state) {
	APPLICATION_T *application;

	application = (APPLICATION_T *) app;

	if (state == EXIT_STATE_SUSPEND) {
		return RESULT_OK;
	}

	APP_UtilUISDialogDelete(&application->dialog);
	return RESULT_OK;
}

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, void *app) {
	if (DumpBatteryRom() == RESULT_OK) {
		APP_UtilChangeState(APP_STATE_DUMP_OK, ev_st, app);
	} else {
		APP_UtilChangeState(APP_STATE_DUMP_FAIL, ev_st, app);
	}
	return RESULT_OK;
}

static UINT32 HandleEventNo(EVENT_STACK_T *ev_st, void *app) {
	ApplicationStop(ev_st, app);
	return RESULT_OK;
}

static UINT32 DumpBatteryRom(void) {
	UINT32 status;
	UINT32 written;
	FILE rom;
	UINT8 battery_id[HAPI_BATTERY_ROM_UNIQUE_ID_SIZE];
	UINT8 battery_rom[HAPI_BATTERY_ROM_BYTE_SIZE];
	HAPI_BATTERY_ROM_T battery_status;

	battery_status = HAPI_BATTERY_ROM_NONE;
	HAPI_BATTERY_ROM_get_unique_id(battery_id);
	battery_status = HAPI_BATTERY_ROM_read(battery_rom);

	if (DL_FsFFileExist((WCHAR *) g_uri_battery_rom_dump)) {
		status = DL_FsDeleteFile((WCHAR *) g_uri_battery_rom_dump, 0);
		if (status != RESULT_OK)
			return RESULT_FAIL;
	}

	rom = DL_FsOpenFile((WCHAR *) g_uri_battery_rom_dump, FILE_WRITE_MODE, 0);

	/*
	 * Binary battery file format:
	 *    4 bytes = Battery status.
	 *    6 bytes = Battery id.
	 *  128 bytes = Battery ROM.
	 */
	status = DL_FsWriteFile((void *) &battery_status, sizeof(HAPI_BATTERY_ROM_T), 1, rom, &written);
	if (status != RESULT_OK)
		return RESULT_FAIL;
	status = DL_FsWriteFile((void *) &battery_id, HAPI_BATTERY_ROM_UNIQUE_ID_SIZE, 1, rom, &written);
	if (status != RESULT_OK)
		return RESULT_FAIL;
	status = DL_FsWriteFile((void *) &battery_rom, HAPI_BATTERY_ROM_BYTE_SIZE, 1, rom, &written);

	DL_FsCloseFile(rom);

	return status;
}
