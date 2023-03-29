#ifndef PERLIN_H
#define PERLIN_H

#include "fixed.h"

Fixed perlin(Fixed x, Fixed y, Fixed angle, int cache_idx);

void perlin16_fast(int width, int height, Fixed angle);

typedef unsigned char uchar;
void draw_pixel(uchar x, uchar y);

extern int randoms;
extern int rotations;
extern int gradients;
extern int interpolations;

#endif /* PERLIN_H */
