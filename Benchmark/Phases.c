#include <loader.h>
#include <filesystem.h>
#include <utilities.h>
#include <mem.h>
#if defined(EP1) || defined(EP2)
#include <tasks.h>
#endif

#include "Benchmark.h"

char float_string[FLOAT_STRING];

static UINT32 DeleteFileIfExists(const WCHAR *file_path);

#if defined(PALMOS_BOGOMIPS)
/* This is the number of bits of precision for the loops_per_second.  Each
   bit takes on average 1.5/HZ seconds.  This (like the original) is a little
   better than 1% */
#define LPS_PREC 8
#define HZ 1000
#define TimGetTicks() ((UINT32) suPalTicksToMsec(suPalReadTime()))

UINT32 BogoMIPS(BENCHMARK_RESULTS_CPU_T *result) {
	UINT32 ticks;
	UINT32 loopbit;
	UINT32 bmips_i;
	UINT32 bmips_f;
	UINT64 delta_a;
	UINT64 delta_b;
	UINT64 delta;
	UINT32 lps_precision = LPS_PREC;
	UINT32 loops_per_sec = 1;

	delta_a = suPalReadTime();

	while (loops_per_sec <<= 1) {  //end while when counter overflows
		/* wait for "start of" clock tick */
		ticks = TimGetTicks();
		while (ticks == TimGetTicks())
			/* nothing */;
		/* Go .. */
		ticks = TimGetTicks();
		delay_bmips(loops_per_sec);
		ticks = TimGetTicks() - ticks;
		if (ticks) /* Break as soon as get delay longer than 1 tick*/
			break;
	}

	/* Do a binary approximation to get loops_per_jiffy set to equal one clock (up to lps_precision bits) */
	loops_per_sec >>= 1;

	loopbit = loops_per_sec;
	while (lps_precision-- && (loopbit >>= 1)) {
		loops_per_sec |= loopbit;
		ticks = TimGetTicks();
		while (ticks == TimGetTicks());
		ticks = TimGetTicks();
		delay_bmips(loops_per_sec);
		if (TimGetTicks() != ticks) {  /* longer than 1 tick */
			loops_per_sec &= ~loopbit;
		}
	}

	loops_per_sec *= HZ / 1000;

	bmips_i = loops_per_sec / (500000 / HZ);
	bmips_f = (loops_per_sec / (5000 / HZ)) % 100;

	delta_b = suPalReadTime();
	delta = delta_b - delta_a;

	u_ltou((UINT32) suPalTicksToMsec(delta), result->bogo_time);
	u_strcpy(result->bogo_time + u_strlen(result->bogo_time), L" ms");

	sprintf(float_string, "%lu.%02lu", bmips_i, bmips_f);
	u_atou(float_string, result->bogo_mips);
	u_strcpy(result->bogo_mips + u_strlen(result->bogo_mips), L" BMIPS");

	LOG("CPU: Delta A ticks: %llu\n", delta_a);
	LOG("CPU: Delta A ms: %lu\n", (UINT32) suPalTicksToMsec(delta_a));
	LOG("CPU: Delta B ticks: %llu\n", delta_b);
	LOG("CPU: Delta B ms: %lu\n", (UINT32) suPalTicksToMsec(delta_b));
	LOG("CPU: Delta ticks: %llu\n", delta);
	LOG("CPU: Delta ms: %lu\n", (UINT32) suPalTicksToMsec(delta));
	LOG("CPU: Loops/s: %lu\n", loops_per_sec);
	LOG("CPU: BogoMIPS: %lu.%02lu\n", bmips_i, bmips_f);

	return RESULT_OK;
}
#endif

