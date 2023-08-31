#include <loader.h>
#include <utilities.h>

#include "Phases.h"

#if __CC_ARM && __arm
#define nop() \
	__asm { \
		nop \
	}
#endif

//#pragma O2
static void BogoMIPS_Delay(long loops) {
	long i;
	for (i = loops; !!(i > 0); --i) {
		;
//		nop();
	}
}
//#pragma O2

UINT32 BogoMIPS(BENCHMARK_RESULTS_CPU_T *result) {
	UINT32 loops_per_sec = 1;

	while ((loops_per_sec <<= 1)) {
		UINT64 ticks;
		UINT32 delta;

		ticks = suPalReadTime();

		BogoMIPS_Delay(loops_per_sec);

		delta = (UINT32) (suPalReadTime() - ticks);

		if (delta >= TICKS_PER_SEC) {
			UINT32 lps = loops_per_sec;
			UINT32 bmips_i;
			UINT32 bmips_f;
			lps = (lps / delta) * TICKS_PER_SEC;

			bmips_i = lps / 500000;
			bmips_f = (lps / 5000) % 100;

			u_ltou((UINT32) suPalTicksToMsec(delta), result->bogo_time);
			u_strcpy(result->bogo_time + u_strlen(result->bogo_time), L" ms");

			u_ltou(bmips_i, result->bogo_mips);
			u_strcpy(result->bogo_mips + u_strlen(result->bogo_mips), L".");
			u_ltou(bmips_f, result->bogo_mips + u_strlen(result->bogo_mips));
			u_strcpy(result->bogo_mips + u_strlen(result->bogo_mips), L" BMIPS");

			LOG("OK: Delta Ticks: %lu\n", delta);
			LOG("OK: Delta Ms: %lu\n", (UINT32) suPalTicksToMsec(delta));
			LOG("OK: Loops/s: %lu\n", loops_per_sec);
			LOG("OK: BogoMIPS: %lu.%02lu\n", bmips_i, bmips_f);

			return RESULT_OK;
		}
	}

	LOG("FAIL: %s\n", "Cannot calculate BogoMIPS!");
	return RESULT_FAIL;
}
