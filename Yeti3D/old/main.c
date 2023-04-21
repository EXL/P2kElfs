/*
** Name: Yeti3D
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
*/

#include "yeti.h"

#if defined(PLATFORM_SDL)
#include <SDL/SDL.h>

static SDL_Surface *video;
static SDL_Surface *surface;
static int quit_loop = 0;
static const int SCREEN_FPS = 35;

texture_t *textures = NULL;
int *sintable = NULL;
unsigned short (*lua)[LUA_HEIGHT] = NULL;
int *reciprocal = NULL;
#endif

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
#if defined(PLATFORM_GBA)
  world->screen = (viewport_t*) 0x06000000;
  world->buffer = (viewport_t*) 0x0600A000;
#elif defined(PLATFORM_SDL)
  surface = SDL_CreateRGBSurface(SDL_HWPALETTE, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, 0xF800, 0x07E0, 0x001F, 0x0000); // RGB565
//  surface = SDL_CreateRGBSurface(SDL_HWPALETTE, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, 0x7C00, 0x03E0, 0x001F, 0x0000); // RGB555
//  surface = SDL_CreateRGBSurface(SDL_HWPALETTE, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, 0x001F, 0x03E0, 0x7C00, 0x0000); // BGR555
  world->screen = NULL;
  world->buffer = (viewport_t*) surface->pixels;
#endif

  camera = entity_create(0, 0, 0);

  camera->x = MAP_SIZE << 15;
  camera->y = MAP_SIZE << 15;
  camera->z = 3 << 15;
  camera->p = 100 << 16;
}

#if defined(PLATFORM_GBA)
int main(void)
#else
int main(int argc, char *argv[])
#endif
{
  int x, y;

#if defined(PLATFORM_GBA)
  *(short*)0x4000000 = 0x405;
  *(short*)0x4000020 = 0;
  *(short*)0x4000022 = -128;
  *(short*)0x4000024 = 128;
  *(short*)0x4000026 = 0;
  *(short*)0x4000028 = i2f(120);
  *(short*)0x400002C = i2f(4);
#elif defined(PLATFORM_SDL)
  SDL_Event event;
  SDL_Init(SDL_INIT_VIDEO);
  video = SDL_SetVideoMode(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 16, SDL_HWSURFACE);

  textures = (texture_t*) malloc(TEXTURE_WIDTH * TEXTURE_HEIGHT * TEXTURE_MAX);
  FILE *res_file = fopen("Yeti3D.tex", "rb");
  size_t readen = fread(textures, TEXTURE_WIDTH * TEXTURE_HEIGHT * TEXTURE_MAX, 1, res_file);
  fclose(res_file);

  sintable = (int *) malloc(SINTABLE_SIZE * SINTABLE_MAX);
  res_file = fopen("Yeti3D.sin", "rb");
  readen = fread(sintable, SINTABLE_SIZE * SINTABLE_MAX, 1, res_file);
  fclose(res_file);

  reciprocal = (int *) malloc(RECIPROCAL_SIZE * RECIPROCAL_MAX);
  res_file = fopen("Yeti3D.rec", "rb");
  readen = fread(reciprocal, RECIPROCAL_SIZE * RECIPROCAL_MAX, 1, res_file);
  fclose(res_file);

  lua = (unsigned short (*)[LUA_HEIGHT]) malloc(sizeof(lua_t));
  res_file = fopen("Yeti3D.lua", "rb");
  readen = fread(lua, sizeof(lua_t), 1, res_file);
  fclose(res_file);
#endif
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
#if defined(PLATFORM_GBA)
  while (!KEY_SELECT)
  {     
    behaviour(camera);
    box->t += 50 << 16;
    box->r += 40 << 16;
    box->p += 30 << 16;

    draw_world();
  }
#elif defined(PLATFORM_SDL)
  while (!quit_loop) {
      if (KEY_SELECT) {
          quit_loop = 1;
      }
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT:
            quit_loop = 1;
            break;
        }
      }
      behaviour(camera);
      box->t += 50 << 16;
      box->r += 40 << 16;
      box->p += 30 << 16;
      draw_world();

      SDL_BlitSurface(surface, NULL, video, NULL);
      SDL_Flip(video);
      SDL_Delay(1000 / SCREEN_FPS);
    }

  free(textures);
  free(sintable);
  free(reciprocal);
  free(lua);

  SDL_FreeSurface(surface);
  SDL_FreeSurface(video);
  SDL_Quit();
#endif

  return 0;
}