#if defined(LINUX_BOGOMIPS)
UINT32 BogoMIPS(BENCHMARK_RESULTS_CPU_T *result) {
	UINT32 loops_per_sec = 1;

	while ((loops_per_sec *= 2)) {
		UINT64 delta_a;
		UINT64 delta_b;
		UINT32 delta;

		delta_a = suPalReadTime();

		delay_bmips(loops_per_sec);

		delta_b = suPalReadTime();

		delta = (UINT32) (delta_b - delta_a);

		D("=> %lu %lu\n", loops_per_sec, delta);

		if (delta >= TICKS_PER_SEC) {
			UINT32 lps = loops_per_sec;
			UINT32 bmips_i;
			UINT32 bmips_f;
			lps = (lps / delta) * TICKS_PER_SEC;

			bmips_i = lps / 500000;
			bmips_f = (lps / 5000) % 100;

			u_ltou((UINT32) suPalTicksToMsec(delta), result->bogo_time);
			u_strcpy(result->bogo_time + u_strlen(result->bogo_time), L" ms");

			sprintf(float_string, "%lu.%02lu", bmips_i, bmips_f);
			u_atou(float_string, result->bogo_mips);
			u_strcpy(result->bogo_mips + u_strlen(result->bogo_mips), L" BMIPS");

			LOG("CPU: Delta A ticks: %llu\n", delta_a);
			LOG("CPU: Delta A ms: %lu\n", (UINT32) suPalTicksToMsec(delta_a));
			LOG("CPU: Delta B ticks: %llu\n", delta_b);
			LOG("CPU: Delta B ms: %lu\n", (UINT32) suPalTicksToMsec(delta_b));
			LOG("CPU: Delta ticks: %lu\n", delta);
			LOG("CPU: Delta ms: %lu\n", (UINT32) suPalTicksToMsec(delta));
			LOG("CPU: Loops/s: %lu\n", loops_per_sec);
			LOG("CPU: BogoMIPS: %lu.%02lu\n", bmips_i, bmips_f);

			return RESULT_OK;
		}
	}

	u_strcpy(result->bogo_time, L"Error");
	u_strcpy(result->bogo_mips, L"Error");

	LOG("CPU: Error: %s\n", "Cannot calculate BogoMIPS!");
	return RESULT_FAIL;
}
#endif

#if defined(__MCORE__)
void *AmMemAllocPointer(int size) {
	INT32 result;
	void *address = suAllocMem(size, &result);
	return (result != RESULT_OK) ? NULL : address;
}

void AmMemFreePointer(void *ptr) {
	suFreeMem(ptr);
}
#endif

