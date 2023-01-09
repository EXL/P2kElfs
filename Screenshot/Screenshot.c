/*
 * About:
 *   The "Screenshot" ELF daemon utility with GUI to fast take screenshots.
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * Application type:
 *   GUI + Deamon
 */

#include <loader.h>
#include <apps.h>
#include <ati.h>
#include <mme.h>
#include <mem.h>
#include <dal.h>
#include <uis.h>
#include <utilities.h>

#ifndef __P2K__
#define __packed
#endif

#define KEY_LONG_PRESS_START_MS    (500)
#define KEY_LONG_PRESS_STOP_MS     (1500)

typedef enum {
	APP_STATE_ANY,
	APP_STATE_INIT,
	APP_STATE_MAIN,
	APP_STATE_MAX
} APP_STATE_T;

typedef enum {
	APP_DISPLAY_HIDE,
	APP_DISPLAY_SHOW
} APP_DISPLAY_T;

typedef enum {
	APP_TIMER_SCREEN_OK,
	APP_TIMER_SCREEN_FAIL,
	APP_TIMER_EXIT
} APP_TIMER_T;

typedef enum {
	APP_RESOURCE_STRING_GOT_IT,
	APP_RESOURCE_ACTION_GOT_IT,
	APP_RESOURCE_MAX
} APP_RESOURCES_T;

typedef struct {
	APPLICATION_T app;

	APP_DISPLAY_T state;
	RESOURCE_ID resources[APP_RESOURCE_MAX];
	UINT64 ms_key_press_start;
} APP_INSTANCE_T;

typedef struct {
	UINT32 width;
	UINT32 height;
	UINT32 pixels;  /* Count of pixels */
	UINT32 bpp;     /* Bytes per pixel. */
	UINT32 size;    /* In bytes. */
	UINT16 *buffer;
} BITMAP_T;

typedef enum {
	BMP_OFFSET_SIZE_FILE   = 0x02,
	BMP_OFFSET_WIDTH       = 0x12,
	BMP_OFFSET_HEIGHT      = 0x16,
	BMP_OFFSET_SIZE_BITMAP = 0x22
} BMP_OFFSET_T;

static __inline UINT16 SwapUINT16(UINT16 value);
static __inline UINT32 SwapUINT32(UINT32 value);
static __inline void InsertData(UINT8 *start_address, UINT32 start_offset, UINT32 value);

UINT32 Register(const char *elf_path_uri, const char *args, UINT32 ev_code);
static UINT32 ApplicationStart(EVENT_STACK_T *ev_st, REG_ID_T reg_id, void *reg_hdl);
static UINT32 ApplicationStop(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 ApplicationDisplay(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_DISPLAY_T display);

static UINT32 InitResourses(RESOURCE_ID *resources);
static UINT32 FreeResourses(RESOURCE_ID *resources);

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state);
static UINT32 HandleStateExit(EVENT_STACK_T *ev_st, APPLICATION_T *app, EXIT_STATE_TYPE_T state);
static UINT32 DeleteDialog(APPLICATION_T *app);

static void HandleEventMain(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id);
static UINT32 HandleEventHide(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventShow(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app);
static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app);

static UINT32 MakeScreenshot(void);
static UINT32 CopyVramToRamAndInitBitmap(BITMAP_T *bitmap);
static UINT32 CreateConvertedBitmap(BITMAP_T *bitmap);
static UINT32 PatchBmpHeader(const BITMAP_T *bitmap);
static UINT32 SaveScreenshotFile(const BITMAP_T *bitmap);
static UINT32 GenerateScreenshotFilePath(WCHAR *output_path);

static const char g_app_name[APP_NAME_LEN] = "Screenshot";

static const UINT8 g_key_exit = KEY_STAR;
static const UINT8 g_key_help = KEY_0;
static const UINT8 g_key_screenshot = KEY_POUND;

