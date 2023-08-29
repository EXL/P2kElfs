#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <typedefs.h>

#define BOGOMIPS_STRING              (16)
#define CLOCKS_PER_SEC               (8192)

typedef struct {
	WCHAR bogomips[BOGOMIPS_STRING];
	UINT32 ms;
} BENCHMARK_RESULTS_CPU_T;

extern UINT32 BogoMIPS(BENCHMARK_RESULTS_CPU_T *result);

#endif // BENCHMARK_H
