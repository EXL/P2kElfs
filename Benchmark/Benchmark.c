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

RESOURCE_ID resource_icon;

static const unsigned char p2k_app_icon_15x15_gif[603] = {
	0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x0f, 0x00, 0x0f, 0x00, 0xc6, 0x51,
	0x00, 0x90, 0x6f, 0x00, 0x91, 0x6f, 0x01, 0x90, 0x70, 0x00, 0x91, 0x70,
	0x00, 0x92, 0x71, 0x00, 0xa2, 0x7c, 0x01, 0xa8, 0x81, 0x00, 0xac, 0x84,
	0x00, 0xab, 0x85, 0x00, 0xaf, 0x84, 0x01, 0xad, 0x85, 0x00, 0xad, 0x85,
	0x01, 0xad, 0x85, 0x02, 0xac, 0x86, 0x01, 0xae, 0x86, 0x00, 0xae, 0x86,
	0x02, 0xb1, 0x87, 0x00, 0xaf, 0x88, 0x00, 0xc5, 0x98, 0x00, 0xc6, 0x99,
	0x00, 0xc7, 0x99, 0x00, 0xc8, 0x99, 0x01, 0xca, 0x99, 0x00, 0xc7, 0x9a,
	0x01, 0xc8, 0x9a, 0x00, 0xc8, 0x9b, 0x0c, 0xcf, 0x9d, 0x00, 0xca, 0x9d,
	0x2a, 0xcf, 0x9f, 0x00, 0xc9, 0x9e, 0x2e, 0xd2, 0xa0, 0x00, 0xcb, 0xa0,
	0x3a, 0xcc, 0xa0, 0x3f, 0xcc, 0xa3, 0x47, 0xcd, 0xa4, 0x4c, 0xdb, 0xa6,
	0x02, 0xda, 0xa7, 0x00, 0xdb, 0xa8, 0x01, 0xdd, 0xaa, 0x00, 0xe2, 0xac,
	0x01, 0xe2, 0xad, 0x00, 0xd0, 0xab, 0x64, 0xe2, 0xae, 0x00, 0xe3, 0xae,
	0x00, 0xe4, 0xaf, 0x01, 0xd2, 0xb2, 0x75, 0xd3, 0xb3, 0x78, 0xd5, 0xb4,
	0x7f, 0xca, 0xb6, 0x95, 0xd4, 0xb5, 0x7e, 0xd4, 0xb5, 0x7f, 0xd5, 0xb5,
	0x82, 0xd5, 0xb6, 0x80, 0xdb, 0xc2, 0x9a, 0xde, 0xc6, 0xa0, 0xe0, 0xcc,
	0xab, 0xe3, 0xcf, 0xb4, 0xe6, 0xd5, 0xbb, 0xe6, 0xd6, 0xbc, 0xeb, 0xdc,
	0xc9, 0xec, 0xdd, 0xca, 0xeb, 0xe0, 0xce, 0xed, 0xe1, 0xd3, 0xee, 0xe2,
	0xd2, 0xf3, 0xec, 0xe2, 0xf7, 0xf0, 0xea, 0xf6, 0xf1, 0xeb, 0xf6, 0xf3,
	0xec, 0xf8, 0xf3, 0xed, 0xf8, 0xf3, 0xef, 0xf7, 0xf6, 0xf1, 0xfa, 0xf6,
	0xf3, 0xfa, 0xf7, 0xf2, 0xfc, 0xf8, 0xf5, 0xfb, 0xfa, 0xf8, 0xfc, 0xfb,
	0xf9, 0xfd, 0xfb, 0xfc, 0xfd, 0xfc, 0xfa, 0xfe, 0xfd, 0xfb, 0xfe, 0xfe,
	0xfc, 0xfd, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x21, 0xf9, 0x04, 0x01, 0x0a, 0x00, 0x7f, 0x00, 0x2c, 0x00, 0x00,
	0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x07, 0xb8, 0x80, 0x7f, 0x2b,
	0x83, 0x2b, 0x28, 0x83, 0x2c, 0x84, 0x2a, 0x2b, 0x7f, 0x28, 0x26, 0x24,
	0x8f, 0x90, 0x91, 0x24, 0x17, 0x06, 0x2b, 0x25, 0x1e, 0x1c, 0x99, 0x9a,
	0x9a, 0x1a, 0x11, 0x01, 0x27, 0x24, 0x18, 0x18, 0x16, 0x18, 0x15, 0xa2,
	0xa2, 0x13, 0x0e, 0x0e, 0x03, 0x2b, 0xa1, 0x19, 0x38, 0x43, 0x1d, 0xa6,
	0x31, 0x50, 0x34, 0x0c, 0x0c, 0x03, 0x28, 0xa1, 0x3c, 0x44, 0x2d, 0xa2,
	0x20, 0x45, 0x4e, 0x2f, 0x0b, 0x0b, 0xac, 0x24, 0x2e, 0x4d, 0x29, 0xa4,
	0x18, 0x3f, 0x3d, 0x4b, 0x34, 0x08, 0x08, 0xc6, 0x3a, 0x49, 0x47, 0x48,
	0x35, 0x36, 0x42, 0x1f, 0x4e, 0x34, 0x07, 0x07, 0x00, 0x2b, 0x23, 0x3b,
	0x40, 0x21, 0x38, 0x46, 0x48, 0x4f, 0x51, 0xea, 0x30, 0xd2, 0xad, 0x37,
	0x42, 0x22, 0x39, 0x41, 0xa3, 0x18, 0x4c, 0x34, 0x0f, 0x0f, 0xc6, 0x1b,
	0x3e, 0x4a, 0x43, 0x32, 0xa2, 0x16, 0x9a, 0xcc, 0x20, 0x66, 0x8c, 0xde,
	0x29, 0x0c, 0x14, 0x30, 0x4c, 0x68, 0xd0, 0x80, 0x80, 0x8a, 0x49, 0x13,
	0x22, 0x4a, 0x8c, 0x28, 0x81, 0x58, 0x80, 0x15, 0x13, 0x20, 0x38, 0x58,
	0x90, 0x80, 0xd8, 0x82, 0x03, 0xc4, 0x14, 0x14, 0x20, 0xf0, 0xc7, 0x40,
	0x80, 0x01, 0x28, 0x05, 0x0c, 0x08, 0xa0, 0x12, 0x00, 0x01, 0x92, 0x81,
	0x00, 0x00, 0x3b
};

