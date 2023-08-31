#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <typedefs.h>

#define RESULT_STRING                  (64)
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

#endif // BENCHMARK_H
