#!/usr/bin/env python

"""
Convert color tables, palettes, and sprites from GBA default BGR555 color format to RGB565 default format.
"""

import sys

def rgb_convert(color, rmask, gmask, bmask):
	return \
		((((color >>  0) & 31) * int(rmask / 31)) & rmask) | \
		((((color >>  5) & 31) * int(gmask / 31)) & gmask) | \
		((((color >> 10) & 31) * int(bmask / 31)) & bmask)

def convert(filename, base, patch):
	with open(filename, 'rb') as file_in, open(filename + '_rgb565', 'wb') as file_out:
		while (buff := file_in.read(base)):
			rev_buff = rgb_convert(int.from_bytes(buff, 'little'), 0xF800, 0x07E0, 0x001F).to_bytes(base, 'little')
			file_out.write(rev_buff)
			print(buff.hex(' ').upper() + ' => ' + rev_buff.hex(' ').upper())
	if patch:
		patch_file(filename + '_rgb565')

def patch_file(filename):
	print('\nPatching file!')
	size_sprite_0 = 1430
	size_sprite_ball = 32772
	spr_00_w = (23).to_bytes(2, 'little')
	spr_00_h = (31).to_bytes(2, 'little')
	ball_1_w = (128).to_bytes(2, 'little')
	ball_1_h = (128).to_bytes(2, 'little')

	with open(filename, 'rb+') as file:
		file.seek(size_sprite_0 * 0)
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + spr_00_w.hex(' ').upper())
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + spr_00_h.hex(' ').upper())
		file.seek(size_sprite_0 * 0)
		file.write(spr_00_w)
		file.write(spr_00_h)

		file.seek(size_sprite_0 * 1)
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + spr_00_w.hex(' ').upper())
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + spr_00_h.hex(' ').upper())
		file.seek(size_sprite_0 * 1)
		file.write(spr_00_w)
		file.write(spr_00_h)

		file.seek(size_sprite_0 * 2)
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + spr_00_w.hex(' ').upper())
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + spr_00_h.hex(' ').upper())
		file.seek(size_sprite_0 * 2)
		file.write(spr_00_w)
		file.write(spr_00_h)

		file.seek(size_sprite_0 * 3)
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + spr_00_w.hex(' ').upper())
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + spr_00_h.hex(' ').upper())
		file.seek(size_sprite_0 * 3)
		file.write(spr_00_w)
		file.write(spr_00_h)

		file.seek(size_sprite_0 * 4)
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + ball_1_w.hex(' ').upper())
		print('0x%08X' % file.tell() + ': ' + file.read(2).hex(' ').upper() + ' => ' + ball_1_h.hex(' ').upper())
		file.seek(size_sprite_0 * 4)
		file.write(ball_1_w)
		file.write(ball_1_h)

if __name__ == '__main__':
	if len(sys.argv) == 2:
		convert(sys.argv[1], 2, False)
	elif len(sys.argv) == 3:
		convert(sys.argv[1], 2, True)
	else:
		print('Usage:\n\t./color_convert.py [file] [patch]')
		print('Example:\n\t./color_convert.py dumb.bin')
		print('Example:\n\t./color_convert.py dumb.bin patch')
