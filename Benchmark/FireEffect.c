/*
 * Extracted from FireEffect/FireEffect ELF application.
 */

#include <loader.h>
#include <ati.h>
#include <dal.h>
#include <mem.h>
#include <utilities.h>

#include "Phases.h"

static const UINT32 fire_palette[] = {
	ATI_565RGB(0x07, 0x07, 0x07), /*  0 */
	ATI_565RGB(0x1F, 0x07, 0x07), /*  1 */
	ATI_565RGB(0x2F, 0x0F, 0x07), /*  2 */
	ATI_565RGB(0x47, 0x0F, 0x07), /*  3 */
	ATI_565RGB(0x57, 0x17, 0x07), /*  4 */
	ATI_565RGB(0x67, 0x1F, 0x07), /*  5 */
	ATI_565RGB(0x77, 0x1F, 0x07), /*  6 */
	ATI_565RGB(0x8F, 0x27, 0x07), /*  7 */
	ATI_565RGB(0x9F, 0x2F, 0x07), /*  8 */
	ATI_565RGB(0xAF, 0x3F, 0x07), /*  9 */
	ATI_565RGB(0xBF, 0x47, 0x07), /* 10 */
	ATI_565RGB(0xC7, 0x47, 0x07), /* 11 */
	ATI_565RGB(0xDF, 0x4F, 0x07), /* 12 */
	ATI_565RGB(0xDF, 0x57, 0x07), /* 13 */
	ATI_565RGB(0xDF, 0x57, 0x07), /* 14 */
	ATI_565RGB(0xD7, 0x5F, 0x07), /* 15 */
	ATI_565RGB(0xD7, 0x5F, 0x07), /* 16 */
	ATI_565RGB(0xD7, 0x67, 0x0F), /* 17 */
	ATI_565RGB(0xCF, 0x6F, 0x0F), /* 18 */
	ATI_565RGB(0xCF, 0x77, 0x0F), /* 19 */
	ATI_565RGB(0xCF, 0x7F, 0x0F), /* 20 */
	ATI_565RGB(0xCF, 0x87, 0x17), /* 21 */
	ATI_565RGB(0xC7, 0x87, 0x17), /* 22 */
	ATI_565RGB(0xC7, 0x8F, 0x17), /* 23 */
	ATI_565RGB(0xC7, 0x97, 0x1F), /* 24 */
	ATI_565RGB(0xBF, 0x9F, 0x1F), /* 25 */
	ATI_565RGB(0xBF, 0x9F, 0x1F), /* 26 */
	ATI_565RGB(0xBF, 0xA7, 0x27), /* 27 */
	ATI_565RGB(0xBF, 0xA7, 0x27), /* 28 */
	ATI_565RGB(0xBF, 0xAF, 0x2F), /* 29 */
	ATI_565RGB(0xB7, 0xAF, 0x2F), /* 30 */
	ATI_565RGB(0xB7, 0xB7, 0x2F), /* 31 */
	ATI_565RGB(0xB7, 0xB7, 0x37), /* 32 */
	ATI_565RGB(0xCF, 0xCF, 0x6F), /* 33 */
	ATI_565RGB(0xDF, 0xDF, 0x9F), /* 34 */
	ATI_565RGB(0xEF, 0xEF, 0xC7), /* 35 */
	ATI_565RGB(0xFF, 0xFF, 0xFF)  /* 36 */
};

void FPS_Meter(void) {
#if defined(FPS_METER)
	UINT64 current_time;
	UINT32 delta;

	static UINT32 one = 0;
	static UINT64 last_time = 0;
	static UINT32 tick = 0;
	static UINT32 fps = 0;

	current_time = suPalTicksToMsec(suPalReadTime());
	delta = (UINT32) (current_time - last_time);
	last_time = current_time;

	tick = (tick + delta) / 2;
	if (tick != 0) {
		fps = 1000 * 10 / tick;
	}

	if (one > 30) {
		UtilLogStringData("FPS: %d.%d\n", fps / 10, fps % 10);
		PFprintf("FPS: %d.%d\n", fps / 10, fps % 10);
#if defined(EP2)
		cprintf("FPS: %d.%d\n", fps / 10, fps % 10);
#endif
		one = 0;
	}
	one++;
#endif
}

static UINT32 ATI_Display_Mode_Log(AHIDISPMODE_T *display_mode) {
	LOG("%s\n", "ATI Display Mode.");
	LOG("ATI Display Size: %dx%d\n", display_mode->size.x, display_mode->size.y);
	LOG("ATI Display Pixel Format: %d\n", display_mode->pixel_format);
	LOG("ATI Display Frequency: %d\n", display_mode->frequency);
	LOG("ATI Display Rotation: %d\n", display_mode->rotation);
	LOG("ATI Display Mirror: %d\n", display_mode->mirror);

	return RESULT_OK;
}

