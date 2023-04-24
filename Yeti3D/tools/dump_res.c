/*
 * Tool for creating files from C-precompiled resources.
 * Useful for big-endian routines, see: https://bierbaumer.net/qemu/
 *
 * Compile:
 *   gcc -D__GBA__ -I.. dump_res.c -o dump_res
 *
 * Run:
 *   ./dump_res
 */

#include <stdio.h>
#include <stdlib.h>

#include "data.c"
#include "sprites.c"
#include "e1m1.c"

int main(int argc, char *argv[]) {
	FILE *res_file;

	res_file = fopen("Yeti3D.tex", "wb");
	fwrite(textures, sizeof(texture_t) * YETI_TEXTURE_MAX, 1, res_file);
	fclose(res_file);

	res_file = fopen("Yeti3D.lua", "wb");
	fwrite(lua, sizeof(lua_t), 1, res_file);
	fclose(res_file);

	res_file = fopen("Yeti3D.rec", "wb");
	fwrite(reciprocal, sizeof(int) * YETI_RECIPROCAL_MAX, 1, res_file);
	fclose(res_file);

	res_file = fopen("Yeti3D.sin", "wb");
	fwrite(sintable, sizeof(int) * YETI_SINTABLE_MAX, 1, res_file);
	fclose(res_file);

	res_file = fopen("Yeti3D.map", "wb");
	fwrite(&e1m1, sizeof(rom_map_t), 1, res_file);
	fclose(res_file);

	res_file = fopen("Yeti3D.spr", "wb");
	fwrite(spr_00, sizeof(spr_00), 1, res_file);
	fwrite(spr_01, sizeof(spr_01), 1, res_file);
	fwrite(spr_02, sizeof(spr_02), 1, res_file);
	fwrite(spr_03, sizeof(spr_03), 1, res_file);
	fwrite(spr_ball1, sizeof(spr_ball1), 1, res_file);
	fclose(res_file);

	return 0;
}
