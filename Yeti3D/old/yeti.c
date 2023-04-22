/*
** Name: Yeti3D
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
*/

#include "yeti.h"

/*
** Name: entity_create
** Desc: Creates a new entity and returns a entity pointer.
*/
entity_t* entity_create(int x, int y, int z)
{
  entity_t* e = &entities[nentities++];
  e->x = x;
  e->y = y;
  e->z = z;
  return e;
}

/*
** Name: draw_light
** Desc: Renders a spot light at a given cell location.
*/
void draw_light(const int lightx, const int lighty, const int diffuse)
{
  int i;
  world.time++;

  for (i = 0; i < 2048; i += 8)
  {
    int x = lightx + 0x8000, xx = fixsin16(i) >> 1;
    int y = lighty + 0x8000, yy = fixcos16(i) >> 1;
    int l = i2f(63), d = 0;

    for (; l >= 0; x += xx, y += yy)
    {
      cell_t* cell = &world.cells[y >> 16][x >> 16];

      if (CELL_ISSOLID(cell)) break;

      if (world.time != cell->time)
      {
        int nl = cell->l + f2i(l);
        cell->l = min(nl, 63);
        cell->time = world.time;
      }
      l -= d += diffuse;
    }
  }
}

/*
** Name: entity_to_world_collision
** Desc: Simple map to entity collision.
*/
void entity_to_world_collision(entity_t* const e, int const radius)
{
  int x, y;
  cell_t* c;

#define IS_COLLISION ((e->z > (c->top << 16)) || (e->z < (c->bot << 16)))

  c = &world.cells[e->y >> 16][(x = e->x - radius) >> 16];
  
  if (IS_COLLISION)
  {
    e->x += 0x10000 - (x & 0xFFFF);
    e->xx = 0;
  }

  c = &world.cells[e->y >> 16][(x = e->x + radius) >> 16];
  
  if (IS_COLLISION)
  {
    e->x -= (x & 0xFFFF);
    e->xx = 0;
  }

  c = &world.cells[(y = e->y - radius) >> 16][e->x >> 16];
  
  if (IS_COLLISION)
  {
    e->y += 0x10000 - (y & 0xFFFF);
    e->yy = 0;
  }
  
  c = &world.cells[(y = e->y + radius) >> 16][e->x >> 16];
  
  if (IS_COLLISION)
  {
    e->y -= (y & 0xFFFF);
    e->yy = 0;
  }
}
