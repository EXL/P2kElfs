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

#include <SDL/SDL.h>

static SDL_Surface *video;
static SDL_Surface *surface;
static SDL_Surface *surface_back;
static int quit_loop = 0;
static const int SCREEN_FPS = 35;

IN_EWRAM yeti_t yeti;

void sdl_init(void)
{
  surface = SDL_CreateRGBSurface(SDL_HWPALETTE, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, 0xF800, 0x07E0, 0x001F, 0x0000); // RGB565
  surface_back = SDL_CreateRGBSurface(SDL_HWPALETTE, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, 0xF800, 0x07E0, 0x001F, 0x0000); // RGB565

//  surface = SDL_CreateRGBSurface(SDL_HWPALETTE, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, 0x7C00, 0x03E0, 0x001F, 0x0000); // RGB555
//  surface = SDL_CreateRGBSurface(SDL_HWPALETTE, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, 0x001F, 0x03E0, 0x7C00, 0x0000); // BGR555
}

void sdl_flip(void) {
//	framebuffer_t* temp;
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

//	temp = yeti.viewport.back;
//	yeti.viewport.back = yeti.viewport.front;
//	yeti.viewport.front = temp;
}

int main(int argc, char *argv[])
{
	SDL_Event event;
	SDL_Init(SDL_INIT_VIDEO);
	video = SDL_SetVideoMode(YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, SDL_HWSURFACE);

	sdl_init();

	yeti_init(&yeti, (framebuffer_t*) surface->pixels, (framebuffer_t*) surface_back->pixels, textures, palette, lua);
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

		game_tick(&yeti);
		game_draw(&yeti);

		sdl_flip();

		SDL_BlitSurface(surface_back, NULL, video, NULL);
		SDL_Flip(video);
		SDL_Delay(1000 / SCREEN_FPS);
	  }

	return 0;
}
