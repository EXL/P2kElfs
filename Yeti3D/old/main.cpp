/*
** Name: Yeti3D
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
*/

#include "yeti.h"

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
  world->screen = (viewport_t*) malloc(VIEWPORT_WIDTH * VIEWPORT_HEIGHT * 2);
  world->buffer = (viewport_t*) malloc(VIEWPORT_WIDTH * VIEWPORT_HEIGHT * 2);
#endif

  camera->x = MAP_SIZE << 15;
  camera->y = MAP_SIZE << 15;
  camera->z = 3 << 15;
  camera->p = 100 << 16;
}

int main(void)
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
  
  while (!KEY_SELECT)
  {     
    behaviour(camera);
    box->t += 50 << 16;
    box->r += 40 << 16;
    box->p += 30 << 16;

    draw_world();
  }

  return 0;
}

