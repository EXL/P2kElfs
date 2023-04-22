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

#include <SDL/SDL.h>

static SDL_Surface *video;
static SDL_Surface *surface;
static int quit_loop = 0;

static IN_EWRAM yeti_t yeti;

static texture_t *res_tex = NULL;
static rgb555_t (*res_lua)[256] = NULL;
int *reciprocal = NULL;
int *sintable = NULL;

void read_resourse_files(void) {
	FILE *res_file;
	int readen;

	readen = 0;

	res_tex = (texture_t *) malloc(sizeof(texture_t) * YETI_TEXTURE_MAX);
	res_file = fopen("Yeti3D.tex", "rb");
	readen = fread(res_tex, sizeof(texture_t) * YETI_TEXTURE_MAX, 1, res_file);
	fclose(res_file);
	if (readen == 0) {
		fprintf(stderr, "Error: cannot read 'Yeti3D.tex' resource file.\n");
	}

	res_lua = (rgb555_t (*)[256]) malloc(sizeof(lua_t));
	res_file = fopen("Yeti3D.lua", "rb");
	readen = fread(res_lua, sizeof(lua_t), 1, res_file);
	fclose(res_file);
	if (readen == 0) {
		fprintf(stderr, "Error: cannot read 'Yeti3D.lua' resource file.\n");
	}

	reciprocal = (int *) malloc(sizeof(int) * YETI_RECIPROCAL_MAX);
	res_file = fopen("Yeti3D.rec", "rb");
	readen = fread(reciprocal, sizeof(int) * YETI_RECIPROCAL_MAX, 1, res_file);
	fclose(res_file);
	if (readen == 0) {
		fprintf(stderr, "Error: cannot read 'Yeti3D.rec' resource file.\n");
	}

	sintable = (int *) malloc(sizeof(int) * YETI_SINTABLE_MAX);
	res_file = fopen("Yeti3D.sin", "rb");
	readen = fread(sintable, sizeof(int) * YETI_SINTABLE_MAX, 1, res_file);
	fclose(res_file);
	if (readen == 0) {
		fprintf(stderr, "Error: cannot read 'Yeti3D.sin' resource file.\n");
	}
}

void sdl_init(void) {
#define w YETI_VIEWPORT_WIDTH
#define h YETI_VIEWPORT_HEIGHT
	SDL_Init(SDL_INIT_VIDEO);
	video = SDL_SetVideoMode(w, h, 16, SDL_HWSURFACE);
	surface = SDL_CreateRGBSurface(SDL_HWPALETTE, w, h, 16, 0xF800, 0x07E0, 0x001F, 0x0000); // RGB565
//	surface = SDL_CreateRGBSurface(SDL_HWPALETTE, w, h, 16, 0x7C00, 0x03E0, 0x001F, 0x0000); // RGB555
//	surface = SDL_CreateRGBSurface(SDL_HWPALETTE, w, h, 16, 0x001F, 0x03E0, 0x7C00, 0x0000); // BGR555
#undef h
#undef w
}

void check_keys(void) {
	Uint8 * keyboard = SDL_GetKeyState(NULL);
	yeti.keyboard.a      = keyboard[SDLK_z];
	yeti.keyboard.b      = keyboard[SDLK_x];
	yeti.keyboard.select = keyboard[SDLK_ESCAPE];
	yeti.keyboard.right  = keyboard[SDLK_RIGHT];
	yeti.keyboard.left   = keyboard[SDLK_LEFT];
	yeti.keyboard.up     = keyboard[SDLK_UP];
	yeti.keyboard.down   = keyboard[SDLK_DOWN];
	yeti.keyboard.l      = keyboard[SDLK_a];
	yeti.keyboard.r      = keyboard[SDLK_s];
}

int main(int argc, char *argv[]) {
	SDL_Event event;

	read_resourse_files();

	sdl_init();

	yeti_init(
		&yeti,
		NULL,
		(framebuffer_t *) surface->pixels,
		res_tex,
		NULL,
		res_lua
	);
	game_init(&yeti);

	while (!quit_loop) {
		if (yeti.keyboard.select) {
			quit_loop = 1;
		}
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit_loop = 1;
				break;
			}
		}

		yeti_tick(&yeti);
		yeti_draw(&yeti);

		check_keys();

		SDL_BlitSurface(surface, NULL, video, NULL);
		SDL_Flip(video);
		SDL_Delay(1000 / YETI_VIEWPORT_INTERVAL);
	}

	free(res_tex);
	free(res_lua);
	free(reciprocal);
	free(sintable);

	SDL_FreeSurface(surface);
	SDL_FreeSurface(video);
	SDL_Quit();

	return 0;
}
