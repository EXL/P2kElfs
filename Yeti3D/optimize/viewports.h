/*
Copyright (C) 2003 - Derek John Evans 

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
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans <derek@theteahouse.com.au>
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
**
** YY  YY EEEEEE TTTTTT IIIIII 33333  DDDDD
** YY  YY EE       TT     II       33 DD  DD
**  YYYY  EEEE     TT     II     333  DD  DD
**   YY   EE       TT     II       33 DD  DD
**   YY   EEEEEE   TT   IIIIII 33333  DDDDD
*/

#ifndef __VIEWPORTS_H__
#define __VIEWPORTS_H__

/******************************************************************************/

/*
** Some Porting Notes:
**
** - The standard pixel format is 5:5:5. There is a function in yeti.c which
**   will copy the viewport to a video display that supports a different pixel
**   format.
** - Define YETI_ROM as const for all ROM based ports.
** - You can patch functions by using the __PATCH?????__ macros.
** - Make sure you define the platform macro in your IDE/make scripts.
** - Viewports larger than 320x240 are not recommended unless you patch the
**   default texture mapper. The standard DDA code is limited to small displays.
**   Look at the OpenGL port for how todo this.
**
*/

/******************************************************************************/

#ifdef __P2K__

#define YETI_RGB555
#define YETI_LOW_QUALITY_WALLS

#define YETI_ROM const

#define YETI_VIEWPORT_INTERVAL 30
#define YETI_VIEWPORT_INTERVAL_ERROR 7

#if defined(ROT_90)
    #if !defined(G_DEFAULT)
        #define YETI_DISPLAY_WIDTH 96
        #define YETI_DISPLAY_HEIGHT 64
    #else
        #define YETI_DISPLAY_WIDTH 120
        #define YETI_DISPLAY_HEIGHT 80
    #endif
#elif defined(ROT_0)
    #if !defined(G_DEFAULT)
        #define YETI_DISPLAY_WIDTH 220
        #define YETI_DISPLAY_HEIGHT 176
    #else
        #define YETI_DISPLAY_WIDTH 160
        #define YETI_DISPLAY_HEIGHT 128
    #endif
#elif defined(VIEW_96X64)
    #define YETI_DISPLAY_WIDTH 96
    #define YETI_DISPLAY_HEIGHT 64
#elif defined(VIEW_128X128)
    #define YETI_DISPLAY_WIDTH 128
    #define YETI_DISPLAY_HEIGHT 128
#endif

#if (YETI_DISPLAY_WIDTH == 220) && (YETI_DISPLAY_HEIGHT == 176)
#define NO_STRETCH
#endif

#if (YETI_DISPLAY_WIDTH == 160) && (YETI_DISPLAY_HEIGHT == 128)
#define NO_STRETCH
#endif

#if (YETI_DISPLAY_WIDTH == 128) && (YETI_DISPLAY_HEIGHT == 128)
#define NO_STRETCH
#endif

#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 YETI_DISPLAY_WIDTH
#define YETI_VIEWPORT_Y2 YETI_DISPLAY_HEIGHT
#define YETI_VIEWPORT_WIDTH YETI_DISPLAY_WIDTH
#define YETI_VIEWPORT_HEIGHT YETI_DISPLAY_HEIGHT

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

#ifdef __SDL__

#define YETI_RGB555
#define YETI_LOW_QUALITY_WALLS

#define YETI_ROM const

#define YETI_VIEWPORT_INTERVAL 35
#define YETI_VIEWPORT_INTERVAL_ERROR 7

#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

#ifdef __GBA__

#define YETI_RGB555
#define YETI_LOW_QUALITY_WALLS

#define YETI_ROM const

#define YETI_VIEWPORT_INTERVAL 14
#define YETI_VIEWPORT_INTERVAL_ERROR 7

#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 160
#define YETI_VIEWPORT_Y2 128
#define YETI_VIEWPORT_WIDTH 160
#define YETI_VIEWPORT_HEIGHT 128

//#define __PATCH_DRAW_POLY__

#define CODE_IN_IWRAM __attribute__ ((section (".iwram"), long_call))
#define CODE_IN_ROM __attribute__ ((section (".text"), long_call))
#define IN_IWRAM __attribute__ ((section (".iwram")))
#define IN_EWRAM __attribute__ ((section (".ewram")))

#endif

/******************************************************************************/

#ifdef __SERIES60__

#define YETI_RGB444
#define YETI_LOW_QUALITY_WALLS

#define YETI_ROM const

#define YETI_VIEWPORT_INTERVAL 35
#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

/******************************************************************************/

#ifdef __GP32__

#define YETI_RGB565
#define YETI_LOW_QUALITY_WALLS

#define YETI_ROM

#define YETI_VIEWPORT_INTERVAL 35
#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

/******************************************************************************/

#ifdef __POCKETPC__

#define YETI_RGB565
#define YETI_LOW_QUALITY_WALLS
#define YETI_ROM

#define YETI_VIEWPORT_INTERVAL 35
#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

/******************************************************************************/

#ifdef __SVGALIB__

#define YETI_RGB555
#define YETI_ROM

#define YETI_VIEWPORT_INTERVAL 35
#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

/******************************************************************************/

#ifdef __GAMECUBE__

#define YETI_RGB565
#define YETI_ROM

#define YETI_VIEWPORT_INTERVAL 35
#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

/******************************************************************************/

#if defined(__WIN32__) && !defined(__SDL__)

#define YETI_RGB555
#define YETI_ROM 

#define YETI_VIEWPORT_INTERVAL 35
#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240
#define YETI_VIDEO_LUT

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

/******************************************************************************/

#ifdef __MSDOS__

#define YETI_RGB555
#define YETI_ROM

#define YETI_VIEWPORT_INTERVAL 35
#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

/******************************************************************************/

#ifdef __PSONE__

#define YETI_RGB555
#define YETI_ROM

#define YETI_VIEWPORT_INTERVAL 3
#define YETI_VIEWPORT_INTERVAL_ERROR 1

#define YETI_VIEWPORT_X1 0
#define YETI_VIEWPORT_Y1 0
#define YETI_VIEWPORT_X2 320
#define YETI_VIEWPORT_Y2 240
#define YETI_VIEWPORT_WIDTH 320
#define YETI_VIEWPORT_HEIGHT 240

#define __PATCH_DRAW_CLIPPED_POLY__

#define CODE_IN_IWRAM
#define CODE_IN_ROM
#define IN_IWRAM
#define IN_EWRAM

#endif

/******************************************************************************/

#endif

 
