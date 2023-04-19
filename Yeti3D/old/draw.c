/*
** Name: Yeti3D
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
*/

#include "yeti.h"

#if defined(__P2K__)
#include <utilities.h>
#endif

#define NO_LIGHTING

#define RAY_COUNT 40
#define RAY_WIDTH 20
#define CELL_MAX 100

world_t world;
int nentities;
entity_t entities[100];
entity_t* camera;

const signed char cube[] =
{
  8,
  -32,   32,  32,
   32,   32,  32,
   32,   32, -32,
  -32,   32, -32,
  -32,  -32,  32,
   32,  -32,  32,
   32,  -32, -32,
  -32,  -32, -32,
  6,
  4, 0, 0, 63, 1, 63, 63, 2, 63, 0, 3, 0, 0, 4,
  4, 3, 0, 63, 2, 63, 63, 6, 63, 0, 7, 0, 0, 4,
  4, 2, 0, 63, 1, 63, 63, 5, 63, 0, 6, 0, 0, 5,
  4, 0, 0, 63, 4, 63, 63, 5, 63, 0, 1, 0, 0, 4,
  4, 0, 0, 63, 3, 63, 63, 7, 63, 0, 4, 0, 0, 5,
  4, 4, 0, 63, 7, 63, 63, 6, 63, 0, 5, 0, 0, 4
};

/*
** Name: polygon_clip
** Desc: clip a polygon to pre-calculated plane distances.
*/
inline int polygon_clip(vertex_t* dst, vertex_t* src, int n)
{
  vertex_t* pre;
  vertex_t* start = dst;

  if (n > 2)
  {
    for (pre = &src[n - 1]; n--; pre = src++)
    {
      if (pre->d >= 0) *dst++ = *pre;

      if ((pre->d ^ src->d) < 0)
      {
        int r = fixdiv(pre->d, pre->d - src->d);

        dst->x = pre->x + fixmul(src->x - pre->x, r);
        dst->y = pre->y + fixmul(src->y - pre->y, r);
        dst->z = pre->z + fixmul(src->z - pre->z, r);
        dst->u = pre->u + fixmul(src->u - pre->u, r);
        dst->v = pre->v + fixmul(src->v - pre->v, r);
        dst->l = pre->l + fixmul(src->l - pre->l, r);

        dst++;
      }
    }
  }
  return dst - start;
}

