#!/usr/bin/env python

"""
Convert color tables and palettes from GBA default BGR555 color format to RGB565 default format.
"""

import sys

def rgb_convert(color, rmask, gmask, bmask):
	return \
		((((color >>  0) & 31) * int(rmask / 31)) & rmask) | \
		((((color >>  5) & 31) * int(gmask / 31)) & gmask) | \
		((((color >> 10) & 31) * int(bmask / 31)) & bmask)

def convert(filename, base):
	with open(filename, 'rb') as file_in, open(filename + '_rgb565', 'wb') as file_out:
		while (buff := file_in.read(base)):
			rev_buff = rgb_convert(int.from_bytes(buff, 'little'), 0xF800, 0x07E0, 0x001F).to_bytes(base, 'little')
			file_out.write(rev_buff)
			print(buff.hex(' ').upper() + ' => ' + rev_buff.hex(' ').upper())

if __name__ == '__main__':
	if len(sys.argv) == 2:
		convert(sys.argv[1], 2)
	else:
		print('Usage:\n\t./color_convert.py [file]')
		print('Example:\n\t./color_convert.py dumb.bin')
