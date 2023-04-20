/*
** Name: Yeti3D
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
*/

#ifndef __YETI3D_H__
#define __YETI3D_H__

#if defined(__P2K__)
#define inline __inline
#include <mem.h>
#else
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#endif

#define RGB_SET(r,g,b) (((b)<<10)+((g)<<5)+(r))

#if 0 /* EXL: Dublicate function? */
inline int abs(const int a)
{
  return a < 0 ? -a : a;
}
#endif

inline int min(const int a, const int b)
{
  return a < b ? a : b;
}

inline int max(const int a, const int b)
{
  return a > b ? a : b;
}

/*
** Name: isqrt
** Desc: Integer square root. Take the square root of an integer.
*/
inline int isqrt(int value)
{
  int root = 0;

#define STEP(shift) \
    if((0x40000000 >> shift) + root <= value)          \
    {                                                   \
        value -= (0x40000000 >> shift) + root;          \
        root = (root >> 1) | (0x40000000 >> shift);     \
    }                                                   \
    else                                                \
    {                                                   \
        root >>= 1;                                     \
    }

  STEP( 0); STEP( 2); STEP( 4); STEP( 6);
  STEP( 8); STEP(10); STEP(12); STEP(14);
  STEP(16); STEP(18); STEP(20); STEP(22);
  STEP(24); STEP(26); STEP(28); STEP(30);

  // round to the nearest integer, cuts max error in half

  if (root < value) root++;

  return root;
}

#define FIXED_BITS 8
#define FIXED_ONE 0x100
#define FIXED_HALF 0x80

#define f2i(A) ((A) >> FIXED_BITS)
#define i2f(A) ((A) << FIXED_BITS)

#define fixmul(A,B) f2i((A)*(B))
#define fixdiv(A,B) (i2f(A)/(B))

#define fixceil(A) f2i((A)+0xFF)
#define fixinv(A) fixdiv(FIXED_ONE,(A))

#define SINTABLE_SIZE (4)
#define SINTABLE_MAX (2048)
#if defined(PLATFORM_GBA)
extern const int sintable[];
#else
extern int *sintable;
#endif

#define fixsin16(A) sintable[(A)&2047]
#define fixcos16(A) sintable[((A)+512)&2047]
#define fixsin(A) (fixsin16(A)>>8)
#define fixcos(A) (fixcos16(A)>>8)

#if defined(PLATFORM_GBA)
#define ROMMEM __attribute__ ((section (".ewram")))
#define CODE_IN_IWRAM __attribute__ ((section (".iwram"), long_call))
#define CODE_IN_ROM __attribute__ ((section (".text"), long_call))
#define IN_IWRAM __attribute__ ((section (".iwram")))
#define IN_EWRAM __attribute__ ((section (".ewram")))
#else
#define ROMMEM
#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM
#endif

typedef struct
{
  int x, y;
} vec2_t;

typedef struct
{
  int x, y;
  int z;
} vec3_t;

inline void vector_init(vec3_t* v, int x, int y, int z)
{
  v->x = x;
  v->y = y;
  v->z = z;
}

inline void vector_add(vec3_t* a, vec3_t* b, vec3_t* c)
{
  a->x = b->x + c->x;
  a->y = b->y + c->y;
  a->z = b->z + c->z;
}

inline void vector_sub(vec3_t* a, vec3_t* b, vec3_t* c)
{
  a->x = b->x - c->x;
  a->y = b->y - c->y;
  a->z = b->z - c->z;
}

typedef int matrix_t[3][3];

/*
** Name: matrix_copy
** Desc: Copies the contents of one matrix to another.
*/
inline void matrix_copy(matrix_t dst, matrix_t src)
{
  memcpy(dst, src, sizeof(matrix_t));
}

/*
** Name: matrix_identity
** Desc: Returns a identity matrix.
*/
inline void matrix_identity(matrix_t dst)
{
  matrix_t src =
  {
    {i2f(1), i2f(0), i2f(0)},
    {i2f(0), i2f(1), i2f(0)},
    {i2f(0), i2f(0), i2f(1)},
  };
  matrix_copy(dst, src);
}

