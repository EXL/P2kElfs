/*
 * Application type: Daemon.
 */

#include <apps.h>
#include <mme.h>
#include <dl.h>

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
	APP_TIMER_ATTACH,
	APP_TIMER_DETACH,
	APP_TIMER_EXIT
} APP_TIMER_T;

typedef enum {
	JANUARY = 1,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
} MONTH_T;

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, void *app);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state);

static UINT32 HandleEventDeviceAttach(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventDeviceDetach(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, void *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, void *app);

static UINT32 SendPowerAlertSms(void *app, BOOL power);
static UINT32 GeneratePowerAlert(WCHAR *alert, BOOL power);
static const char *GetStringMonth(const UINT32 month);

static const char g_app_name[APP_NAME_LEN] = "PowerAlert";

static const WCHAR *g_sms_alert_phone = L"+7913XXXXXXX";
static const char g_sms_alert_template[] = "Power: %s\nDate: %02d-%s-%04d\nTime: %02d:%02d:%02d";
static const char *g_pwr_on = "ON";
static const char *g_pwr_off = "OFF";

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
	{ APP_STATE_MAIN, HandleStateEnter, NULL, g_state_main_hdls }
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

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, void *app, ENTER_STATE_TYPE_T state) {
	return RESULT_OK;
}

static UINT32 HandleEventDeviceAttach(EVENT_STACK_T *ev_st, void *app) {
	/* Not headset or earphones. */
	if (!DL_AccIsHeadsetAvailable()) {
		g_is_earphones = FALSE;
		APP_UtilStartTimer(100, APP_TIMER_ATTACH, app);
	} else {
		g_is_earphones = TRUE;
	}
	return RESULT_OK;
}
static UINT32 HandleEventDeviceDetach(EVENT_STACK_T *ev_st, void *app) {
	if (!g_is_earphones) {
		APP_UtilStartTimer(100, APP_TIMER_DETACH, app);
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
		/* Start vibration motor on R3443H (L6i). */
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

	if (timer_id == APP_TIMER_ATTACH) {
		/* Play a normal camera shutter sound using loud speaker. */
		MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter1.amr");

		/* Send SMS with Power Status: ON. */
		SendPowerAlertSms(app, TRUE);
	} else if (timer_id == APP_TIMER_DETACH) {
		/* Play a normal camera shutter sound using loud speaker. */
		MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter2.amr");

		/* Send SMS with Power Status: OFF. */
		SendPowerAlertSms(app, FALSE);
	} else if (timer_id == APP_TIMER_STOP_VIBRATION) {
		/* Stop vibration motor on R3443H (L6i). */
		hPortWrite(735, 0);
	} else if (timer_id == APP_TIMER_EXIT) {
		/* Play an exit sound using quiet speaker. */
		DL_AudPlayTone(0x00,  0xFF);

		/* Exit App! */
		return ApplicationStop(ev_st, app);
	}

	return RESULT_OK;
}

static UINT32 SendPowerAlertSms(void *app, BOOL power) {
	UINT32 status;
	IFACE_DATA_T iface_data;
	SEND_TEXT_MESSAGE_T send_message;

	status = RESULT_OK;
	iface_data.port = ((APPLICATION_T *) app)->port;
	send_message.addr_type = 0;

	status |= (u_strcpy(send_message.address, (WCHAR *) g_sms_alert_phone) == NULL);
	status |= (u_strcpy(send_message.contents, (WCHAR *) g_sms_alert_phone) == NULL);
	status |= GeneratePowerAlert(send_message.contents, power);
	status |= DL_SigMsgSendTextMsgReq(&iface_data, &send_message);

	return status;
}

static UINT32 GeneratePowerAlert(WCHAR *alert, BOOL power) {
	UINT32 status;
	CLK_DATE_T date;
	CLK_TIME_T time;
	const char *pwr;
	char message[sizeof(g_sms_alert_template) + 1];

	status = RESULT_OK;
	pwr = (power) ? g_pwr_on : g_pwr_off;

	status |= (DL_ClkGetDate(&date) == FALSE);
	status |= (DL_ClkGetTime(&time) == FALSE);
	status |= (sprintf(message, g_sms_alert_template, pwr, date.day, GetStringMonth(date.month), date.year,
		time.hour, time.minute, time.second) < 0);
	status |= (u_atou(message, alert) == NULL);

	UtilLogStringData(message);

	return status;
}

static const char *GetStringMonth(const UINT32 month) {
	switch (month) {
		case JANUARY:   return "Jan";
		case FEBRUARY:  return "Feb";
		case MARCH:     return "Mar";
		case APRIL:     return "Apr";
		case MAY:       return "May";
		case JUNE:      return "Jun";
		case JULY:      return "Jul";
		case AUGUST:    return "Aug";
		case SEPTEMBER: return "Sep";
		case OCTOBER:   return "Oct";
		case NOVEMBER:  return "Nov";
		case DECEMBER:  return "Dec";
		default:        return "Err";
	}
}