#if defined(EP1)
UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code); /* ElfPack 1.x entry point. */
#elif defined(EP2)
ldrElf *_start(WCHAR *uri, WCHAR *arguments);                                /* ElfPack 2.x entry point. */
#elif defined(EPMCORE)
UINT32 ELF_Entry(ldrElf *elf, WCHAR *arguments);                             /* ElfPack M*CORE entry point. */
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

static const COLOR_T g_color_background   = { 0xFF, 0xFF, 0xFF, 15 };

UINT64 ticks_start = 0LLU;
UINT64 ticks_end = 0LLU;
UINT64 ms_start = 0LLU;
UINT64 ms_end = 0LLU;

#if defined(EP2)
static ldrElf g_app_elf;
#elif defined(EPMCORE)
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

#if __CC_ARM && __arm
#define nop() \
 __asm \
     { \
         nop \
     }
#endif

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

static INT32 BogoMIPSed(void) {
	ticks_start = suPalReadTime();

	BogoMIPS_Delay(1000000);

	ticks_end = suPalReadTime();

	ms_start = suPalTicksToMsec(ticks_start);
	ms_end = suPalTicksToMsec(ticks_end);

	// 18446744073709551615
	// 31679919293464576
	// 699066194405097681

	LOG("\nTICKS: %llu %llu\nMILLIS: %llu %llu\n", ticks_start, ticks_end, ms_start, ms_end);

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
#elif defined(EPMCORE)
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

		DRM_CreateResource(&resource_icon, RES_TYPE_GRAPHICS,
			(void *) p2k_app_icon_15x15_gif, sizeof(p2k_app_icon_15x15_gif));

#if defined(EP2)
		g_app_elf.app = (APPLICATION_T *) app_instance;
#elif defined(EPMCORE)
		g_app_elf->app = &app_instance->app;
#endif
	}

	return status;
}

static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	UINT32 status;

	status = RESULT_OK;

	DeleteDialog(app);

	DRM_ClearResource(resource_icon);

	status |= APP_Exit(ev_st, app, 0);

#if defined(EP1)
	LdrUnloadELF(&Lib);
#elif defined(EP2)
	ldrUnloadElf();
#elif defined(EPMCORE)
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
#if defined(EPMCORE)
			UIS_CanvasGetDisplaySize(&point);
#else
			point = UIS_CanvasGetDisplaySize();
#endif

			PFprintf("Display: %dx%d\n", point.x, point.y);

			buffer.w = point.x + 1;
			buffer.h = point.y + 1;
			buffer.buf = NULL;

			app_instance->area.ulc.x = 0;
			app_instance->area.ulc.y = 0;
			app_instance->area.lrc.x = buffer.w;
			app_instance->area.lrc.y = buffer.h;

			dialog = UIS_CreateColorCanvas(&port, &buffer, TRUE);
			UIS_CreateColorCanvasWithWallpaper(dialog, TRUE);
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
			//BogoMIPS();
			break;
		case KEY_4:
			BogoMIPSed();
			break;
		case KEY_5:
			MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter1.wav");
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