/*
** Name: draw_texture
** Desc: Renders a texture to a screen. The texture is DDA mapped to the given polygon.
*/
void draw_texture(
  viewport_t* const dst,
  vertex_t* const src,
  int n,
  const texture_t texture)
{
  polygon_t tm1, tm2;
  int i;

  for (i = n; i--; src[i].d = src[i].x  + src[i].z); n = polygon_clip(tm1, src, n); // Left
  for (i = n; i--; tm1[i].d = tm1[i].z  - tm1[i].x); n = polygon_clip(tm2, tm1, n); // Right
  for (i = n; i--; tm2[i].d = tm2[i].y  + tm2[i].z); n = polygon_clip(tm1, tm2, n); // Top
  for (i = n; i--; tm1[i].d = tm1[i].z  - tm1[i].y); n = polygon_clip(tm2, tm1, n); // Bottom

  if (n > 2)
  {
    int lt_i, lt_x, lt_xx, lt_u, lt_uu, lt_v, lt_vv, lt_l, lt_ll;
    int rt_i, rt_x, rt_xx, rt_u, rt_uu, rt_v, rt_vv, rt_l, rt_ll;
    int x1, x2, j, u, uu, v, vv, l, ll;
    int lt_length = 0, rt_length = 0;
    int y1 = 999999, y2 = -999999;

    for (i = n; i--;)
    {
      u = (i2f( 82) >> 1) * tm2[i].y / tm2[i].z + (i2f(VIEWPORT_WIDTH ) >> 1);
      v = (i2f(122) >> 1) * tm2[i].x / tm2[i].z + (i2f(VIEWPORT_HEIGHT) >> 1);
      tm2[i].x = u;
      tm2[i].y = v;
      tm2[i].d = fixceil(tm2[i].y);

      if (tm2[i].y < y1) {y1 = tm2[i].y; lt_i = rt_i = i;}
      if (tm2[i].y > y2) {y2 = tm2[i].y;}
    }
    y1 = fixceil(y1);
    y2 = fixceil(y2);

    for (; y1 < y2; y1++)
    {
      if (--lt_length <= 0)
      {
        do
        {
          i = lt_i;
          if (--lt_i < 0) lt_i = n - 1;
        }
        while ((lt_length = tm2[lt_i].d - tm2[i].d) <= 0);

        j = reciprocal[lt_length];

        lt_xx = ((tm2[lt_i].x - (lt_x = tm2[i].x)) * j) >> 16;
        lt_uu = ((tm2[lt_i].u - (lt_u = tm2[i].u)) * j) >> 16;
        lt_vv = ((tm2[lt_i].v - (lt_v = tm2[i].v)) * j) >> 16;
        lt_ll = ((tm2[lt_i].l - (lt_l = tm2[i].l)) * j) >> 16;
      }
      if (--rt_length <= 0)
      {
        do
        {
          i = rt_i;
          if (++rt_i >= n) rt_i = 0;
        }
        while ((rt_length = tm2[rt_i].d - tm2[i].d) <= 0);

        j = reciprocal[rt_length];

        rt_xx = ((tm2[rt_i].x - (rt_x = tm2[i].x)) * j) >> 16;
        rt_uu = ((tm2[rt_i].u - (rt_u = tm2[i].u)) * j) >> 16;
        rt_vv = ((tm2[rt_i].v - (rt_v = tm2[i].v)) * j) >> 16;
        rt_ll = ((tm2[rt_i].l - (rt_l = tm2[i].l)) * j) >> 16;
      }
	x1 = fixceil(lt_x);
      x2 = fixceil(rt_x);

      if ((i = x2 - x1) > 0)
      {
        unsigned short* fb;
        j  = reciprocal[i];
        uu = ((rt_u - (u = lt_u)) * j) >> 16;
        vv = ((rt_v - (v = lt_v)) * j) >> 16;
        ll = ((rt_l - (l = lt_l)) * j) >> 16;

        fb = &dst->pixels[y1][x1];

#define CASE(I) case I: AFFINE(0) fb++;

#define INNER_LOOP  \
for (; i >= 32; i -= 32, fb += 32) { \
AFFINE( 0) AFFINE( 1) AFFINE( 2) AFFINE( 3) AFFINE( 4) AFFINE( 5) AFFINE( 6) AFFINE( 7) \
AFFINE( 8) AFFINE( 9) AFFINE(10) AFFINE(11) AFFINE(12) AFFINE(13) AFFINE(14) AFFINE(15) \
AFFINE(16) AFFINE(17) AFFINE(18) AFFINE(19) AFFINE(20) AFFINE(21) AFFINE(22) AFFINE(23) \
AFFINE(24) AFFINE(25) AFFINE(26) AFFINE(27) AFFINE(28) AFFINE(29) AFFINE(30) AFFINE(31) \
} switch (i) { \
CASE(32) CASE(31) CASE(30) CASE(29) CASE(28) CASE(27) CASE(26) CASE(25) \
CASE(24) CASE(23) CASE(22) CASE(21) CASE(20) CASE(19) CASE(18) CASE(17) \
CASE(16) CASE(15) CASE(14) CASE(13) CASE(12) CASE(11) CASE(10) CASE( 9) \
CASE( 8) CASE( 7) CASE( 6) CASE( 5) CASE( 4) CASE( 3) CASE( 2) CASE( 1) \
}

#define AFFINE(I) fb[I] = lua[f2i(l)][texture[f2i(u)][f2i(v)]]; u+=uu; v+=vv; l+=ll;
        INNER_LOOP;
#undef  AFFINE        
      }
      lt_x += lt_xx; lt_u += lt_uu; lt_v += lt_vv; lt_l += lt_ll;
      rt_x += rt_xx; rt_u += rt_uu; rt_v += rt_vv; rt_l += rt_ll;
    }
  }
}

void CODE_IN_IWRAM draw_square(const texture_t texture, polygon_t p, const int light_offset)
{ 
  int i;
  polygon_t pp;
  
  for (i = 4; i--;)
  {
    int x = p[i].x - (camera->x >> 8);
    int y = p[i].y - (camera->z >> 8);
    int z = p[i].z - (camera->y >> 8);
    int l = world.cells[f2i(p[i].z)][f2i(p[i].x)].l + (abs((world.time & 31) - 16) - 16);

    pp[i].l = min(max(i2f(l), i2f(1)), i2f(62));
    pp[i].x = fixmul(world.m[0][0], x) + fixmul(world.m[0][1], y) + fixmul(world.m[0][2], z);
    pp[i].y = fixmul(world.m[1][0], x) + fixmul(world.m[1][1], y) + fixmul(world.m[1][2], z);
    pp[i].z = fixmul(world.m[2][0], x) + fixmul(world.m[2][1], y) + fixmul(world.m[2][2], z);
    pp[i].u = p[i].u;
    pp[i].v = p[i].v;
  }
  draw_texture(world.buffer, pp, 4, texture);
}

