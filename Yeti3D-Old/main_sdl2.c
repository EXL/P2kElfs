/*
** Name: Yeti3D
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
*/

#include "yeti.h"

#include <time.h>

#include <SDL2/SDL.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>

typedef struct {
	entity_t* box;
} CONTEXT_EMSCRIPTEN_T;
#endif

#define SDL_VIEWPORT_WIDTH  (240)
#define SDL_VIEWPORT_HEIGHT (320)
#define SCREEN_FPS          (35)

static SDL_Surface *video;
static SDL_Surface *surface;
static SDL_Renderer *render;
static SDL_Texture *texture;

static int quit_loop = 0;

void behaviour(entity_t* const e)
{    
	if (KEY_LEFT)
	{
		e->tt -= (12 << 16);
		e->r -= 800000;
	}
	if (KEY_RIGHT)
	{
		e->tt += (12 << 16);
		e->r += 800000;
	}

	if (KEY_UP)
	{
		e->xx += fixsin16(e->t >> 16) >> 5;
		e->yy += fixcos16(e->t >> 16) >> 5;
	}
	if (KEY_DOWN)
	{
		e->xx -= fixsin16(e->t >> 16) >> 5;
		e->yy -= fixcos16(e->t >> 16) >> 5;
	}
	e->xx -= ((e->xx + (e->xx < 0 ? -15 : 15)) >> 4);
	e->yy -= ((e->yy + (e->yy < 0 ? -15 : 15)) >> 4);
	e->tt -= ((e->tt + (e->tt < 0 ? - 3 :  3)) >> 2);

	e->r  -= ((e->r  + (e->r  < 0 ? -7 : 7)) >> 3);

	e->x += e->xx; e->y += e->yy; e->z += e->zz;
	e->r += e->rr; e->t += e->tt; e->p += e->pp;

	entity_to_world_collision(e, 0x8000);
}

void world_create(world_t* world)
{
	world->screen = NULL;
	world->buffer = (viewport_t*) surface->pixels;

	camera = entity_create(0, 0, 0);

	camera->x = MAP_SIZE << 15;
	camera->y = MAP_SIZE << 15;
	camera->z = 3 << 15;
	camera->p = 100 << 16;
}

static void main_loop_step(entity_t* box) {
	SDL_Rect dst;
	SDL_Event event;

	if (KEY_SELECT) {
		quit_loop = 1;
	}
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
#if !defined(__EMSCRIPTEN__)
				quit_loop = 1;
#endif
				break;
			}
	}

	behaviour(camera);
	box->t += 50 << 16;
	box->r += 40 << 16;
	box->p += 30 << 16;
	draw_world();

	SDL_BlitSurface(surface, NULL, video, NULL);
	SDL_UpdateTexture(texture, NULL, video->pixels, video->pitch);

	dst.x = (SDL_VIEWPORT_HEIGHT - SDL_VIEWPORT_WIDTH) / 2;
	dst.y = (SDL_VIEWPORT_WIDTH - SDL_VIEWPORT_HEIGHT) / 2;
	dst.w = SDL_VIEWPORT_WIDTH;
	dst.h = SDL_VIEWPORT_HEIGHT;
	SDL_RenderCopyEx(render, texture, NULL, &dst, 270, NULL, SDL_FLIP_NONE);

	SDL_RenderPresent(render);
}

#ifdef __EMSCRIPTEN__
static void main_loop(void *arguments) {
	CONTEXT_EMSCRIPTEN_T *context = arguments;
	main_loop_step(context->box);
}
#endif

int main(int argc, char *argv[])
{
	int x, y;

	srand(time(NULL));

	SDL_Window *window = SDL_CreateWindow(
		"Yeti3D Old",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SDL_VIEWPORT_HEIGHT, SDL_VIEWPORT_WIDTH,
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

	video = SDL_CreateRGBSurface(0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (video == NULL) {
		SDL_Log("SDL_CreateRGBSurface (video) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

//	surface = SDL_CreateRGBSurface(0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, 0x7C00, 0x03E0, 0x001F, 0x0000); // RGB555
//	surface = SDL_CreateRGBSurface(0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, 0xF800, 0x07E0, 0x001F, 0x0000); // RGB565
	surface = SDL_CreateRGBSurface(0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, 0x001F, 0x03E0, 0x7C00, 0x0000); // BGR555
	if (surface == NULL) {
		SDL_Log("SDL_CreateRGBSurface (surface) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	if (texture == NULL) {
		SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	world_create(&world);

	entity_t* box = entity_create(33 << 16, 35 << 16, 1 << 16);
	entity_create(33 << 16, 33 << 16, 1 << 16);

	for (y = 0; y < MAP_SIZE; y++)
	{
		for (x = 0; x < MAP_SIZE; x++)
		{
			world.cells[y][x].l   = 0;
			world.cells[y][x].bot = 0;
			world.cells[y][x].top = 4;
			world.cells[y][x].wtx = 0;
			world.cells[y][x].btx = 2;
			world.cells[y][x].ttx = 1;

			if (x == 0 || y == 0 || x == (MAP_SIZE - 1) || y == (MAP_SIZE - 1))
			{
				world.cells[y][x].top = 0;
				world.cells[y][x].bot = 0;
			}
		}
	}

	for (x = 1; x < MAP_SIZE - 1; x++)
	{
		for (y = 1; y < MAP_SIZE - 1; y += 10)
		{
			if (rand() % 10)
			{
				world.cells[y][x].top = 0;
				world.cells[y][x].bot = 0;
				world.cells[x][y].top = 0;
				world.cells[x][y].bot = 0;

				world.cells[y][x].wtx = 0;
				world.cells[x][y].wtx = 0;
			}
			else
			{
				world.cells[y][x].top--;
			}
		}
	}
	for (x = 0; x < 400; x++)
	{
		cell_t* cell = &world.cells[rand() % MAP_SIZE][rand() % MAP_SIZE];

		cell->bot += 2;
		cell->wtx = 3;
		cell->btx = 3;
	}

	for (y = 0; y < MAP_SIZE; y += 1)
	{
		for (x = 0; x < MAP_SIZE; x += 1)
		{
			world.cells[y][x].l = 0;
		}
	}

	for (y = 1; y < MAP_SIZE; y += 5)
	{
		for (x = 1; x < MAP_SIZE; x += 5)
		{
			draw_light((x << 16) + 0x8000, (y << 16) + 0x8000, 900);
			world.cells[y][x].ttx = 5;
			world.cells[y][x].btx = 6;
		}
	}

#if !defined(__EMSCRIPTEN__)
	while (!quit_loop) {
		main_loop_step(box);
		SDL_Delay(1000 / SCREEN_FPS);
	}
#else
	CONTEXT_EMSCRIPTEN_T context;
	context.box = box;
	emscripten_set_main_loop_arg(main_loop, &context, SCREEN_FPS, 1); // 35 FPS.
#endif

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(video);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
