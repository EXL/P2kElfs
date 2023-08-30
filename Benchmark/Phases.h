#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <typedefs.h>

#define RESULT_STRING                  (64)
#define TICKS_PER_SEC                (8192)

typedef struct {
	WCHAR bogomips_res[RESULT_STRING];
	WCHAR bogomips_time[RESULT_STRING];

	WCHAR dhrys_res[RESULT_STRING];
	WCHAR dhrys_time[RESULT_STRING];
} BENCHMARK_RESULTS_CPU_T;

extern UINT32 BogoMIPS(BENCHMARK_RESULTS_CPU_T *result);

#endif // BENCHMARK_H
