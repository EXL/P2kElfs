#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <apps.h>
#include <ati.h>
#include <typedefs.h>

#define RESULT_STRING                  (64)

/* CPU Benchmark */

#define TICKS_PER_SEC                  (8192)
#define DHRYSTONE_LOOP_RUNS            (20000) /* 2000 - Import from GBA */
#define DMIPS_VAX_11_780_CONST         (1757)

typedef struct {
	WCHAR bogo_time[RESULT_STRING];
	WCHAR bogo_mips[RESULT_STRING];

	WCHAR dhrys_time[RESULT_STRING];
	WCHAR dhrys_score[RESULT_STRING];
	WCHAR dhrys_mips[RESULT_STRING];
} BENCHMARK_RESULTS_CPU_T;

extern UINT32 BogoMIPS(BENCHMARK_RESULTS_CPU_T *result);

extern int Dhrystone(BENCHMARK_RESULTS_CPU_T *result);

/* GPU Benchmark */

#define BITMAP_WIDTH                   (64)
#define BITMAP_HEIGHT                  (48)
#define START_Y_COORD                  (220)
#define FPS_METER

typedef struct {
	BOOL is_CSTN_display;
	UINT16 width;
	UINT16 height;
	UINT16 bmp_width;
	UINT16 bmp_height;

	UINT8 *p_fire;
	UINT16 y_coord;
	BOOL flag_restart_demo;

	AHIDRVINFO_T *info_driver;
	AHIDEVCONTEXT_T context;
	AHISURFACE_T screen;
	AHISURFACE_T draw;
	AHISURFINFO_T info_surface_screen;
	AHISURFINFO_T info_surface_draw;
	AHIBITMAP_T bitmap;

	AHIPOINT_T point_bitmap;
	AHIRECT_T rect_bitmap;
	AHIRECT_T rect_draw;
	AHIUPDATEPARAMS_T update_params;
} APP_AHI_T;

typedef struct {
	WCHAR fps[RESULT_STRING];
	WCHAR properties[RESULT_STRING];
} BENCHMARK_RESULTS_GPU_T;

extern const char g_app_name[APP_NAME_LEN];

extern UINT32 ATI_Driver_Start(APP_AHI_T *ahi);
extern UINT32 ATI_Driver_Flush(APP_AHI_T *ahi);
extern UINT32 ATI_Driver_Stop(APP_AHI_T *ahi);

extern UINT32 GFX_Draw_Start(APP_AHI_T *ahi);
extern UINT32 GFX_Draw_Step(APP_AHI_T *ahi);
extern UINT32 GFX_Draw_Stop(APP_AHI_T *ahi);

extern void FPS_Meter(void);

/* RAM Benchmark */

#define RAM_TOP_BLOCKS_COUNT           (6)
#define RAM_STEP_SIZE                  (256)
#define RAM_TOTAL_BLOCKS_COUNT         (512)
#define RAM_START_SIZE_TOTAL           (RAM_STEP_SIZE * 4)
#define RAM_START_SIZE_BLOCK           (RAM_STEP_SIZE * 8)

typedef struct {
	WCHAR total[RESULT_STRING];
	WCHAR blocks[RAM_TOP_BLOCKS_COUNT][RESULT_STRING];
} BENCHMARK_RESULTS_RAM_T;

typedef struct {
	void *block_address;
	UINT32 block_size;
	UINT32 block_time;
} RAM_ALLOCATED_BLOCK_T;

extern UINT32 TopOfBiggestRamBlocks(BENCHMARK_RESULTS_RAM_T *result);
extern UINT32 TotalRamSize(BENCHMARK_RESULTS_RAM_T *result);

/* Java Heap Benchmark */

#define HEAP_STEP_SIZE                 (4096)
#define HEAP_TOTAL_BLOCKS_COUNT        (512)
#define HEAP_START_SIZE_TOTAL          (HEAP_STEP_SIZE * 8)

typedef struct {
	WCHAR total[RESULT_STRING];
	WCHAR desc[RESULT_STRING];
} BENCHMARK_RESULTS_HEAP_T;

typedef struct {
	void *block_address;
	UINT32 block_size;
} HEAP_ALLOCATED_BLOCK_T;

extern UINT32 TotalHeapSize(BENCHMARK_RESULTS_HEAP_T *result);

#endif // BENCHMARK_H