static UINT32 ATI_Driver_Log(APP_AHI_T *ahi) {
	LOG("%s\n", "ATI Driver Dump.");
	LOG("ATI Driver Name: %s\n", ahi->info_driver->drvName);
	LOG("ATI Driver Version: %s\n", ahi->info_driver->drvVer);
	LOG("ATI S/W Revision: %d (0x%08X)\n",
		ahi->info_driver->swRevision, ahi->info_driver->swRevision);
	LOG("ATI Chip ID: %d (0x%08X)\n",
		ahi->info_driver->chipId, ahi->info_driver->chipId);
	LOG("ATI Revision ID: %d (0x%08X)\n",
		ahi->info_driver->revisionId, ahi->info_driver->revisionId);
	LOG("ATI CPU Bus Interface Mode: %d (0x%08X)\n",
		ahi->info_driver->cpuBusInterfaceMode, ahi->info_driver->cpuBusInterfaceMode);
	LOG("ATI Total Memory: %d (%d KiB)\n",
		ahi->info_driver->totalMemory, ahi->info_driver->totalMemory / 1024);
	LOG("ATI Internal Memory: %d (%d KiB)\n",
		ahi->info_driver->internalMemSize, ahi->info_driver->internalMemSize / 1024);
	LOG("ATI External Memory: %d (%d KiB)\n",
		ahi->info_driver->externalMemSize, ahi->info_driver->externalMemSize / 1024);
	LOG("ATI CAPS 1: %d (0x%08X)\n", ahi->info_driver->caps1, ahi->info_driver->caps1);
	LOG("ATI CAPS 2: %d (0x%08X)\n", ahi->info_driver->caps2, ahi->info_driver->caps2);

	LOG("ATI Surface Screen Info: width=%d, height=%d, pixFormat=%d, byteSize=%d, byteSize=%d KiB\n",
		ahi->info_surface_screen.width, ahi->info_surface_screen.height,
		ahi->info_surface_screen.pixFormat,
		ahi->info_surface_screen.byteSize, ahi->info_surface_screen.byteSize / 1024);
	LOG("ATI Surface Screen Info: offset=%d, stride=%d, numPlanes=%d\n",
		ahi->info_surface_screen.offset,
		ahi->info_surface_screen.stride,
		ahi->info_surface_screen.numPlanes);

	LOG("ATI Surface Draw Info: width=%d, height=%d, pixFormat=%d, byteSize=%d, byteSize=%d KiB\n",
		ahi->info_surface_draw.width, ahi->info_surface_draw.height,
		ahi->info_surface_draw.pixFormat,
		ahi->info_surface_draw.byteSize, ahi->info_surface_draw.byteSize / 1024);
	LOG("ATI Surface Draw Info: offset=%d, stride=%d, numPlanes=%d\n",
		ahi->info_surface_draw.offset,
		ahi->info_surface_draw.stride,
		ahi->info_surface_draw.numPlanes);

	return RESULT_OK;
}