void CODE_IN_IWRAM draw_cell(int x1, int y1)
{
  polygon_t p;
  int i;
  int cx = camera->x >> 8;
  int cy = camera->y >> 8;
  
  cell_t* c0 = &world.cells[y1-1][x1];  
  cell_t* c1 = &world.cells[y1  ][x1];
  cell_t* c2 = &world.cells[y1+1][x1];

  int x2 = i2f(x1 + 1);
  int y2 = i2f(y1 + 1);
  x1 = i2f(x1);
  y1 = i2f(y1);
  
  p[0].u = i2f( 0); p[0].v = i2f(63);
  p[1].u = i2f(63); p[1].v = i2f(63);
  p[2].u = i2f(63); p[2].v = i2f( 0);
  p[3].u = i2f( 0); p[3].v = i2f( 0);

  // Right

  if (cx < x2)
  {
    p[0].x = x2; p[0].z = y1;
    p[1].x = x2; p[1].z = y1;
    p[2].x = x2; p[2].z = y2;
    p[3].x = x2; p[3].z = y2;

    for (i = c1[0].bot; i < c1[1].bot; i++)
    {
      p[0].y = i2f(i + 1); p[1].y = i2f(i); p[2].y = i2f(i); p[3].y = i2f(i + 1);

      draw_square(textures[c1[1].wtx], p, 0);
    }
    for (i = c1[1].top; i < c1[0].top; i++)
    {
      p[0].y = i2f(i + 1); p[1].y = i2f(i); p[2].y = i2f(i); p[3].y = i2f(i + 1);

      draw_square(textures[c1[1].wtx], p, 0);
    }
  }

  // Left
   
  if (cx > x1)
  {
    p[0].x = x1; p[0].z = y2;
    p[1].x = x1; p[1].z = y2;
    p[2].x = x1; p[2].z = y1;
    p[3].x = x1; p[3].z = y1;
      
    for (i = c1[0].bot; i < c1[-1].bot; i++)
    {
      p[0].y = i2f(i + 1); p[1].y = i2f(i); p[2].y = i2f(i); p[3].y = i2f(i + 1);
    
      draw_square(textures[c1[-1].wtx], p, 0);
    }
    for (i = c1[-1].top; i < c1[0].top; i++)
    {
      p[0].y = i2f(i + 1); p[1].y = i2f(i); p[2].y = i2f(i); p[3].y = i2f(i + 1);

      draw_square(textures[c1[-1].wtx], p, 0);
    }
  }

  // Front
  
  if (cy < y2)
  {
    p[0].x = x2; p[0].z = y2;
    p[1].x = x2; p[1].z = y2;
    p[2].x = x1; p[2].z = y2;
    p[3].x = x1; p[3].z = y2;
      
    for (i = c1[0].bot; i < c2[0].bot; i++)
    {
      p[0].y = i2f(i + 1); p[1].y = i2f(i); p[2].y = i2f(i); p[3].y = i2f(i + 1);

      draw_square(textures[c2[0].wtx], p, 0);
    }
    for (i = c2[0].top; i < c1[0].top; i++)
    {
      p[0].y = i2f(i + 1); p[1].y = i2f(i); p[2].y = i2f(i); p[3].y = i2f(i + 1);

      draw_square(textures[c2[0].wtx], p, 0);
    }
  }
 
  // Back
  if (cy > y1)
  {
    p[0].x = x1; p[0].z = y1;
    p[1].x = x1; p[1].z = y1;
    p[2].x = x2; p[2].z = y1;
    p[3].x = x2; p[3].z = y1;
      
    for (i = c1[0].bot; i < c0[0].bot; i++)
    {
      p[0].y = i2f(i + 1); p[1].y = i2f(i); p[2].y = i2f(i); p[3].y = i2f(i + 1);

      draw_square(textures[c0[0].wtx], p, 0);
    }
    for (i = c0[0].top; i < c1[0].top; i++)
    {
      p[0].y = i2f(i + 1); p[1].y = i2f(i); p[2].y = i2f(i); p[3].y = i2f(i + 1);

      draw_square(textures[c0[0].wtx], p, 0);
    }
  }

  if (!CELL_ISSOLID(c1))
  {
    // Cell Bottom (Floor)

    p[0].x = x1; p[0].y = i2f(c1->bot); p[0].z = y2;
    p[1].x = x2; p[1].y = i2f(c1->bot); p[1].z = y2;
    p[2].x = x2; p[2].y = i2f(c1->bot); p[2].z = y1;
    p[3].x = x1; p[3].y = i2f(c1->bot); p[3].z = y1;

    draw_square(textures[c1->btx], p, c1->bot);

    // Cell Top (Ceiling)

    p[0].x = x1; p[0].y = i2f(c1->top); p[0].z = y1;
    p[1].x = x2; p[1].y = i2f(c1->top); p[1].z = y1;
    p[2].x = x2; p[2].y = i2f(c1->top); p[2].z = y2;
    p[3].x = x1; p[3].y = i2f(c1->top); p[3].z = y2;

    draw_square(textures[c1->ttx], p, c1->top);
  }
  if (c1->ent < nentities) 
  {
    draw_entity(&entities[c1->ent], cube);
  }
}

