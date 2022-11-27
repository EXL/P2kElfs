#include <apps.h>
#include <ati.h>

typedef struct {
	APPLICATION_T application;
} APP_SCREENSHOT_T;

typedef enum {
	APP_SCREENSHOT_STATE_ANY,
	APP_SCREENSHOT_STATE_MAIN,
	APP_SCREENSHOT_STATE_MAX
} APP_SCREENSHOT_STATES_T;

static const char application_name[APP_NAME_LEN] = "Screenshot";
static BOOL destroy_application_status = FALSE;

static UINT8 bmp_header[] = {
	0x42, 0x4D, 0x46, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00,
	0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xA0, 0x00,
	0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xA0,
	0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07,
	0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static UINT32 copy_ati_display_vram_to_ram(void) {
	AHIBITMAP_T     ahi_bitmap;
	AHIPOINT_T      ahi_point;
	AHIRECT_T       ahi_rectangle;
	AHIDEVCONTEXT_T ahi_device_context = DAL_GetDeviceContext(DISPLAY_MAIN);
	AHISURFACE_T    ahi_surface        = DAL_GetDrawingSurface(DISPLAY_MAIN);

	ahi_bitmap.width  = 128;
	ahi_bitmap.height = 160;
	ahi_bitmap.stride = 128 * 2;
	ahi_bitmap.format = AHIFMT_16BPP_565;
	ahi_bitmap.image  = (void *) 0x12200254;

	ahi_rectangle.x1 = 0;
	ahi_rectangle.y1 = 0;
	ahi_rectangle.x2 = 0 + 128;
	ahi_rectangle.y2 = 0 + 160;

	ahi_point.x = 0;
	ahi_point.y = 0;

	return AhiSurfCopy(ahi_device_context, ahi_surface, &ahi_bitmap, &ahi_rectangle, &ahi_point, NULL, 1);
}

// TODO: Refactoring here.
static UINT16 *create_converted_bitmap(void) {
	INT32 result;
	UINT16 bitmap_pixel;
	UINT16 *address;
	UINT16 *bitmap_buffer;
	INT32 line;
	INT32 stroke;
	INT32 ppp;
	INT32 pixel;

	// TODO: Check Alloca prototype and return value.
	bitmap_buffer = (UINT16 *) suAllocMem(128*160*2, &result);
	if (result != RESULT_OK)
		return NULL;

	line = 128;
	stroke = 0;
	ppp = 0;
	for (pixel = 128*160 - 1, address = ((void *) 0x12200254); pixel >= 0; --pixel, ++address) {
		bitmap_pixel = ((*address) << 8) | ((*address) >> 8);
		ppp = 128*160 - line - stroke * 128;

		bitmap_buffer[ppp] = bitmap_pixel;
		line--;
		if (line == 0) {
			line = 128;
			stroke++;
		}
	}

	return bitmap_buffer;
}

static void generate_screenshot_path(WCHAR *output_path) {
	char path_buffer[FILEURI_MAX_LEN + 1];
	CLK_DATE_T date;
	CLK_TIME_T time;

	DL_ClkGetTime(&time);
	DL_ClkGetDate(&date);

	sprintf(
		path_buffer,
		"/c/mobile/picture/SCR_%02d%02d%04d_%02d%02d%02d.bmp",
		date.day,
		date.month,
		date.year,
		time.hour,
		time.minute,
		time.second
	);

	u_atou(path_buffer, output_path);
}

static UINT32 file_ops_close(FILE file, UINT32 result, UINT32 *written_bytes) {
	// TODO: Check return also?
	DL_FsCloseFile(file);

	if (written_bytes != NULL)
		*written_bytes = 0;

	return result;
}

static UINT32 save_screenshot_file(void *bitmap_converted_buffer, UINT32 bitmap_size_bytes) {
	UINT32 written_bytes = 0;
	WCHAR screenshot_path_buffer[FILEURI_MAX_LEN + 1]; // TODO: Half-2?
	FILE screenshot_file = NULL;

	generate_screenshot_path(screenshot_path_buffer);

	screenshot_file = DL_FsOpenFile(screenshot_path_buffer, FILE_WRITE_MODE, 0);
	if (screenshot_file == NULL)
		return RESULT_FAIL;

	// TODO: Check return also?
	DL_FsWriteFile(bmp_header, sizeof(bmp_header), 1, screenshot_file, &written_bytes);
	if (written_bytes == 0)
		return file_ops_close(screenshot_file, RESULT_FAIL, &written_bytes);

	// TODO: Check return also?
	DL_FsWriteFile(bitmap_converted_buffer, bitmap_size_bytes, 1, screenshot_file, &written_bytes);
	if (written_bytes == 0)
		return file_ops_close(screenshot_file, RESULT_FAIL, NULL);

	return file_ops_close(screenshot_file, RESULT_OK, NULL);
}

static UINT32 make_screenshot(void) {
	UINT32 status;
	UINT16 *bitmap_buffer;

	status = copy_ati_display_vram_to_ram();

	bitmap_buffer = create_converted_bitmap();

	status = save_screenshot_file(bitmap_buffer, 128*160*2);

	// TODO: Check Alloca prototype and return value.
	suFreeMem(bitmap_buffer);

	return status;
}

static UINT32 destroy_application(EVENT_STACK_T *event_stack, void *application) {
	UINT32 status = RESULT_OK;
	if (destroy_application_status) {
		status = APP_Exit(event_stack, application, NULL);
		LdrUnloadELF(&Lib);
	}
	return status;
}

static UINT32 handle_key_press(EVENT_STACK_T *event_stack, void *application) {
	EVENT_T *event = AFW_GetEv(event_stack);

	PFprintf("KeyPress = %d.\n", event->data.key_pressed);

	switch (event->data.key_pressed) {
	case KEY_STAR:
		return make_screenshot();
	case KEY_0:
		destroy_application_status = TRUE;
		return destroy_application(event_stack, application);
	default:
		break;
	}
	return RESULT_OK;
}

// TODO: Can I skip this?
static UINT32 state_main_enter(EVENT_STACK_T *event_stack, void *application, ENTER_STATE_TYPE_T state_type) {
	return RESULT_OK;
}

// TODO: Can I skip this?
static void handle_event(EVENT_STACK_T *event_stack, void *application, APP_ID_T app_id, REG_ID_T reg_id) {
	APP_HandleEventPrepost(event_stack, application, app_id, reg_id);
}

static const EVENT_HANDLER_ENTRY_T state_handlers_any[] = {
	{ EV_KEY_PRESS,       handle_key_press },
	{ STATE_HANDLERS_END, NULL }
};

static const EVENT_HANDLER_ENTRY_T state_handlers_main[] = {
	{ STATE_HANDLERS_END, NULL }
};

static const STATE_HANDLERS_ENTRY_T state_handlers_table[] = {
	{ APP_SCREENSHOT_STATE_ANY,  NULL,             NULL, state_handlers_any  },
	{ APP_SCREENSHOT_STATE_MAIN, state_main_enter, NULL, state_handlers_main }
};

static UINT32 start_application(EVENT_STACK_T *event_stack, REG_ID_T reg_id, UINT32 parameter) {
	if (AFW_InquireRoutingStackByRegId(reg_id) != RESULT_OK) {
		APP_SCREENSHOT_T *application = (APP_SCREENSHOT_T *) APP_InitAppData(
			(void *) handle_event,
			sizeof(APP_SCREENSHOT_T),
			reg_id,
			0,
			1,
			1,
			2,
			0,
			0
		);
		return APP_Start(
			event_stack,
			&application->application,
			APP_SCREENSHOT_STATE_MAIN,
			state_handlers_table,
			destroy_application,
			application_name,
			0
		);
	}
	return RESULT_FAIL;
}

// TODO: Const chars ?
UINT32 Register(char *executable_uri, char *arguments, UINT32 event) {
	UINT32 status;
	UINT32 event_code_base;

	// TODO: ?
	event_code_base = event;

	status = APP_Register(
		&event_code_base,
		1,
		state_handlers_table,
		APP_SCREENSHOT_STATE_MAX,
		(void *) start_application
	);

	LdrStartApp(event);
	return status;
}