static void *AllocateBiggestBlock(UINT32 start_size, UINT32 *max_block_size, UINT32 step, BOOL java_heap) {
	UINT32 size;
	INT32 error;
	void *block_address;

	size = start_size;
	error = RESULT_OK;
	block_address = NULL;

	while (error == RESULT_OK) {
		if (java_heap) {
			block_address = AmMemAllocPointer(size);
			if (block_address == NULL) {
				error = RESULT_FAIL;
			}
		} else {
			block_address = suAllocMem(size, &error);
		}
		if (error == RESULT_OK) {
			if (java_heap) {
				AmMemFreePointer(block_address);
			} else {
				suFreeMem(block_address);
			}
			size += step * 4;
		} else {
			while (error != RESULT_OK && size > start_size) {
				size -= step;
				if (java_heap) {
					block_address = AmMemAllocPointer(size);
					if (block_address != NULL) {
						error = RESULT_OK;
					}
				} else {
					block_address = suAllocMem(size, &error);
				}
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
		top_blocks[i].block_address = AllocateBiggestBlock(
			RAM_START_SIZE_BLOCK, &top_blocks[i].block_size, RAM_STEP_SIZE, FALSE
		);
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
	UINT32 time_result;
	RAM_ALLOCATED_BLOCK_T ram_blocks[RAM_TOTAL_BLOCKS_COUNT];

	status = RESULT_OK;
	i = 0;
	total_size = 0;

	time_start = suPalReadTime();

	do {
		ram_blocks[i].block_address = AllocateBiggestBlock(
			RAM_START_SIZE_TOTAL, &ram_blocks[i].block_size, RAM_STEP_SIZE, FALSE
		);
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

UINT32 TotalHeapSize(BENCHMARK_RESULTS_HEAP_T *result) {
	UINT16 i;
	UINT32 status;
	UINT32 total_size;
	UINT64 time_start;
	UINT64 time_end;
	UINT32 time_result;
	HEAP_ALLOCATED_BLOCK_T heap_blocks[HEAP_TOTAL_BLOCKS_COUNT];

	status = RESULT_OK;
	i = 0;
	total_size = 0;

	time_start = suPalReadTime();

	/* Can I use Java Heap functions? */
	heap_blocks[i].block_address = AmMemAllocPointer(HEAP_STEP_SIZE);
	if (heap_blocks[i].block_address != NULL) {
		AmMemFreePointer(heap_blocks[i].block_address);
	} else {
		status = RESULT_FAIL;
	}

	do {
		heap_blocks[i].block_address = AllocateBiggestBlock(
			HEAP_START_SIZE_TOTAL, &heap_blocks[i].block_size, HEAP_STEP_SIZE, TRUE
		);
		total_size += heap_blocks[i].block_size;
	} while (heap_blocks[i++].block_address != NULL);

	time_end = suPalReadTime();

	i -= 1;
	while (i-- > 0) {
		AmMemFreePointer(heap_blocks[i].block_address);
	}

	time_result = (UINT32) suPalTicksToMsec(time_end - time_start);

	LOG("HEAP: Total time: %d\n", time_result);
	LOG("HEAP: Total size: %d\n", total_size);

	u_ltou(time_result, result->total);
	u_strcpy(result->total + u_strlen(result->total), L" ms | ");
	u_ltou(total_size, result->total + u_strlen(result->total));
	u_strcpy(result->total + u_strlen(result->total), L" B");

	{
		UINT32 time_i;
		UINT32 time_f;
		UINT32 size_i;
		UINT32 size_f;

		time_i = time_result / 1000;
		time_f = ((time_result % 1000) * 100) / 1000;

		size_i = total_size / 1024;
		size_f = ((total_size % 1024) * 100) / 1024;

		sprintf(float_string, "%lu.%02lu", time_i, time_f);
		u_atou(float_string, result->desc);
		u_strcpy(result->desc + u_strlen(result->desc), L" sec | ");

		sprintf(float_string, "%lu.%02lu", size_i, size_f);
		u_atou(float_string, result->desc + u_strlen(result->desc));
		u_strcpy(result->desc + u_strlen(result->desc), L" KiB");
	}

	return status;
}

#if defined(EP1) || defined(EP2)
UINT32 Bench_GPU_Passes(UINT32 bmp_width, UINT32 bmp_height, WCHAR *fps, WCHAR *fms, WCHAR *props) {
	UINT32 status;
	APP_AHI_T ahi;

	ahi.info_driver = NULL;
	ahi.bmp_width = bmp_width;
	ahi.bmp_height = bmp_height;
	ahi.p_fire = NULL;
	ahi.flag_restart_demo = FALSE;

	ATI_Driver_Start(&ahi, props);
	GFX_Draw_Start(&ahi);

	do {
		FPS_Meter();
		status = GFX_Draw_Step(&ahi);
		ATI_Driver_Flush(&ahi);
	} while (status == RESULT_OK);

	GFX_Draw_Stop(&ahi);
	ATI_Driver_Stop(&ahi);

	CalculateAverageFpsAndTime(fps, fms);

	return status;
}
#endif

UINT32 DisksResult(WCHAR *disk_result) {
	UINT32 i;
	UINT32 status;
	WCHAR *result;
	VOLUME_DESCR_T volume_description;
	WCHAR volumes[MAX_VOLUMES_COUNT * 3];
	UINT32 disk_count;
	WCHAR disks[LENGTH_VOLUME_NAME * MAX_VOLUMES_COUNT];

	disk_count = 0;
	status = RESULT_OK;

	/* Clean it first. */
	disk_result[0] = '\0';
	disk_result[1] = '\0';

	result = DL_FsVolumeEnum(volumes);
	if (!result) {
		u_strcpy(disk_result, L"Error: Cannot get list of disks!\n");
		return RESULT_FAIL;
	}

	for (i = 0; i < MAX_VOLUMES_COUNT; ++i) {
		disks[(i * 4) + 0] = volumes[i * 3];
		disks[(i * 4) + 1] = volumes[i * 3 + 1];
		disks[(i * 4) + 2] = volumes[i * 3];
		disks[(i * 4) + 3] = 0x0000;
		disk_count += 1;
		if (!volumes[i * 3 + 2]) {
			break;
		}
	}

	for (i = 0; i < disk_count; ++i) {
		char disk_a[LENGTH_VOLUME_NAME];
		WCHAR *disk_u = disks + (i * 4);
		UINT32 free_size_memory;
		UINT32 copy_address;

		u_utoa(disk_u, disk_a);
		if (!DL_FsGetVolumeDescr(disk_u, &volume_description)) {
			u_strcpy(disk_result, L"Error: Cannot get volume description of disk: ");
			u_strcat(disk_result, disk_u);
			u_strcat(disk_result, L"\n");
			return RESULT_FAIL;
		}

#if defined(EP1) || defined(EP2)
		free_size_memory = volume_description.free;
		copy_address = 0x03FC0000; /* iRAM */
#elif defined(EM1) || defined(EM2)
		free_size_memory = volume_description.free_size;
		copy_address = 0x08000000; /* iRAM or RAM */
#else
		free_size_memory = 0;
		copy_address = 0x00000000; /* iROM */
#endif

		LOG("Found volume: %s, free size: %d bytes.\n", disk_a, free_size_memory);
		u_strcat(disk_result, disk_u);
		u_strcat(disk_result, L" (128 KiB file):\n");
		if (free_size_memory < 0x40000) { /* At least 262144 bytes. */
			u_strcat(disk_result, L"No free 262144 bytes.");
		} else {
			DiskBenchmark(disk_result, disk_u, copy_address, 0x1000, 0x20000); /* Chunk: 4096  B, Size: 128 KiB. */
			DiskBenchmark(disk_result, disk_u, copy_address, 0x2000, 0x20000); /* Chunk: 8192  B, Size: 128 KiB. */
			DiskBenchmark(disk_result, disk_u, copy_address, 0x4000, 0x20000); /* Chunk: 16384 B, Size: 128 KiB. */
		}
	}

	if (!disk_count) {
		u_strcpy(disk_result, L"Error: No disks available!\n");
	}

	return status;
}

extern UINT32 DiskBenchmark(WCHAR *disk_result, WCHAR *disk, UINT32 addr, UINT32 c_size, UINT32 f_size) {
	UINT32 i;
	UINT32 rw;
	INT32 res;
	UINT32 status;
	FILE_HANDLE_T file;
	WCHAR file_path[FS_MAX_URI_NAME_LENGTH / 4];
	char *read_buffer;
	UINT64 time_start;
	UINT64 time_end;
	UINT32 time_result;
	WCHAR num_u[RESULT_STRING];

	status = RESULT_OK;

	u_strcpy(file_path, L"file:/");
	u_strcat(file_path, disk);
	u_strcat(file_path, L"BENCH.bin");

	u_ltou(c_size / 1024, num_u);
	u_strcat(disk_result, num_u);
	u_strcat(disk_result, L"K, ");

	status |= DeleteFileIfExists(file_path);

	/* Write File */
	time_start = suPalReadTime();
	file = DL_FsOpenFile(file_path, FILE_WRITE_MODE, NULL);
	for (i = addr; i < addr + f_size; i += c_size) {
		status |= DL_FsWriteFile((void *) i, c_size, 1, file, &rw);
		if (rw == 0) {
			status = RESULT_FAIL;
		}
	}
	status |= DL_FsCloseFile(file);

	time_end = suPalReadTime();
	time_result = (UINT32) suPalTicksToMsec(time_end - time_start);

	u_ltou(time_result, num_u);
	u_strcat(disk_result, L"W:");
	u_strcat(disk_result, num_u);
	u_strcat(disk_result, L", ");

	/* Read File */
	read_buffer = suAllocMem(c_size, &res);
	time_start = suPalReadTime();
	if (res == RESULT_OK) { /* No Allocation Error. */
		file = DL_FsOpenFile(file_path, FILE_READ_MODE, NULL);
		for (i = addr; i < addr + f_size; i += c_size) {
			status = DL_FsReadFile((void *) read_buffer, c_size, 1, file, &rw);
			if (rw == 0) {
				status = RESULT_FAIL;
			}
		}
		status |= DL_FsCloseFile(file);
		time_end = suPalReadTime();
		time_result = (UINT32) suPalTicksToMsec(time_end - time_start);
		suFreeMem(read_buffer);

		u_ltou(time_result, num_u);
		u_strcat(disk_result, L"R:");
		u_strcat(disk_result, num_u);
		u_strcat(disk_result, L", ms.\n");
	} else {
		u_strcat(disk_result, L"Error: Can allocate chunk buffer.\n");
	}

	status |= DeleteFileIfExists(file_path);

	return status;
}

static UINT32 DeleteFileIfExists(const WCHAR *file_path) {
	UINT32 status;

	status = RESULT_FAIL;

	if (DL_FsFFileExist(file_path)) {
		status = DL_FsDeleteFile(file_path, 0);
	}

	return status;
}
