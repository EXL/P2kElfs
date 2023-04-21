#ifndef __DATA_H__
#define __DATA_H__
#include "yeti.h"
#if defined(PLATFORM_GBA)
extern const texture_t textures[];
extern const lua_t lua;
#else
extern texture_t *textures;
extern unsigned short (*lua)[LUA_HEIGHT];
#endif
extern const int reciprocal[];
#endif
