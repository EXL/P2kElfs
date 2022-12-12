/*
 * Application type: GUI.
 */

#include <loader.h>
#include <apps.h>
#include <mme.h>
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
} APP_STATE_T;

typedef enum {
	APP_TIMER_DUMP_OK,
	APP_TIMER_DUMP_FAIL
} APP_TIMER_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventNo(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 DumpBatteryRom(void);
static UINT32 ClearDataArrays(UINT8 *data_arr, UINT32 size);

static const char g_app_name[APP_NAME_LEN] = "BattDump";

static const WCHAR g_uri_battery_rom_dump[] = L"/a/battery.rom";
static const WCHAR g_msg_state_main[] = L"Dump battery ROM to the \"/a/battery.rom\" file?";
static const WCHAR g_msg_state_dump_ok[] =  L"The battery ROM has been dumped to the \"/a/battery.rom\" file!";
static const WCHAR g_msg_state_dump_fail[] = L"Error while dumping battery ROM to the \"/a/battery.rom\" file!";

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DONE, ApplicationStop },
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_NO, HandleEventNo },
	{ EV_YES, HandleEventYes },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_dump_hdls[] = {
	{ EV_DONE, ApplicationStop },
	{ EV_DIALOG_DONE, ApplicationStop },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls },
	{ APP_STATE_DUMP_OK, HandleStateEnter, HandleStateExit, g_state_dump_hdls },
	{ APP_STATE_DUMP_FAIL, HandleStateEnter, HandleStateExit, g_state_dump_hdls }
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

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	DeleteDialog(app);

	status = APP_Exit(ev_st, app, 0);

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	DeleteDialog(app);

	port = app->port;
	app_state = app->state;

	switch (app_state) {
		case APP_STATE_MAIN:
			UIS_MakeContentFromString("MCq0", &content, g_msg_state_main);
			dialog = UIS_CreateConfirmation(&port, &content);
			break;
		case APP_STATE_DUMP_OK:
			UIS_MakeContentFromString("RMq0", &content, g_msg_state_dump_ok);
			dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_OK);
			break;
		case APP_STATE_DUMP_FAIL:
			UIS_MakeContentFromString("RMq0", &content, g_msg_state_dump_fail);
			dialog = UIS_CreateTransientNotice(&port, &content, NOTICE_TYPE_FAIL);
			break;
		default:
			dialog = DialogType_None;
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

static UINT32 HandleEventYes(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	if (DumpBatteryRom() == RESULT_OK) {
		APP_UtilChangeState(APP_STATE_DUMP_OK, ev_st, app);
		APP_UtilStartTimer(100, APP_TIMER_DUMP_OK, app);
	} else {
		APP_UtilChangeState(APP_STATE_DUMP_FAIL, ev_st, app);
		APP_UtilStartTimer(100, APP_TIMER_DUMP_FAIL, app);
	}
	return RESULT_OK;
}

static UINT32 HandleEventNo(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	ApplicationStop(ev_st, app);
	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	if (timer_id == APP_TIMER_DUMP_OK) {
		/* Play a normal camera shutter sound using loud speaker. */
		MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter5.amr");
	} else if (timer_id == APP_TIMER_DUMP_FAIL) {
		/* Play an error sound using quiet speaker. */
		DL_AudPlayTone(0x02,  0xFF);
	}

	return RESULT_OK;
}

static UINT32 DumpBatteryRom(void) {
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
	status |= DL_FsWriteFile((void *) &battery_status, sizeof(HAPI_BATTERY_ROM_T), 1, rom, &written);
	status |= DL_FsWriteFile((void *) &battery_id, HAPI_BATTERY_ROM_UNIQUE_ID_SIZE, 1, rom, &written);
	status |= DL_FsWriteFile((void *) &battery_rom, HAPI_BATTERY_ROM_BYTE_SIZE, 1, rom, &written);

	DL_FsCloseFile(rom);

	return status;
}

static UINT32 ClearDataArrays(UINT8 *data_arr, UINT32 size) {
	UINT32 i;
	for (i = 0; i < size; ++i) {
		data_arr[i] = 0;
	}
	return RESULT_OK;
}
