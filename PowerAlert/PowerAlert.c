/*
 * Application type: Daemon.
 */

#include <apps.h>
#include <mme.h>

typedef struct {
	APPLICATION_T app;
} ELF_T;

typedef enum {
	APP_STATE_ANY,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_STOP_VIBRATION,
	APP_TIMER_PLAY_ATTACH_SOUND,
	APP_TIMER_PLAY_DETACH_SOUND,
	APP_TIMER_EXIT
} APP_TIMER_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, void *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, void *app, EXIT_STATE_TYPE_T state);

static UINT32 HandleEventDeviceAttach(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventDeviceDetach(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, void *app);

static const char g_app_name[APP_NAME_LEN] = "PowerAlert";

static const UINT8 g_key_exit = KEY_0;
static const UINT8 g_key_vibration = KEY_STAR;

static UINT64 g_ms_key_press_start = 0LLU;
static BOOL g_is_earphones = TRUE;

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_DEVICE_ATTACH, HandleEventDeviceAttach },
	{ EV_DEVICE_DETACH, HandleEventDeviceDetach },
	{ EV_KEY_PRESS, HandleEventKeyPress },
	{ EV_KEY_RELEASE, HandleEventKeyRelease },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls }
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
		elf = (ELF_T *) APP_InitAppData((void *) APP_HandleEventPrepost, sizeof(ELF_T), reg_id, 0, 1, 1, 2, 0, 0);
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

// TODO: Can I delete this?
static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state) {
	return RESULT_OK;
}

// TODO: Can I delete this?
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, void *app, EXIT_STATE_TYPE_T state) {
	return RESULT_OK;
}

static UINT32 HandleEventDeviceAttach(EVENT_STACK_T *ev_st, void *app) {
	/* Not headset or earphones. */
	if (!DL_AccIsHeadsetAvailable()) {
		g_is_earphones = FALSE;
		APP_UtilStartTimer(100, APP_TIMER_PLAY_ATTACH_SOUND, app);
	} else {
		g_is_earphones = TRUE;
	}
	return RESULT_OK;
}
static UINT32 HandleEventDeviceDetach(EVENT_STACK_T *ev_st, void *app) {
	if (!g_is_earphones) {
		APP_UtilStartTimer(100, APP_TIMER_PLAY_DETACH_SOUND, app);
	}
	return RESULT_OK;
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, void *app) {
	EVENT_T *event;
	UINT8 key;

	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_exit) {
		g_ms_key_press_start = suPalTicksToMsec(suPalReadTime());
	}

	return RESULT_OK;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, void *app) {
	EVENT_T *event;
	UINT8 key;
	UINT32 ms_key_release_stop;

	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_exit) {
		/*
		 * Detect long key press between 500 ms (0.5 s) and 1500 ms (1.5 s) and ignore rest.
		 */
		ms_key_release_stop = (UINT32) (suPalTicksToMsec(suPalReadTime()) - g_ms_key_press_start);

		if ((ms_key_release_stop >= 500) && (ms_key_release_stop <= 1500)) {
			if (key == g_key_exit) {
				APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
			}
		}
	} else if (key == g_key_vibration) {
		/* Start vibration on R3443H (L6i). */
		hPortWrite(735, 1);
		APP_UtilStartTimer(30, APP_TIMER_STOP_VIBRATION, app);
	}

	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, void *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	if (timer_id == APP_TIMER_PLAY_ATTACH_SOUND) {
		MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter1.amr");
	} else if (timer_id == APP_TIMER_PLAY_DETACH_SOUND) {
		MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter2.amr");
	} else if (timer_id == APP_TIMER_STOP_VIBRATION) {
		/* Stop vibration on R3443H (L6i). */
		hPortWrite(735, 0);
	} else if (timer_id == APP_TIMER_EXIT) {
		/* Play an exit sound using quiet speaker. */
		DL_AudPlayTone(0x00,  0xFF);

		/* Exit App! */
		return ApplicationStop(ev_st, app);
	}

	return RESULT_OK;
}