static UINT32 SetWorikingArea(GRAPHIC_REGION_T *working_area) {
	UINT32 status;
	GRAPHIC_REGION_T rect;
	UINT8 count_lines;
	UINT8 chars_on_line;
	UINT8 height_title_bar_end;
	UINT8 height_soft_keys_start;

	status = RESULT_OK;

#if defined(EP1) || defined(EP2)
	UIS_CanvasGetWorkingArea(&rect, &count_lines, &chars_on_line, TITLE_BAR_AREA, TRUE, 1);
	height_title_bar_end = rect.lrc.y + 1;

	UIS_CanvasGetWorkingArea(&rect, &count_lines, &chars_on_line, SOFTKEY_AREA, TRUE, 1);
	height_soft_keys_start = rect.ulc.y - 1;

	rect.ulc.y = height_title_bar_end;
	/* rect.lrc.x -= 1; */
	rect.lrc.y = height_soft_keys_start;
	/* rect.lrc.y += 1; */
#elif defined(EM1) || defined(EM2)
	/* TODO: Fix these values! */
	rect.ulc.x = 0;
	rect.ulc.y = 32;
	rect.lrc.x = 220;
	rect.lrc.y = 150;
#endif

	memcpy(working_area, &rect, sizeof(GRAPHIC_REGION_T));

	return status;
}

static UINT32 PaintAll(EVENT_STACK_T *ev_st, APPLICATION_T *app, BENCHMARK_STATE_T state) {
	APP_INSTANCE_T *app_instance;
	GRAPHIC_POINT_T point;
	GRAPHIC_POINT_T end;
	GRAPHIC_REGION_T r;
	GRAPHIC_REGION_T r4;
	GRAPHIC_METRIC_T string_measure;
	COLOR_T color;
	WCHAR *text;

	app_instance = (APP_INSTANCE_T *) app;

	switch (state) {
		case BENCHMARK_MAIN_SCREEN:
			UIS_CanvasSetFillColor(g_color_background);
			UIS_CanvasFillRect(app_instance->area, app->dialog);
			color.red = 0xFF;
			color.green = 0x00;
			color.blue = 0x00;
			color.transparent = 0x00;
			UIS_CanvasSetForegroundColor(color);
			color.transparent = 0xFF;
			UIS_CanvasSetBackgroundColor(g_color_background);

			UIS_CanvasSetFont(0x09, app->dialog);

			point.x = 10;
			point.y = 10;
			text = L"Benchmark P2K";
			UIS_CanvasDrawColorText(text, 0, u_strlen(text), point, 0, app->dialog);
			point.x = 10;
			point.y = 35;
			text = L"Press some key.";
			UIS_CanvasDrawColorText(text, 0, u_strlen(text), point, 0, app->dialog);

			UIS_CanvasSetLineWidth(1);

			point.y += 20;
			end.x = 170;
			end.y = point.y;

			UIS_CanvasDrawLine(point, end, app->dialog);

			r.ulc.x = 10;
			r.ulc.y = point.y + 20;
			r.lrc.x = 170;
			r.lrc.y = r.ulc.y + 20;

			UIS_CanvasDrawRect(r, TRUE, app->dialog);

			r.ulc.x = 10;
			r.ulc.y = point.y + 80;
			r.lrc.x = 170;
			r.lrc.y = r.ulc.y + 20;

			UIS_CanvasDrawRoundRect(r, 10, 10, TRUE, app->dialog);

			SetWorikingArea(&r4);

			PFprintf("WAREA: %d, %d, %d, %d\n", r4.ulc.x, r4.ulc.y, r4.lrc.x, r4.lrc.y);

			UIS_CanvasGetStringSize(text, &string_measure, 0x01);

			PFprintf("STRM: %d, %d\n", string_measure.height, string_measure.width);

			UIS_CanvasDrawColorSoftkey(L"LOL", 2, TRUE, TRUE, app->dialog);

//			UIS_CanvasDrawTitleBar(L"MY APP", FALSE, 1, FALSE, FALSE, app->dialog);

			UIS_CanvasDrawTitleBarWithIcon(L"MY APP", resource_icon,
				FALSE, 1, FALSE, FALSE, app->dialog, 0, 0);

			break;
		default:
			break;
	}
	return RESULT_OK;
}