void CODE_IN_IWRAM draw_world(void)
{
  int x, y, i;
  int ray_x[RAY_COUNT], ray_xx[RAY_COUNT];
  int ray_y[RAY_COUNT], ray_yy[RAY_COUNT];
  vec2_t visible_cells[CELL_MAX];
  int raycount, cellcount;

  world.time++;
    
  matrix_rotate_world(world.m, -(camera->r >> 16), -(camera->p >> 16), -(camera->t >> 16));
  
  x = camera->x;
  y = camera->y;
  
  for (i = 0; i < RAY_COUNT; i++)
  {
    int a = ((i - (RAY_COUNT >> 1)) * RAY_WIDTH) + (camera->t >> 16);

    ray_xx[i] = fixsin16(a) >> 0; ray_x[i] = x;
    ray_yy[i] = fixcos16(a) >> 0; ray_y[i] = y;
  }
  
  for (raycount = RAY_COUNT, cellcount = 0; raycount > 0 && cellcount < CELL_MAX;)
  {
    for (i = RAY_COUNT; i--;)
    {    
      if (ray_x[i])
      {
        cell_t* cell = &world.cells[ray_y[i] >> 16][ray_x[i] >> 16];
                  
        if (CELL_ISSOLID(cell))
        {
          raycount--;
          ray_x[i] = 0;                   
        }
        else
        {
          if (world.time != cell->time)
          {
            cell->time = world.time;
            cell->ent = 255;
            visible_cells[cellcount].x = ray_x[i] >> 16;
            visible_cells[cellcount].y = ray_y[i] >> 16;
            if (++cellcount == CELL_MAX) break;            
          }         
                  
          ray_x[i] += ray_xx[i];
          ray_y[i] += ray_yy[i];
        }
      }
    }
  }
  
  /* Merge entities into world map. */
  for (i = 0; i < nentities; i++)
  {
    int x = entities[i].x;
    int y = entities[i].y;
    if (x > camera->x) x -= 0x10000;
    if (y > camera->y) y -= 0x10000;
    world.cells[y >> 16][x >> 16].ent = i;
  }
  /* Render cells from back to front. */
  for (i = cellcount; i--;)
  {
    draw_cell(visible_cells[i].x, visible_cells[i].y);
  }
  
  // flip the screen display buffers
#if defined(PLATFORM_GBA)
  *(unsigned char*)0x04000000 ^= 0x10;
  viewport_t* temp = world.screen;
  world.screen = world.buffer;
  world.buffer = temp;
#endif
}

/*
** Name: draw_entity
** Desc: Draws a entity 3D model at the given location.
*/
void CODE_IN_IWRAM draw_entity(const entity_t* entity, const signed char* model)
{
  matrix_t m;
  polygon_t p;
  int nvertices, i, j;
  vec3_t vertices[100];

  matrix_rotate_object(m, entity->r >> 16, entity->p >> 16, entity->t >> 16);
  
  for (nvertices = *model++, i = 0; i < nvertices; i++)
  {
    int u = *model++ << 1;
    int v = *model++ << 1;
    int w = *model++ << 1;
    int x = fixmul(m[0][0], u) + fixmul(m[0][1], v) + fixmul(m[0][2], w) + ((entity->x - camera->x) >> 8);
    int y = fixmul(m[1][0], u) + fixmul(m[1][1], v) + fixmul(m[1][2], w) + ((entity->z - camera->z) >> 8);
    int z = fixmul(m[2][0], u) + fixmul(m[2][1], v) + fixmul(m[2][2], w) + ((entity->y - camera->y) >> 8);
    
    vertices[i].x = fixmul(world.m[0][0], x) + fixmul(world.m[0][1], y) + fixmul(world.m[0][2], z);
    vertices[i].y = fixmul(world.m[1][0], x) + fixmul(world.m[1][1], y) + fixmul(world.m[1][2], z);
    vertices[i].z = fixmul(world.m[2][0], x) + fixmul(world.m[2][1], y) + fixmul(world.m[2][2], z);
  }
  for (i = *model++; i--;)
  {
    int npts = *model++;
    for (j = 0; j < npts; j++)
    {
      *((vec3_t*)&p[j]) = vertices[*model++];
      p[j].u = i2f(*model++);
      p[j].v = i2f(*model++);
      p[j].l = i2f(32);
    }
    draw_texture(world.buffer, p, npts, textures[*model++]);
  }
}