static UINT32 ATI_Driver_Log_Memory(APP_AHI_T *ahi, AHIPIXFMT_T pixel_format) {
	enum {
		INTERNAL_MEMORY,
		EXTERNAL_MEMORY,
		SYSTEM_MEMORY,
		MEMORY_MAX
	};

	UINT32 status[MEMORY_MAX] = { 0 };
	UINT32 sizes[MEMORY_MAX] = { 0 };
	UINT32 alignment[MEMORY_MAX] = { 0 };

	status[INTERNAL_MEMORY] = AhiSurfGetLargestFreeBlockSize(ahi->context, pixel_format,
		&sizes[INTERNAL_MEMORY], &alignment[INTERNAL_MEMORY], AHIFLAG_INTMEMORY);
	status[EXTERNAL_MEMORY] = AhiSurfGetLargestFreeBlockSize(ahi->context, pixel_format,
		&sizes[EXTERNAL_MEMORY], &alignment[EXTERNAL_MEMORY], AHIFLAG_EXTMEMORY);
	status[SYSTEM_MEMORY] = AhiSurfGetLargestFreeBlockSize(ahi->context, pixel_format,
		&sizes[SYSTEM_MEMORY], &alignment[SYSTEM_MEMORY], AHIFLAG_SYSMEMORY);

	LOG("%s\n", "ATI Memory Dump.");
	LOG("\tATI Internal Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[INTERNAL_MEMORY], pixel_format, sizes[INTERNAL_MEMORY], sizes[INTERNAL_MEMORY] / 1024, alignment[INTERNAL_MEMORY]);
	LOG("\tATI External Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[EXTERNAL_MEMORY], pixel_format, sizes[EXTERNAL_MEMORY], sizes[EXTERNAL_MEMORY] / 1024, alignment[EXTERNAL_MEMORY]);
	LOG("\tATI System Memory Largest Block: status=%d, pixel_format=%d, size=%d, size=%d KiB, align=%d\n",
		status[SYSTEM_MEMORY], pixel_format, sizes[SYSTEM_MEMORY], sizes[SYSTEM_MEMORY] / 1024, alignment[SYSTEM_MEMORY]);

	return status[INTERNAL_MEMORY] && status[EXTERNAL_MEMORY] && status[SYSTEM_MEMORY];
}

UINT32 ATI_Driver_Start(APP_AHI_T *ahi) {
	UINT32 status;
	INT32 result;
	AHIDEVICE_T ahi_device;
	AHIDISPMODE_T display_mode;

	status = RESULT_OK;
	result = RESULT_OK;

	ahi->info_driver = suAllocMem(sizeof(AHIDRVINFO_T), &result);
	if (!ahi->info_driver && result) {
		return RESULT_FAIL;
	}
	status |= AhiDevEnum(&ahi_device, ahi->info_driver, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}
	status |= AhiDevOpen(&ahi->context, ahi_device, g_app_name, 0);
	if (status != RESULT_OK) {
		return RESULT_FAIL;
	}

	status |= AhiDispModeGet(ahi->context, &display_mode);

	status |= AhiDispSurfGet(ahi->context, &ahi->screen);
	ahi->draw = DAL_GetDrawingSurface(DISPLAY_MAIN);

	status |= AhiDrawClipDstSet(ahi->context, NULL);
	status |= AhiDrawClipSrcSet(ahi->context, NULL);

	status |= AhiSurfInfo(ahi->context, ahi->screen, &ahi->info_surface_screen);
	status |= AhiSurfInfo(ahi->context, ahi->draw, &ahi->info_surface_draw);

	ahi->width = ahi->info_surface_screen.width;
	ahi->height = ahi->info_surface_screen.height;

	ahi->update_params.size = sizeof(AHIUPDATEPARAMS_T);
	ahi->update_params.sync = FALSE;
	ahi->update_params.rect.x1 = 0;
	ahi->update_params.rect.y1 = 0;
	ahi->update_params.rect.x2 = 0 + ahi->width;
	ahi->update_params.rect.y2 = 0 + ahi->height;
	ahi->point_bitmap.x = 0;
	ahi->point_bitmap.y = 0;
	ahi->is_CSTN_display = (ahi->width < DISPLAY_WIDTH) || (ahi->height < DISPLAY_HEIGHT); /* Motorola L6 */

	ahi->bitmap.width = ahi->bmp_width;
	ahi->bitmap.height = ahi->bmp_height;
	ahi->bitmap.stride = ahi->bmp_width; /* (width * bpp) */
	ahi->bitmap.format = AHIFMT_8BPP;
	ahi->bitmap.image = suAllocMem(ahi->bmp_width * ahi->bmp_height, &result);
	if (result) {
		return RESULT_FAIL;
	}
	ahi->rect_bitmap.x1 = 0;
	ahi->rect_bitmap.y1 = 0;
	ahi->rect_bitmap.x2 = 0 + ahi->bmp_width;
	ahi->rect_bitmap.y2 = 0 + ahi->bmp_height;

	ahi->rect_draw.x1 = 0;
	ahi->rect_draw.y1 = ahi->bmp_height + 1;
	ahi->rect_draw.x2 = 0 + ahi->bmp_height;
	ahi->rect_draw.y2 = ahi->bmp_height + 1 + ahi->bmp_width;

	status |= AhiDrawSurfDstSet(ahi->context, ahi->screen, 0);

	status |= AhiDrawBrushFgColorSet(ahi->context, ATI_565RGB(0xFF, 0xFF, 0xFF));
	status |= AhiDrawBrushSet(ahi->context, NULL, NULL, 0, AHIFLAG_BRUSH_SOLID);
	status |= AhiDrawRopSet(ahi->context, AHIROP3(AHIROP_PATCOPY));
	status |= AhiDrawSpans(ahi->context, &ahi->update_params.rect, 1, 0);

	AhiDrawRopSet(ahi->context, AHIROP3(AHIROP_SRCCOPY));

	status |= ATI_Display_Mode_Log(&display_mode);
	status |= ATI_Driver_Log(ahi);
	status |= ATI_Driver_Log_Memory(ahi, AHIFMT_16BPP_565);

	return status;
}

UINT32 ATI_Driver_Stop(APP_AHI_T *ahi) {
	UINT32 status;

	status = RESULT_OK;

	status |= AhiDevClose(ahi->context);
	if (ahi->info_driver) {
		suFreeMem(ahi->info_driver);
	}

	return status;
}

UINT32 ATI_Driver_Flush(APP_AHI_T *ahi) {
	AhiDrawSurfDstSet(ahi->context, ahi->draw, 0);
	AhiDrawBitmapBlt(ahi->context,
		&ahi->rect_bitmap, &ahi->point_bitmap, &ahi->bitmap, (void *) fire_palette, 0);

	AhiDrawRotateBlt(ahi->context,
		&ahi->rect_draw, &ahi->point_bitmap, AHIROT_90, AHIMIRR_NO, 0);

	AhiDrawSurfSrcSet(ahi->context, ahi->draw, 0);
	AhiDrawSurfDstSet(ahi->context, ahi->screen, 0);

	AhiDispWaitVBlank(ahi->context, 0);
	AhiDrawStretchBlt(ahi->context, &ahi->update_params.rect, &ahi->rect_draw, AHIFLAG_STRETCHFAST);

	if (ahi->is_CSTN_display) {
		AhiDispUpdate(ahi->context, &ahi->update_params);
	}

	return RESULT_OK;
}

UINT32 GFX_Draw_Start(APP_AHI_T *ahi) {
	ahi->y_coord = START_Y_COORD;
	ahi->p_fire = (UINT8 *) ahi->bitmap.image;

	/* Fill all screen to RGB(0x07, 0x07, 0x07) except last line. */
	memset(ahi->p_fire, 0, ahi->bmp_width * (ahi->bmp_height - 1));

	/* Fill last line to RGB(0xFF, 0xFF, 0xFF) except last line. */
	memset((UINT8 *) (ahi->p_fire + (ahi->bmp_height - 1) * ahi->bmp_width), 36, ahi->bmp_width);

	return RESULT_OK;
}

UINT32 GFX_Draw_Stop(APP_AHI_T *ahi) {
	if (ahi->p_fire) {
		suFreeMem(ahi->p_fire);
		ahi->p_fire = NULL;
	}

	return RESULT_OK;
}

static BOOL Fire_Demo_Is_Screen_Empty(APP_AHI_T *ahi) {
	UINT16 i;
	UINT16 stop;

	stop = ahi->bmp_width * ahi->bmp_height;
	for (i = 0; i < stop; ++i) {
		if (ahi->p_fire[i]) {
			return FALSE;
		}
	}

	return TRUE;
}

UINT32 GFX_Draw_Step(APP_AHI_T *ahi) {
	UINT16 x;
	UINT16 y;
	UINT16 start;
	UINT16 stop;

	if (ahi->flag_restart_demo) {
		GFX_Draw_Start(ahi);
		ahi->flag_restart_demo = FALSE;
		return RESULT_FAIL;
	}

	for (x = 0; x < ahi->bmp_width; ++x) {
		for (y = 1; y < ahi->bmp_height; ++y) {
			const UINT8 pixel = ahi->p_fire[y * ahi->bmp_width + x];
			if (pixel == 0) {
				ahi->p_fire[(y * ahi->bmp_width + x) - ahi->bmp_width] = 0;
			} else {
				const UINT8 rand_idx = rand() % 4;
				const UINT16 destination = (y * ahi->bmp_width + x) - rand_idx + 1;
				ahi->p_fire[destination - ahi->bmp_width] = pixel - (rand_idx & 1);
			}
		}
	}

	start = ahi->bmp_height - 1;
	stop = ahi->bmp_height - 8;

	if (ahi->y_coord != ahi->bmp_height / 4) {
		ahi->y_coord -= 2;
	} else {
		for(y = start; y > stop; --y) {
			for(x = 0; x < ahi->bmp_width; ++x) {
				if (ahi->p_fire[y * ahi->bmp_width + x] > 0) {
					ahi->p_fire[y * ahi->bmp_width + x] -= ((rand() % 2) & 3);
				}
			}
		}
		ahi->flag_restart_demo = Fire_Demo_Is_Screen_Empty(ahi);
	}

	return RESULT_OK;
}
