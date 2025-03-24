#include <cybiko.h>

typedef struct {
	int width;
	int height;
	int frames;
	int max_compressed_size;
	int bpp;
	int frame_size;
} FBM_HEADER_T;

struct module_t g_main_module;

static bool g_quit = FALSE;
static bool g_focus = FALSE;
static bool g_req_draw = FALSE;

static int fbm_frame = 0;

static void draw(struct FileInput *p_file_input, char *fbm_frame_buffer, FBM_HEADER_T *fbm_head) {
	int size = Graphics_get_bytes_total(g_main_module.m_gfx);
	char *vbuf = Graphics_get_buf_addr(g_main_module.m_gfx);

	if (fbm_frame <= fbm_head->frames) {
		FileInput_read(p_file_input, vbuf, fbm_head->frame_size);
		fbm_frame += 1;
	} else {
		FileInput_seek(p_file_input, 0 + sizeof(FBM_HEADER_T), SEEK_SET);
		fbm_frame = 0;
	}

	/* EXL, 25-Mar-2025: Bitmap 1bpp => 2bpp conversion is too slow on Cybikos :( */
#if 0
	{
		int i;
		int input_index;
		int output_index;

		input_index = output_index = 0;

		for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT / 8; ++i) {
			char byte = fbm_frame_buffer[i];
			char pixels[8];

			// Unpack 8 1bpp pixels
			pixels[0] = (byte & 0x80) ? 0x00 : 0x03;
			pixels[1] = (byte & 0x40) ? 0x00 : 0x03;
			pixels[2] = (byte & 0x20) ? 0x00 : 0x03;
			pixels[3] = (byte & 0x10) ? 0x00 : 0x03;
			pixels[4] = (byte & 0x08) ? 0x00 : 0x03;
			pixels[5] = (byte & 0x04) ? 0x00 : 0x03;
			pixels[6] = (byte & 0x02) ? 0x00 : 0x03;
			pixels[7] = (byte & 0x01) ? 0x00 : 0x03;

			// Pack into 4 bytes (8 pixels -> 4 bytes of 2bpp)
			vbuf[output_index++] = (pixels[0] << 6) | (pixels[1] << 4) | (pixels[2] << 2) | pixels[3];
			vbuf[output_index++] = (pixels[4] << 6) | (pixels[5] << 4) | (pixels[6] << 2) | pixels[7];
		}
	}
#endif

	DisplayGraphics_show(g_main_module.m_gfx);
}

long main(int argc, char *argv[], bool start) {
	static FBM_HEADER_T fbm_head;
	struct FileInput *p_file_input;
	struct MSequence music;
	const char *fbm_frame_buffer;

	init_module(&g_main_module);

	p_file_input = (struct FileInput *) malloc(sizeof(struct FileInput));
	FileInput_ctor_Ex(p_file_input, "BadApple.fbm");
	if (FileInput_is_good(p_file_input)) {
		long file_size = FileInput_get_size(p_file_input);
		if (file_size > 0) {
			FileInput_read(p_file_input, &fbm_head, sizeof(FBM_HEADER_T));
		}
	}

	MSequence_ctor(&music, "title.mus");
	if (MSequence_is_sane(&music)) {
		MSequence_play_background(&music);
	}

	fbm_frame_buffer = (char *) malloc((SCREEN_WIDTH * SCREEN_HEIGHT) / 2);

	AppGeneric_clear_screen();

	while (!g_quit) {
		struct Message *p_msg;
		struct KeyParam *p_key_param;

		p_msg = cWinApp_get_message(g_main_module.m_process, 1, 1, MSG_USER);

		g_req_draw = TRUE;

		switch (p_msg->msgid) {
			case MSG_SHUTUP:
			case MSG_QUIT:
				g_quit = TRUE;
				g_focus = FALSE;
				break;
			case MSG_GOTFOCUS:
				g_focus = TRUE;
				g_req_draw = TRUE;
				if (!MSequence_is_playing(&music)) {
					MSequence_play_background(&music);
				}
				break;
			case MSG_LOSTFOCUS:
				g_focus = FALSE;
				if (MSequence_is_playing(&music)) {
					MSequence_stop(&music);
				}
				break;
			case MSG_KEYDOWN:
				p_key_param = Message_get_key_param(p_msg);
				switch (p_key_param->scancode) {
					case KEY_ESC:
						g_quit = TRUE;
						break;
					default:
						/* Process other keys like "Help", etc. */
						cWinApp_defproc(g_main_module.m_process, p_msg);
						break;
				}
				break;
			default:
				cWinApp_defproc(g_main_module.m_process, p_msg);
				break;
		}

		Message_delete(p_msg);

		if (g_focus && g_req_draw) {
			draw(p_file_input, fbm_frame_buffer, &fbm_head);
		}
	}

	free(fbm_frame_buffer);

	MSequence_dtor(&music, LEAVE_MEMORY);

	FileInput_dtor(p_file_input, FREE_MEMORY);

	return 0L;
}
