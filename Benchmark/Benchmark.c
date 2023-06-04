/*
 * About:
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * Application type:
 *   GUI + UIS Canvas
 *
 * Additional information:
 *
 */

#include <loader.h>
#include <apps.h>
#include <uis.h>
#include <canvas.h>
#include <mem.h>
#include <utilities.h>
#include <dl.h>
#include <dl_keypad.h>

#define TIMER_FAST_TRIGGER_MS             (1)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_TIMER_EXIT = 0x0001,
	APP_TIMER_EXIT_FAST
} APP_TIMER_T;

typedef enum {
	BENCHMARK_MAIN_SCREEN
} BENCHMARK_STATE_T;

typedef struct {
	APPLICATION_T app;

	GRAPHIC_REGION_T area;
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

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 PaintAll(EVENT_STACK_T *ev_st, APPLICATION_T *app, BENCHMARK_STATE_T state);

static const char g_app_name[APP_NAME_LEN] = "Benchmark";

static const COLOR_T g_color_background   = { 0xBB, 0xAD, 0xA0, 0xFF };

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
	{ EV_INK_KEY_PRESS, HandleEventKeyPress },
	{ EV_INK_KEY_RELEASE, HandleEventKeyRelease },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, NULL, g_state_main_hdls }
};

static __inline UINT64 getCycles(void) {
	return suPalReadTime();
}

static __inline UINT64 getMillisecondCounter(void) {
	return suPalTicksToMsec(suPalReadTime());
}

static __inline UINT64 getClockSpeed(void) {
	UINT64 cycles_1;
	UINT64 millis_1;
	UINT64 cycles_2;
	UINT64 millis_2;
	UINT64 millis_e;
	UINT64 result;

	result = 0;
	cycles_1 = getCycles();
	millis_1 = getMillisecondCounter();

	for (;;) {
		int n = 1000000;
		while (--n > 0) {}

		millis_2 = getMillisecondCounter();
		cycles_2 = getCycles();

		millis_e = millis_2 - millis_1;

		if (millis_e > 1000) {
			PFprintf("1: cycles: %llu\n", cycles_1);
			PFprintf("1: millis: %llu\n", millis_1);
			PFprintf("2: cycles: %llu\n", cycles_2);
			PFprintf("2: millis: %llu\n", millis_2);
			PFprintf("e: millis: %llu\n", millis_e);
			return result;
		}
	}
}

#pragma O0
static __inline void BogoMIPS_Delay(int loops) {
	int i;
	for (i = loops; !!(i > 0); --i) {
		;
	}
}
#pragma no_O0

static __inline INT32 BogoMIPS(void) {
	UINT64 loops_per_sec = 1;
	UINT64 ticks;


	ticks = suPalReadTime();

	while ((1)) {
		BogoMIPS_Delay(1);
		ticks = suPalReadTime() - ticks;
		if (ticks >= 160000) {
			UINT64 lps = loops_per_sec;
			lps = (lps / ticks);
			PFprintf("ok - %llu.%02llu BogoMips", lps/500000, (lps/5000) % 100);
			return 1;
		}
	}
	return 0;
}

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
		cprint("Benchmark: Error! Application has already been loaded!\n");
		return NULL;
	}

	status = RESULT_OK;
	ev_code_base = ldrRequestEventBase();
	reserve = ev_code_base + 1;
	reserve = ldrInitEventHandlersTbl(g_state_any_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_init_hdls, reserve);
	reserve = ldrInitEventHandlersTbl(g_state_main_hdls, reserve);

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

	status = RESULT_OK;

	DeleteDialog(app);

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

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	APP_INSTANCE_T *app_instance;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	APP_STATE_T app_state;
	DRAWING_BUFFER_T buffer;
	GRAPHIC_POINT_T point;

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
#if defined(EM2)
			UIS_CanvasGetDisplaySize(&point);
#else
			point = UIS_CanvasGetDisplaySize();
#endif
			buffer.w = point.x + 1;
			buffer.h = point.y + 1;
			buffer.buf = NULL;

			app_instance->area.ulc.x = 0;
			app_instance->area.ulc.y = 0;
			app_instance->area.lrc.x = buffer.w;
			app_instance->area.lrc.y = buffer.h;

			dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);
			break;
		default:
			break;
	}

	if (dialog == DialogType_None) {
		return RESULT_FAIL;
	}

	app->dialog = dialog;

	switch (app_state) {
		case APP_STATE_MAIN:
			PaintAll(ev_st, app, BENCHMARK_MAIN_SCREEN);
			break;
		default:
			break;
	}

	return RESULT_OK;
}

static UINT32 DeleteDialog(APPLICATION_T *app) {
	if (app->dialog != DialogType_None) {
		UIS_Delete(app->dialog);
		app->dialog = DialogType_None;
		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;

	status = RESULT_OK;
	event = AFW_GetEv(ev_st);

	APP_ConsumeEv(ev_st, app);

	switch (event->data.key_pressed) {
		case KEY_1:
			PFprintf("%d\n", *(UINT8 *) 0x3FC3600);
//			PFprintf("%d\n", &hapi_clock_rate_mcu);
//			PFprintf("%d\n", *((UINT8 *) hapi_clock_rate_mcu));
			break;
		case KEY_2:
			PFprintf("%d\n", getClockSpeed());
			break;
		case KEY_3:
			BogoMIPS();
			break;
		case KEY_4:
			break;
		case KEY_5:
			break;
		case KEY_6:
			break;
		case KEY_7:
			break;
		case KEY_8:
			break;
		case KEY_9:
			break;
		default:
			break;
	}

	return status;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;
	EVENT_T *event;

	status = RESULT_OK;
	event = AFW_GetEv(ev_st);

	APP_ConsumeEv(ev_st, app);

	switch (event->data.key_pressed) {
		case KEY_RED:
		case KEY_SOFT_LEFT:
		case KEY_0:
			status |= APP_UtilStartTimer(TIMER_FAST_TRIGGER_MS, APP_TIMER_EXIT, app);
			break;
		default:
			break;
	}

	return status;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	switch (timer_id) {
		case APP_TIMER_EXIT:
			/* No break here. */
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

static UINT32 PaintAll(EVENT_STACK_T *ev_st, APPLICATION_T *app, BENCHMARK_STATE_T state) {
	APP_INSTANCE_T *app_instance;
	GRAPHIC_POINT_T point;
	COLOR_T color;
	WCHAR *text;

	app_instance = (APP_INSTANCE_T *) app;

	switch (state) {
		case BENCHMARK_MAIN_SCREEN:
			UIS_CanvasSetFillColor(g_color_background);
			UIS_CanvasFillRect(app_instance->area, app->dialog);
			color.red = 0x00;
			color.green = 0x00;
			color.blue = 0x00;
			color.transparent = 0x00;
			UIS_CanvasSetForegroundColor(color);
			color.transparent = 0xFF;
			UIS_CanvasSetBackgroundColor(color);
			point.x = 10;
			point.y = 10;
			text = L"Benchmark P2K";
			UIS_CanvasDrawColorText(text, 0, u_strlen(text), point, 0, app->dialog);
			point.x = 10;
			point.y = 35;
			text = L"Press some button!";
			UIS_CanvasDrawColorText(text, 0, u_strlen(text), point, 0, app->dialog);
			break;
		default:
			break;
	}
	return RESULT_OK;
}