inline void matrix_position(matrix_t dst, int x, int y, int z)
{
  matrix_identity(dst);
  dst[0][2] = x;
  dst[1][2] = y;
  dst[2][2] = z;
}

/*
** Name: matrix_scale
** Desc: Creates a scale matrix.
*/
inline void matrix_scale(matrix_t dst, int x, int y, int z)
{
  matrix_identity(dst);
  dst[0][0] = x;
  dst[1][1] = y;
  dst[2][2] = z;
}

inline void matrix_rotate_x(matrix_t dst, int angle)
{
  matrix_identity(dst);
  dst[1][1] =   (dst[2][2] = fixcos(angle));
  dst[1][2] = - (dst[2][1] = fixsin(angle));
}

inline void matrix_rotate_y(matrix_t dst, int angle)
{
  matrix_identity(dst);
  dst[0][0] =   (dst[2][2] = fixcos(angle));
  dst[0][2] = - (dst[2][0] = fixsin(angle));
}

inline void matrix_rotate_z(matrix_t dst, int angle)
{
   matrix_identity(dst);
   dst[0][0] =   (dst[1][1] = fixcos(angle));
   dst[0][1] = - (dst[1][0] = fixsin(angle));
}

inline void matrix_multiply(matrix_t dst, matrix_t ma1, matrix_t ma2)
{
  int i, j;
  matrix_t tmp;

  for (i = 3; i--;)
  {
    for (j = 3; j--;)
    {
      tmp[i][j] =
        fixmul(ma2[i][0], ma1[0][j]) +
        fixmul(ma2[i][1], ma1[1][j]) +
        fixmul(ma2[i][2], ma1[2][j]) ;
    }
  }
  matrix_copy(dst, tmp);
}

inline void matrix_transpose(matrix_t dst, matrix_t src)
{
  int y, x;
  for (y = 3; y--;) for (x = 3; x--;) dst[x][y] = src[y][x];
}

inline void matrix_rotate_world(matrix_t m, int alp, int bet, int gam)
{
 int cosalp = fixcos(alp);
 int sinalp = fixsin(alp);
 int cosbet = fixcos(bet);
 int sinbet = fixsin(bet);
 int cosgam = fixcos(gam);
 int singam = fixsin(gam);

 m[0][0] =  fixmul(singam,fixmul(sinbet,sinalp)) + fixmul(cosgam,cosalp);
 m[0][1] =  fixmul(cosbet,sinalp);
 m[0][2] =  fixmul(singam,cosalp) - fixmul(cosgam,fixmul(sinbet,sinalp));

 m[1][0] =  fixmul(singam,fixmul(sinbet,cosalp)) - fixmul(cosgam,sinalp);
 m[1][1] =  fixmul(cosbet,cosalp);
 m[1][2] = -fixmul(cosgam,fixmul(sinbet,cosalp)) - fixmul(singam,sinalp);

 m[2][0] = -fixmul(singam,cosbet);
 m[2][1] =  sinbet;
 m[2][2] =  fixmul(cosgam,cosbet);
}

inline void matrix_rotate_object(matrix_t m, int alp, int bet, int gam)
{
  int cosalp = fixcos(alp);
  int sinalp = fixsin(alp);
  int cosbet = fixcos(bet);
  int sinbet = fixsin(bet);
  int cosgam = fixcos(gam);
  int singam = fixsin(gam);

  m[0][0] =  fixmul(cosalp,cosgam) - fixmul(sinalp,fixmul(sinbet,singam));
  m[0][1] =  fixmul(sinalp,cosgam) + fixmul(cosalp,fixmul(sinbet,singam));
  m[0][2] =  fixmul(cosbet,singam);

  m[1][0] = -fixmul(sinalp,cosbet);
  m[1][1] =  fixmul(cosalp,cosbet);
  m[1][2] = -sinbet;

  m[2][0] = -fixmul(cosalp,singam) - fixmul(sinalp,fixmul(sinbet,cosgam));
  m[2][1] =  fixmul(cosalp,fixmul(sinbet,cosgam)) - fixmul(sinalp,singam);
  m[2][2] =  fixmul(cosbet,cosgam);
}

