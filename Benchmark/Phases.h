#ifndef BENCHMARK_H
#define BENCHMARK_H

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
