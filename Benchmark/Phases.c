#include <loader.h>
#include <utilities.h>
#include <mem.h>

#include "Phases.h"

#if __CC_ARM && __arm
#define nop() \
	__asm { \
		nop \
	}
#endif

//#pragma O0
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

			LOG("CPU: Delta ticks: %lu\n", delta);
			LOG("CPU: Delta ms: %lu\n", (UINT32) suPalTicksToMsec(delta));
			LOG("CPU: Loops/s: %lu\n", loops_per_sec);
			LOG("CPU: BogoMIPS: %lu.%02lu\n", bmips_i, bmips_f);

			return RESULT_OK;
		}
	}

	LOG("CPU: Error: %s\n", "Cannot calculate BogoMIPS!");
	return RESULT_FAIL;
}

static void *AllocateBiggestBlock(UINT32 start_size, UINT32 *max_block_size) {
	UINT32 size;
	INT32 error;
	void *block_address;

	size = start_size;
	error = RESULT_OK;
	block_address = NULL;

	while (error == RESULT_OK) {
		block_address = suAllocMem(size, &error);
		if (error == RESULT_OK) {
			suFreeMem(block_address);
			size += RAM_STEP_SIZE * 4;
		} else {
			while (error != RESULT_OK && size > start_size) {
				size -= RAM_STEP_SIZE;
				block_address = suAllocMem(size, &error);
			}
			break;
		}
	}

	if (block_address) {
		*max_block_size = size;
	} else {
		*max_block_size = 0;
	}

	return block_address;
}

UINT32 TopOfBiggestRamBlocks(BENCHMARK_RESULTS_RAM_T *result) {
	UINT16 i;
	UINT32 status;
	UINT64 time_start;
	UINT64 time_end;
	RAM_ALLOCATED_BLOCK_T top_blocks[RAM_TOP_BLOCKS_COUNT];

	status = RESULT_OK;

	for (i = 0; i < RAM_TOP_BLOCKS_COUNT; ++i) {
		top_blocks[i].block_address = NULL;
		top_blocks[i].block_size = 0;
		top_blocks[i].block_time = 0;

		time_start = suPalReadTime();
		top_blocks[i].block_address = AllocateBiggestBlock(RAM_STEP_SIZE * 8, &top_blocks[i].block_size);
		time_end = suPalReadTime();
		top_blocks[i].block_time = (UINT32) suPalTicksToMsec(time_end - time_start);

		LOG("RAM: Block %d time: %d\n", i + 1, top_blocks[i].block_time);
		LOG("RAM: Block %d size: %d\n", i + 1, top_blocks[i].block_size);

		u_ltou(top_blocks[i].block_time, result->blocks[i]);
		u_strcpy(result->blocks[i] + u_strlen(result->blocks[i]), L" ms | ");
		u_ltou(top_blocks[i].block_size, result->blocks[i] + u_strlen(result->blocks[i]));
		u_strcpy(result->blocks[i] + u_strlen(result->blocks[i]), L" B");
	}

	for (i = 0; i < RAM_TOP_BLOCKS_COUNT; ++i) {
		if (!top_blocks[i].block_address) {
			status = RESULT_FAIL;
		}
		suFreeMem(top_blocks[i].block_address);
	}

	return status;
}

UINT32 TotalRamSize(BENCHMARK_RESULTS_RAM_T *result) {
	UINT16 i;
	UINT32 status;
	UINT32 total_size;
	UINT64 time_start;
	UINT64 time_end;
	UINT64 time_result;
	RAM_ALLOCATED_BLOCK_T ram_blocks[RAM_TOTAL_BLOCKS_COUNT];

	status = RESULT_OK;
	i = 0;
	total_size = 0;

	time_start = suPalReadTime();

	do {
		ram_blocks[i].block_address = AllocateBiggestBlock(RAM_STEP_SIZE * 4, &ram_blocks[i].block_size);
		total_size += ram_blocks[i].block_size;
	} while (ram_blocks[i++].block_address != NULL);

	time_end = suPalReadTime();

	i -= 1;
	while (i-- > 0) {
		suFreeMem(ram_blocks[i].block_address);
	}

	time_result = (UINT32) suPalTicksToMsec(time_end - time_start);

	LOG("RAM: Total time: %d\n", time_result);
	LOG("RAM: Total size: %d\n", total_size);

	u_ltou(time_result, result->total);
	u_strcpy(result->total + u_strlen(result->total), L" ms | ");
	u_ltou(total_size, result->total + u_strlen(result->total));
	u_strcpy(result->total + u_strlen(result->total), L" B");

	return status;
}