#define VIEWPORT_WIDTH 128
#define VIEWPORT_HEIGHT 160
#define VIEWPORT_X1 0
#define VIEWPORT_Y1 0
#define VIEWPORT_X2 (VIEWPORT_WIDTH - 0)
#define VIEWPORT_Y2 (VIEWPORT_HEIGHT - 0)

typedef struct
{
  unsigned short pixels[VIEWPORT_HEIGHT][VIEWPORT_WIDTH];
} viewport_t;

#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64
#define TEXTURE_MAX 8

typedef unsigned char texture_t[TEXTURE_HEIGHT][TEXTURE_WIDTH];

#define LUA_WIDTH 64
#define LUA_HEIGHT 256
#define LUA_SIZE 2

typedef unsigned short lua_t[LUA_WIDTH][LUA_HEIGHT];

// Type definition of a polygon vertex
typedef struct
{
  int x, y;
  int z;
  int l, u, v, d;
} vertex_t;

// The maxium numver of verties allowed in a polyogn.
#define POLYGON_MAX 8

typedef vertex_t polygon_t[POLYGON_MAX];


#define ENTITY_VISIBLE 1

typedef struct
{
  int x, xx;
  int y, yy;
  int z, zz;
  int p, pp; // pitch
  int r, rr; // roll
  int t, tt; // turn
  unsigned short time;
  unsigned char flags;
} entity_t;

//#define CELL_SOLID 1
//#define CELL_SPACE 0

#define CELL_ISSOLID(A) ((A)->top <= (A)->bot)

typedef struct
{
  unsigned char l, ent;
  unsigned char wtx, ttx, btx;
  signed char top, bot;
  unsigned short time;
} cell_t;

#define MAP_SIZE 64

typedef struct
{
  viewport_t* screen;
  viewport_t* buffer;
  cell_t cells[MAP_SIZE][MAP_SIZE];
  int time;
  matrix_t m;
} world_t;

extern IN_EWRAM world_t world;
extern IN_EWRAM entity_t entities[];
extern int nentities;
extern entity_t* camera;

#if defined(PLATFORM_GBA)
#define KEY_STATE (*(unsigned long*)0x04000130)

#define KEY_SELECT !(KEY_STATE & 4)
#define KEY_RIGHT !(KEY_STATE & 16)
#define KEY_LEFT !(KEY_STATE & 32)
#define KEY_UP !(KEY_STATE & 64)
#define KEY_DOWN !(KEY_STATE & 128)
#elif defined(PLATFORM_SDL)
#define KEY_STATE (SDL_GetKeyState(NULL))

#define KEY_SELECT (KEY_STATE[SDLK_ESCAPE])
#define KEY_RIGHT (KEY_STATE[SDLK_RIGHT])
#define KEY_LEFT (KEY_STATE[SDLK_LEFT])
#define KEY_UP (KEY_STATE[SDLK_UP])
#define KEY_DOWN (KEY_STATE[SDLK_DOWN])
#endif

void entity_to_world_collision(entity_t* const e, int const radius);
entity_t* entity_create(int x, int y, int z);
void draw_light(const int lightx, const int lighty, const int diffuse);

void CODE_IN_IWRAM draw_world(void);
void CODE_IN_IWRAM draw_entity(const entity_t* entity, const signed char* model);
void CODE_IN_IWRAM draw_texture(
  viewport_t* const dst,
  vertex_t* const src,
  int n,
  texture_t texture);

#if defined(PLATFORM_GBA)
#include "data.h"
#else
extern texture_t *textures;

#define RECIPROCAL_SIZE (4)
#define RECIPROCAL_MAX (301)
extern int *reciprocal;

extern unsigned short (*lua)[LUA_HEIGHT];
#endif

#endif