static const WCHAR g_msg_state_main[] = L"Hold \"#\" to Screenshot!\nHold \"0\" to Help.\nHold \"*\" to Exit.";
static const WCHAR g_msg_softkey_got_it[] = L"Got it!";

static const char g_scr_filename_template[] = "/c/mobile/picture/SCR_%02d%02d%04d_%02d%02d%02d.bmp";

/*
 * The 0xFF bytes are for later replacement.
 * Information: https://en.wikipedia.org/wiki/BMP_file_format
 *    14 bytes: Bitmap file header.
 *    56 bytes: Bitmap information header (DIB header).
 *     Version: BITMAPV3INFOHEADER
 */
static UINT8 g_bmp_header[70] = {
	0x42, 0x4D, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00,
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const EVENT_HANDLER_ENTRY_T g_state_any_hdls[] = {
	{ EV_REVOKE_TOKEN, APP_HandleUITokenRevoked },
	{ EV_KEY_PRESS, HandleEventKeyPress },
	{ EV_KEY_RELEASE, HandleEventKeyRelease },
	{ EV_TIMER_EXPIRED, HandleEventTimerExpired },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_init_hdls[] = {
	/*
	 * EXL, 01-Dec-2022:
	 *  Please comment out the `{ EV_GRANT_TOKEN, APP_HandleUITokenGranted }` construction
	 *  then change `app_instance->state` variable to `APP_DISPLAY_HIDE` value
	 *  then change `routing_stack` argument in `APP_InitAppData()` function to `APP_DISPLAY_HIDE` value
	 *  for pure daemon behavior with no GUI at start.
	 */
	{ EV_GRANT_TOKEN, APP_HandleUITokenGranted },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T g_state_main_hdls[] = {
	{ EV_DONE, HandleEventHide },
	{ EV_DIALOG_DONE, HandleEventHide },
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T g_state_table_hdls[] = {
	{ APP_STATE_ANY, NULL, NULL, g_state_any_hdls },
	{ APP_STATE_INIT, NULL, NULL, g_state_init_hdls },
	{ APP_STATE_MAIN, HandleStateEnter, HandleStateExit, g_state_main_hdls }
};

static __inline UINT16 SwapUINT16(UINT16 value) {
	return ((value >>  8) | (value << 8));
}

static __inline UINT32 SwapUINT32(UINT32 value) {
	return ((value >> 24) | ((value & 0x00FF0000) >> 8) | ((value & 0x0000FF00) << 8) | (value << 24));
}

static __inline void InsertData(UINT8 *start_address, UINT32 start_offset, UINT32 value) {
	*((__packed UINT32 *) &start_address[start_offset]) = SwapUINT32(value);

	/*
	 * Alternative way.
	 *
	start_address[start_offset + 0x00] = (value >>  0) & 0x000000FF;
	start_address[start_offset + 0x01] = (value >>  8) & 0x000000FF;
	start_address[start_offset + 0x02] = (value >> 16) & 0x000000FF;
	start_address[start_offset + 0x03] = (value >> 24) & 0x000000FF;
	*/
}

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
	APP_INSTANCE_T *app_instance;

	status = RESULT_FAIL;

	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		app_instance = (APP_INSTANCE_T *) APP_InitAppData((void *) HandleEventMain, sizeof(APP_INSTANCE_T),
			reg_id, 0, 1, 1, 1, APP_DISPLAY_SHOW, 0);

		InitResourses(app_instance->resources);
		app_instance->state = APP_DISPLAY_SHOW;
		app_instance->ms_key_press_start = 0LLU;

		status = APP_Start(ev_st, &app_instance->app, APP_STATE_MAIN,
			g_state_table_hdls, HandleEventHide, g_app_name, 0);
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

	LdrUnloadELF(&Lib);

	return status;
}

static UINT32 InitResourses(RESOURCE_ID *resources) {
	UINT32 status;
	RES_ACTION_LIST_ITEM_T action;

	status = RESULT_OK;

	status |= DRM_CreateResource(&resources[APP_RESOURCE_STRING_GOT_IT], RES_TYPE_STRING,
		(WCHAR *) g_msg_softkey_got_it, (u_strlen(g_msg_softkey_got_it) + 1) * sizeof(WCHAR));

	action.softkey_label = resources[APP_RESOURCE_STRING_GOT_IT];
	action.list_label = resources[APP_RESOURCE_STRING_GOT_IT];
	action.softkey_priority = 1;
	action.list_priority = 1;
	action.isExit = FALSE;
	action.sendDlgDone = FALSE;
	status |= DRM_CreateResource(&resources[APP_RESOURCE_ACTION_GOT_IT], RES_TYPE_ACTION, &action, sizeof(action));

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

static UINT32 ApplicationDisplay(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_DISPLAY_T display) {
	UINT32 status;
	APP_INSTANCE_T *app_instance;
	void *hdl;
	UINT32 routing_stack;

	status = RESULT_OK;
	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->state != display) {
		app_instance->state = display;
		hdl = (void *) ((app_instance->state == APP_DISPLAY_SHOW) ? APP_HandleEvent : APP_HandleEventPrepost);
		routing_stack = (app_instance->state == APP_DISPLAY_SHOW);
		status = APP_ChangeRoutingStack(app, ev_st, hdl, routing_stack, 0, 1, 1);
	}

	return status;
}

static UINT32 HandleStateEnter(EVENT_STACK_T *ev_st, APPLICATION_T *app, ENTER_STATE_TYPE_T state) {
	APP_INSTANCE_T *app_instance;
	SU_PORT_T port;
	CONTENT_T content;
	UIS_DIALOG_T dialog;
	ACTIONS_T actions;

	if (state != ENTER_STATE_ENTER) {
		return RESULT_OK;
	}

	app_instance = (APP_INSTANCE_T *) app;

	DeleteDialog(app);

	port = app->port;

	actions.action[0].operation = ACTION_OP_ADD;
	actions.action[0].event = EV_DIALOG_DONE;
	actions.action[0].action_res = app_instance->resources[APP_RESOURCE_ACTION_GOT_IT];
	actions.count = 1;

	UIS_MakeContentFromString("RMq0", &content, g_msg_state_main);

	dialog = UIS_CreateNotice(&port, &content, 0, NOTICE_TYPE_DEFAULT, FALSE, &actions);

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

static void HandleEventMain(EVENT_STACK_T *ev_st, APPLICATION_T *app, APP_ID_T app_id, REG_ID_T reg_id) {
	APP_INSTANCE_T *app_instance;

	app_instance = (APP_INSTANCE_T *) app;

	if (app_instance->state == APP_DISPLAY_SHOW) {
		APP_HandleEvent(ev_st, app, app_id, reg_id);
	} else {
		APP_HandleEventPrepost(ev_st, app, app_id, reg_id);
	}
}

static UINT32 HandleEventHide(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	ApplicationDisplay(ev_st, app, APP_DISPLAY_HIDE);
	return RESULT_OK;
}

static UINT32 HandleEventShow(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	if (app->state != APP_STATE_MAIN) {
		APP_UtilChangeState(APP_STATE_MAIN, ev_st, app);
	}

	ApplicationDisplay(ev_st, app, APP_DISPLAY_SHOW);

	return RESULT_OK;
}

static UINT32 HandleEventKeyPress(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT8 key;

	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_exit || key == g_key_help || key == g_key_screenshot) {
		app_instance->ms_key_press_start = suPalTicksToMsec(suPalReadTime());
		PFprintf("display_source_buffer addr = 0x%08X.\n", display_source_buffer); /* Send to MIDway. */
		UtilLogStringData("display_source_buffer addr = 0x%08X.\n", display_source_buffer); /* Send to P2KDataLogger. */
		if (key == g_key_screenshot) {
			return APP_ConsumeEv(ev_st, app);
		}
	}

	return RESULT_OK;
}

static UINT32 HandleEventKeyRelease(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	APP_INSTANCE_T *app_instance;
	EVENT_T *event;
	UINT8 key;
	UINT32 ms_key_release_stop;

	app_instance = (APP_INSTANCE_T *) app;
	event = AFW_GetEv(ev_st);
	key = event->data.key_pressed;

	if (key == g_key_exit || key == g_key_help || key == g_key_screenshot) {
		/*
		 * Detect long key press between 500 ms (0.5 s) and 1500 ms (1.5 s) and ignore rest.
		 */
		ms_key_release_stop = (UINT32) (suPalTicksToMsec(suPalReadTime()) - app_instance->ms_key_press_start);
		if ((ms_key_release_stop >= KEY_LONG_PRESS_START_MS) && (ms_key_release_stop <= KEY_LONG_PRESS_STOP_MS)) {
			if (key == g_key_exit) {
				APP_UtilStartTimer(100, APP_TIMER_EXIT, app);
			} else if (key == g_key_help) {
				return HandleEventShow(ev_st, app);
			} else if (g_key_screenshot) {
				if (MakeScreenshot() == RESULT_OK) {
					APP_UtilStartTimer(100, APP_TIMER_SCREEN_OK, app);
				} else {
					APP_UtilStartTimer(100, APP_TIMER_SCREEN_FAIL, app);
				}
				return APP_ConsumeEv(ev_st, app);
			}
		}
	}

	return RESULT_OK;
}

static UINT32 HandleEventTimerExpired(EVENT_STACK_T *ev_st, APPLICATION_T *app) {
	EVENT_T *event;
	APP_TIMER_T timer_id;

	event = AFW_GetEv(ev_st);
	timer_id = ((DL_TIMER_DATA_T *) event->attachment)->ID;

	if (timer_id == APP_TIMER_SCREEN_OK) {
		/* Play a normal camera shutter sound using loud speaker. */
		MME_GC_playback_open_audio_play_forget(L"/a/mobile/system/shutter5.amr");
	} else if (timer_id == APP_TIMER_SCREEN_FAIL) {
		/* Play an error sound using quiet speaker. */
		DL_AudPlayTone(0x02,  0xFF);
	} else if (timer_id == APP_TIMER_EXIT) {
		/* Play an exit sound using quiet speaker. */
		DL_AudPlayTone(0x00,  0xFF);

		/* Exit App! */
		return ApplicationStop(ev_st, app);
	}

	return RESULT_OK;
}

static UINT32 MakeScreenshot(void) {
	UINT32 status;
	BITMAP_T bitmap;

	status = RESULT_OK;
	status |= CopyVramToRamAndInitBitmap(&bitmap);
	status |= CreateConvertedBitmap(&bitmap);
	status |= PatchBmpHeader(&bitmap);
	status |= SaveScreenshotFile(&bitmap);

	suFreeMem(bitmap.buffer);

	return status;
}

static UINT32 CopyVramToRamAndInitBitmap(BITMAP_T *bitmap) {
	UINT32 status;
	AHIDEVCONTEXT_T ahi_device_context;
	AHISURFACE_T ahi_surface;
	AHISURFINFO_T ahi_surface_info;
	AHIBITMAP_T ahi_bitmap;
	AHIRECT_T ahi_rect;
	AHIPOINT_T ahi_point;

	status = RESULT_OK;

	ahi_device_context = DAL_GetDeviceContext(DISPLAY_MAIN);
	ahi_surface = DAL_GetDrawingSurface(DISPLAY_MAIN);

	status |= AhiSurfInfo(ahi_device_context, ahi_surface, &ahi_surface_info);
	bitmap->width = ahi_surface_info.width;
	bitmap->height = ahi_surface_info.height;
	bitmap->pixels = bitmap->width * bitmap->height;
	bitmap->bpp = ahi_surface_info.byteSize / bitmap->pixels;
	bitmap->size = bitmap->pixels * bitmap->bpp;

	ahi_bitmap.width = bitmap->width;
	ahi_bitmap.height = bitmap->height;
	ahi_bitmap.stride = bitmap->width * bitmap->bpp;
	ahi_bitmap.format = AHIFMT_16BPP_565;
	ahi_bitmap.image  = (void *) display_source_buffer;

	ahi_rect.x1 = 0;
	ahi_rect.y1 = 0;
	ahi_rect.x2 = 0 + bitmap->width;
	ahi_rect.y2 = 0 + bitmap->height;

	ahi_point.x = 0;
	ahi_point.y = 0;

	status |= AhiSurfCopy(ahi_device_context, ahi_surface, &ahi_bitmap, &ahi_rect, &ahi_point, 0, 1);

	return status;
}

static UINT32 CreateConvertedBitmap(BITMAP_T *bitmap) {
	UINT32 status;
	INT32 i;
	INT32 x;
	INT32 y;
	UINT16 *address;
	UINT16 bitmap_pixel;

	bitmap->buffer = (UINT16 *) suAllocMem(bitmap->size, (INT32 *) &status);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}

	x = bitmap->width;
	y = 0;
	for (i = bitmap->pixels - 1, address = (void *) display_source_buffer; i >= 0; --i, ++address) {
		bitmap_pixel = SwapUINT16(*address);
		bitmap->buffer[bitmap->pixels - x - (y * bitmap->width)] = bitmap_pixel;
		--x;
		if (x == 0) {
			x = bitmap->width;
			++y;
		}
	}

	return status;
}

static UINT32 PatchBmpHeader(const BITMAP_T *bitmap) {
	UINT32 bmp_file_size;

	bmp_file_size = sizeof(g_bmp_header) + bitmap->size;

	InsertData(g_bmp_header, BMP_OFFSET_SIZE_FILE, bmp_file_size);
	InsertData(g_bmp_header, BMP_OFFSET_WIDTH, bitmap->width);
	InsertData(g_bmp_header, BMP_OFFSET_HEIGHT, bitmap->height);
	InsertData(g_bmp_header, BMP_OFFSET_SIZE_BITMAP, bitmap->size);

	return RESULT_OK;
}

static UINT32 SaveScreenshotFile(const BITMAP_T *bitmap) {
	UINT32 status;
	FILE screenshot_file;
	UINT32 written_bytes;
	WCHAR screenshot_path[sizeof(g_scr_filename_template) * sizeof(WCHAR) + 1];

	status = RESULT_OK;
	written_bytes = 0;

	status |= GenerateScreenshotFilePath(screenshot_path);

	screenshot_file = DL_FsOpenFile(screenshot_path, FILE_WRITE_MODE, 0);
	if (screenshot_file == NULL) {
		return RESULT_FAIL;
	}

	status |= DL_FsWriteFile(g_bmp_header, sizeof(g_bmp_header), 1, screenshot_file, &written_bytes);
	if (written_bytes == 0) {
		status |= RESULT_FAIL;
	}

	written_bytes = 0;
	status |= DL_FsWriteFile(bitmap->buffer, bitmap->size, 1, screenshot_file, &written_bytes);
	if (written_bytes == 0) {
		status |= RESULT_FAIL;
	}

	status |= DL_FsCloseFile(screenshot_file);

	return status;
}

static UINT32 GenerateScreenshotFilePath(WCHAR *output_path) {
	UINT32 status;
	CLK_DATE_T date;
	CLK_TIME_T time;
	char path[sizeof(g_scr_filename_template) + 1];

	status = RESULT_OK;
	status |= (DL_ClkGetDate(&date) == FALSE);
	status |= (DL_ClkGetTime(&time) == FALSE);
	status |= (sprintf(path, g_scr_filename_template, date.day, date.month, date.year,
		time.hour, time.minute, time.second) < 0);
	status |= (u_atou(path, output_path) == NULL);

	return status;
}
