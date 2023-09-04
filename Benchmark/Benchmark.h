#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <apps.h>
#include <typedefs.h>
#if defined(EP1) || defined(EP2)
#include <ati.h>
#endif

#define RESULT_STRING                  (64)
#define FLOAT_STRING                   (16)

extern char float_string[FLOAT_STRING];

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

/* Pass 1 */
#define BITMAP_WIDTH_LOW               (32)
#define BITMAP_HEIGHT_LOW              (24)

/* Pass 2 */
#define BITMAP_WIDTH_MID               (48)
#define BITMAP_HEIGHT_MID              (32)

/* Pass 3 */
#define BITMAP_WIDTH_HIGH              (64)
#define BITMAP_HEIGHT_HIGH             (48)

#define START_Y_COORD                  (220)
#define MAX_FPS_COUNT                  (64)

typedef struct {
	BOOL is_CSTN_display;
	UINT16 width;
	UINT16 height;
	UINT16 bmp_width;
	UINT16 bmp_height;

	UINT8 *p_fire;
	UINT16 y_coord;
	BOOL flag_restart_demo;

#if defined(EP1) || defined(EP2)
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
#endif // EP1 || EP2

} APP_AHI_T;

typedef struct {
	UINT16 fps_i;
	UINT16 fps_f;
} FPS_T;

typedef struct {
	UINT16 size;
	FPS_T values[MAX_FPS_COUNT];
} FPS_VALUES_T;

typedef struct {
	WCHAR fps_pass1[RESULT_STRING];
	WCHAR fms_pass1[RESULT_STRING];
	WCHAR fps_pass2[RESULT_STRING];
	WCHAR fms_pass2[RESULT_STRING];
	WCHAR fps_pass3[RESULT_STRING];
	WCHAR fms_pass3[RESULT_STRING];
	WCHAR properties[RESULT_STRING * 4];
} BENCHMARK_RESULTS_GPU_T;

extern const char g_app_name[APP_NAME_LEN];

extern UINT32 ATI_Driver_Start(APP_AHI_T *ahi, WCHAR *props);
extern UINT32 ATI_Driver_Flush(APP_AHI_T *ahi);
extern UINT32 ATI_Driver_Stop(APP_AHI_T *ahi);

extern UINT32 GFX_Draw_Start(APP_AHI_T *ahi);
extern UINT32 GFX_Draw_Step(APP_AHI_T *ahi);
extern UINT32 GFX_Draw_Stop(APP_AHI_T *ahi);

extern void FPS_Meter(void);

extern UINT32 CalculateAverageFpsAndTime(WCHAR *result_fps, WCHAR *result_fms);

extern UINT32 Bench_GPU_Passes(UINT32 bmp_width, UINT32 bmp_height, WCHAR *fps, WCHAR *fms, WCHAR *props);

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
