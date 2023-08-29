#include <loader.h>
#include <utilities.h>

#include "Phases.h"

#if __CC_ARM && __arm
#define nop() \
	__asm { \
		nop \
	}
#endif

static __inline void BogoMIPS_Delay(int loops);

UINT32 BogoMIPS(BENCHMARK_RESULTS_CPU_T *result) {
	UINT32 loops_per_sec = 1;

	while ((loops_per_sec <<= 1)) {
		UINT64 ticks;
		UINT32 delta;

		ticks = suPalReadTime();

		BogoMIPS_Delay(loops_per_sec);

		delta = (UINT32) (suPalReadTime() - ticks);

		if (delta >= CLOCKS_PER_SEC) {
			UINT32 lps = loops_per_sec;
			UINT32 bmips_i;
			UINT32 bmips_f;
			lps = (lps / delta) * CLOCKS_PER_SEC;

			result->ms = suPalTicksToMsec(delta);
			bmips_i = lps / 500000;
			bmips_f = (lps / 5000) % 100;

			u_ltou(bmips_i, result->bogomips);
			u_strcpy(result->bogomips + u_strlen(result->bogomips), L".");
			u_ltou(bmips_f, result->bogomips + u_strlen(result->bogomips));

			LOG("OK: Delta Ticks: %lu\n", delta);
			LOG("OK: Delta Ms: %lu\n", result->ms);
			LOG("OK: Loops/s: %lu\n", loops_per_sec);
			LOG("OK: BogoMIPS: %lu.%02lu\n", bmips_i, bmips_f);
			return RESULT_OK;
		}
	}

	LOG("FAIL: %s\n", "Cannot calculate BogoMIPS!");
	return RESULT_FAIL;
}

//#pragma O2
static __inline void BogoMIPS_Delay(int loops) {
	int i;
	for (i = loops; !!(i > 0); --i) {
//		nop();
		;
	}
}
//#pragma O2
