/*
Copyright (C) 2003 - Derek John Evans
Copyright (C) 2023 - EXL

This file is part of Yeti3D Portable Engine

Yeti3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 2003 - Derek J. Evans <derek@theteahouse.com.au>
Prepared for public release: 10/24/2003 - Derek J. Evans <derek@theteahouse.com.au>
*/

/*
** Name: Yeti3D
** Desc: GBA port 
** Auth: Derek Evans
**
** Copyright (C) 2003, Derek Evans
**
** YY  YY EEEEEE TTTTTT IIIIII 33333  DDDDD
** YY  YY EE       TT     II       33 DD  DD
**  YYYY  EEEE     TT     II     333  DD  DD
**   YY   EE       TT     II       33 DD  DD
**   YY   EEEEEE   TT   IIIIII 33333  DDDDD
*/

#include "yeti.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#else
static int quit_loop = 0;
#endif

static SDL_Surface *video;
static SDL_Surface *surface;
static SDL_Renderer *render;
static SDL_Texture *texture;

static yeti_t yeti;

void check_keys(void) {
	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
	yeti.keyboard.a      = keyboard[SDL_SCANCODE_Z] || keyboard[SDL_SCANCODE_LCTRL] || keyboard[SDL_SCANCODE_RETURN];
	yeti.keyboard.b      = keyboard[SDL_SCANCODE_X] || keyboard[SDL_SCANCODE_SPACE];
	yeti.keyboard.select = keyboard[SDL_SCANCODE_ESCAPE];
	yeti.keyboard.right  = keyboard[SDL_SCANCODE_RIGHT] || keyboard[SDL_SCANCODE_D];
	yeti.keyboard.left   = keyboard[SDL_SCANCODE_LEFT] || keyboard[SDL_SCANCODE_A];
	yeti.keyboard.up     = keyboard[SDL_SCANCODE_UP] || keyboard[SDL_SCANCODE_W];
	yeti.keyboard.down   = keyboard[SDL_SCANCODE_DOWN] || keyboard[SDL_SCANCODE_S];
	yeti.keyboard.l      = keyboard[SDL_SCANCODE_C] || keyboard[SDL_SCANCODE_PAGEUP];
	yeti.keyboard.r      = keyboard[SDL_SCANCODE_V] || keyboard[SDL_SCANCODE_PAGEDOWN];
}

void main_loop_step(void) {
	SDL_Event event;
#if !defined(__EMSCRIPTEN__)
	if (yeti.keyboard.select) {
		quit_loop = 1;
	}
#endif
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
#if !defined(__EMSCRIPTEN__)
			quit_loop = 1;
#endif
			break;
		}
	}

	yeti_tick(&yeti);
	yeti_draw(&yeti);

	check_keys();

	SDL_BlitSurface(surface, NULL, video, NULL);
	SDL_UpdateTexture(texture, NULL, video->pixels, video->pitch);
	SDL_RenderCopy(render, texture, NULL, NULL);
	SDL_RenderPresent(render);
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	SDL_Window *window = SDL_CreateWindow(
		"Yeti3D",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT,
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

	video = SDL_CreateRGBSurface(0, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (video == NULL) {
		SDL_Log("SDL_CreateRGBSurface (video) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

//	RGB555
//	surface = SDL_CreateRGBSurface(0, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, 0x7C00, 0x03E0, 0x001F, 0x0000);
//	RGB565
//	surface = SDL_CreateRGBSurface(0, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, 0xF800, 0x07E0, 0x001F, 0x0000);
//	BGR555
	surface = SDL_CreateRGBSurface(0, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, 0x001F, 0x03E0, 0x7C00, 0x0000);
	if (surface == NULL) {
		SDL_Log("SDL_CreateRGBSurface (surface) failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT);
	if (texture == NULL) {
		SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	yeti_init(
		&yeti,
		NULL,
		(framebuffer_t *) surface->pixels,
		textures,
		NULL,
		lua
	);
	game_init(&yeti);

#if !defined(__EMSCRIPTEN__)
	while (!quit_loop) {
		main_loop_step();
		SDL_Delay(1000 / YETI_VIEWPORT_INTERVAL);
	}
#else
	emscripten_set_main_loop(main_loop_step, YETI_VIEWPORT_INTERVAL, 1); // 35 FPS.
#endif

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(video);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
