#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <typedefs.h>

#define RESULT_STRING                  (64)

/* CPU Benchmark */

#define TICKS_PER_SEC                (8192)
#define DHRYSTONE_LOOP_RUNS         (20000) /* 2000 - Import from GBA */
#define DMIPS_VAX_11_780_CONST       (1757)

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

#define RAM_TOP_BLOCKS_COUNT            (3)
#define RAM_STEP_SIZE                 (512)
#define RAM_TOTAL_BLOCKS_COUNT        (256)

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

#endif // BENCHMARK_H
