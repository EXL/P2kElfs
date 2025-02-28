/*
 * Draft SDL2 version for experiments.
 *
 * Build:
 *  $ gcc BadApple_SDL2.c -o BadApple -lSDL2 -lz
 *
 * Usage:
 *  $ ./BadApple res/BadApple_220p.fbm
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#include <zlib.h>

#define ZLIB_IN_BUF_SIZE				  (4 * 1024)
#define ZLIB_OUT_BUF_SIZE				 (ZLIB_IN_BUF_SIZE * 2)

typedef struct {
	uint16_t width;
	uint16_t height;
	uint16_t frames;
	uint16_t max_compressed_size;
	uint16_t bpp;
	uint16_t frame_size;
} FBM_HEADER_T;

FBM_HEADER_T fbm_head;

uint8_t *fbm_buffer;
uint8_t *zlib_buffer;

static z_stream d_stream;
static FILE *file_handle;

static int quit_loop = 0;
static int fbm_frame = 0;

static SDL_Surface *surface;
static SDL_Surface *video;
static SDL_Renderer *render;
static SDL_Texture *texture;

#define VIEWPORT_INTERVAL 120

static uint32_t aa = 0, bb = 0, delta = 0;

static uint32_t pal[2] = {
	0xFF000000,
	0xFFFFFFFF
};

static int getWidth;
static int getHeight;

void convert_1bpp_to_rgba(const uint8_t *fbm_data, uint32_t *fb, int width, int height, int stride, uint32_t *pal) {
	int total_pixels = width * height;
	for (int pixel_index = 0; pixel_index < total_pixels; pixel_index++) {
		int x = pixel_index % width;
		int y = pixel_index / width;
		int byte_index = y * stride + x / 8;
		int bit_index = x % 8;

		uint8_t pixel_value = (fbm_data[byte_index] >> (7 - bit_index)) & 0x01;
		uint32_t color = pal[pixel_value];

		fb[pixel_index] = color;
	}
}

uint16_t swap_bytes_uint16(uint16_t value) {
	return (value >> 8) | (value << 8);
}

uint32_t swap_bytes_uint32(uint32_t value) {
	return  ((value >> 24) & 0x000000FF) |
			((value >> 8)  & 0x0000FF00) |
			((value << 8)  & 0x00FF0000) |
			((value << 24) & 0xFF000000);
}

static uint32_t GFX_Draw_Step(void) {
	uint32_t zl_size;
	uint32_t readen;

	inflateReset(&d_stream);

	fbm_frame += 1;
	if (fbm_frame > fbm_head.frames) {
		quit_loop = 1;
	}

	fread(&zl_size, sizeof(uint32_t), 1, file_handle);
	zl_size = swap_bytes_uint32(zl_size);
	readen = fread(zlib_buffer, zl_size, 1, file_handle);

	d_stream.next_in = (uint8_t *) zlib_buffer;
	d_stream.avail_in = readen * zl_size;

	d_stream.next_out = (uint8_t *) fbm_buffer;
	d_stream.avail_out = ZLIB_OUT_BUF_SIZE;

	inflate(&d_stream, Z_SYNC_FLUSH);

	return 0;
}

static uint32_t ZLIB_Start(void) {
	int32_t err;
	uint32_t zl_size;
	uint32_t readen;

	zlib_buffer = (uint8_t *) malloc(ZLIB_IN_BUF_SIZE * 10);
	memset(zlib_buffer, 0, ZLIB_IN_BUF_SIZE * 10);

	fread(&zl_size, sizeof(uint32_t), 1, file_handle);
	zl_size = swap_bytes_uint32(zl_size);
	readen = fread(zlib_buffer, zl_size, 1, file_handle);
	d_stream.next_in = (uint8_t *) zlib_buffer;
	d_stream.avail_in = readen * zl_size;
	err = inflateInit2(&d_stream, -MAX_WBITS);
	fprintf(stderr, "inflateInit2 DONE, err = %d, zl_size = %d\n", err, zl_size);

	d_stream.next_out = (uint8_t *) fbm_buffer;
	d_stream.avail_out = ZLIB_OUT_BUF_SIZE;

	fprintf(stderr, "avail_in = %d, next_in = 0x%p\n", d_stream.avail_in, d_stream.next_in);
	fprintf(stderr, "avail_out = %d, next_out = 0x%p\n", d_stream.avail_out, d_stream.next_out);
	err = inflate(&d_stream, Z_SYNC_FLUSH);
	fprintf(stderr, "inflate DONE, err = %d\n", err);
	fprintf(stderr, "avail_in = %d, total_in = %ld\n", d_stream.avail_in, d_stream.total_in);
	fprintf(stderr, "avail_out = %d, total_out = %ld\n", d_stream.avail_out, d_stream.total_out);

	return 0;
}

static uint32_t ZLIB_Stop(void) {
	inflateEnd(&d_stream);

	return 0;
}

static void sdl_handle_events(void) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				quit_loop = 1;
				exit(1);
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE: quit_loop = 1;   break;
				}
				break;
		}
	}

	convert_1bpp_to_rgba(fbm_buffer, (uint32_t *) surface->pixels,
		getWidth, getHeight, fbm_head.frame_size / fbm_head.height, pal);

	SDL_BlitSurface(surface, NULL, video, NULL);
	SDL_UpdateTexture(texture, NULL, video->pixels, video->pitch);
	SDL_RenderCopy(render, texture, NULL, NULL);
	SDL_RenderPresent(render);
}

static void main_loop_step(void) {
	aa = SDL_GetTicks();
	delta = aa - bb;

	if (delta > 1000 / VIEWPORT_INTERVAL) {
		bb = aa;

		GFX_Draw_Step();
		sdl_handle_events();
	}
}

int main(int argc, char *argv[]) {
	file_handle = fopen(argv[1], "rb");
	fread(&fbm_head, sizeof(FBM_HEADER_T), 1, file_handle);

	fbm_head.width = swap_bytes_uint16(fbm_head.width);
	fbm_head.height = swap_bytes_uint16(fbm_head.height);
	fbm_head.frames = swap_bytes_uint16(fbm_head.frames);
	fbm_head.max_compressed_size = swap_bytes_uint16(fbm_head.max_compressed_size);
	fbm_head.bpp = swap_bytes_uint16(fbm_head.bpp);
	fbm_head.frame_size = swap_bytes_uint16(fbm_head.frame_size);

	getWidth = fbm_head.width;
	getHeight = fbm_head.height;

	fbm_buffer = (uint8_t *) malloc(fbm_head.frame_size);
	memset(fbm_buffer, 0, fbm_head.frame_size);

	fprintf(stderr, "fbm_head.width=%d\n", fbm_head.width);
	fprintf(stderr, "fbm_head.height=%d\n", fbm_head.height);
	fprintf(stderr, "fbm_head.frames=%d\n", fbm_head.frames);
	fprintf(stderr, "fbm_head.max_compressed_size=%d\n", fbm_head.max_compressed_size);
	fprintf(stderr, "fbm_head.bpp=%d\n", fbm_head.bpp);
	fprintf(stderr, "fbm_head.frame_size=%d\n", fbm_head.frame_size);

	ZLIB_Start();

	SDL_Window *window = SDL_CreateWindow(
		"Bad Apple",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		getWidth, getHeight,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
		);
	if (window == NULL) {
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (render == NULL) {
		SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	video = SDL_CreateRGBSurface(0, getWidth, getHeight, 32,
								 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (video == NULL) {
		SDL_Log("SDL_CreateRGBSurface (video) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	surface = SDL_CreateRGBSurface(0, getWidth, getHeight, 32,
								 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (surface == NULL) {
		SDL_Log("SDL_CreateRGBSurface (surface) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING, getWidth, getHeight);
	if (texture == NULL) {
		SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	while (!quit_loop) {
		main_loop_step();
	}

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(video);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);

	ZLIB_Stop();

	free(fbm_buffer);

	fclose(file_handle);

	SDL_Quit();

	return 0;
}
