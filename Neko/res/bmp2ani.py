#!/usr/bin/env python

"""
Converter BMP to ANI and ANI to BMP using Python 3 and PIL library.

python -m pip install --upgrade pip
python -m pip install --upgrade Pillow

ANI header format:
	uint8_t pallete_colors    : count of pallete colors
	uint24_t * pallete_colors : pallete in rgb (uint8_t) colors
	uint8_t frame_width       : bitmap_width / 2
	uint8_t frame_height      : bitmap_width
	uint8_t two_pixels_array  : bitmap data in 4bpp pixel format
"""

import os
import sys

import PIL
from PIL import Image

def print_help():
	print('Usage:\n\t./bmp2ani.py [filename].[ani|bmp]')
	print('Example:\n\t./bmp2ani.py sheep.ani')
	print('Example:\n\t./bmp2ani.py sheep.bmp')

def convert_bmp_palette_to_ani_palette(bmp_palette):
	ani_palette = bytearray()
	rgb = bytearray()
	for byte in bmp_palette:
		rgb.append(byte)
		if len(rgb) == 3:
			for c in rgb:
				ani_palette.append(c)
			rgb.clear()
	return ani_palette

def pack_8bpp_to_4bpp(raw_bytes):
	byte_array = bytearray()
	pack = bytearray()
	for byte in raw_bytes:
		pack.append(byte)
		if len(pack) == 2:
			unpacked_byte_1 = pack[0]
			unpacked_byte_2 = pack[1]
			packed_byte = (unpacked_byte_1 << 4 | unpacked_byte_2)
			# print('0b{:08b}, 0b{:08b} => 0b{:08b}'.format(unpacked_byte_1, unpacked_byte_2, packed_byte))
			byte_array.append(packed_byte)
			pack.clear()
	return byte_array

def unpack_4bpp_to_8bpp(raw_bytes):
	byte_array = bytearray()
	for byte in raw_bytes:
		unpacked_byte_1 = byte >> 4;
		unpacked_byte_2 = byte & 0x0F;
		# print('0b{:08b} => 0b{:08b}, 0b{:08b}'.format(byte, unpacked_byte_1, unpacked_byte_2))
		byte_array.append(unpacked_byte_1)
		byte_array.append(unpacked_byte_2)
	return byte_array

def convert_ani_to_bmp(filename):
	with open(filename, 'rb') as file_in:
		ani_file_size = os.path.getsize(filename)
		ani_pal_size = int.from_bytes(file_in.read(1), 'big')
		ani_pal = file_in.read(ani_pal_size * 3)
		ani_frame_w = int.from_bytes(file_in.read(1), 'big')
		ani_frame_h = int.from_bytes(file_in.read(1), 'big')
		ani_bitmap_size = ani_file_size - (ani_pal_size * 3) - 1 - 1 - 1
		ani_frame_count = int(ani_bitmap_size / (ani_frame_w * ani_frame_h))
		ani_bitmap_w = ani_frame_w * 2
		ani_bitmap_h = ani_frame_h * ani_frame_count

		print('ani_file_size = ' + str(ani_file_size))
		print('ani_pal_size = ' + str(ani_pal_size))
		print('ani_frame_w = ' + str(ani_frame_w))
		print('ani_frame_h = ' + str(ani_frame_h))
		print('ani_bitmap_size = ' + str(ani_bitmap_size))
		print('ani_frame_count = ' + str(ani_frame_count))
		print('ani_bitmap_w = ' + str(ani_frame_count))
		print('ani_bitmap_h = ' + str(ani_frame_count))

		bitmap = Image.new('P', (ani_bitmap_w, ani_bitmap_h))
		bitmap.putpalette(ani_pal)
		bitmap.putdata(unpack_4bpp_to_8bpp(file_in.read(ani_bitmap_size)))
		bitmap.save(filename.replace('.ani', '.bmp'))

def convert_bmp_to_ani(filename):
	bmp = Image.open(filename)
	bmp_bitmap_w = bmp.width
	bmp_bitmap_h = bmp.height
	bmp_bitmap_mode = bmp.mode
	bmp_palette = bmp.getpalette()
	bmp_palette_size = int(len(bmp_palette) / 3)
	bmp_data = bmp.getdata()
	bmp_data_size = len(bmp.getdata())

	print('bmp_bitmap_w = ' + str(bmp_bitmap_w))
	print('bmp_bitmap_h = ' + str(bmp_bitmap_h))
	print('bmp_bitmap_mode = ' + str(bmp_bitmap_mode))
	print('bmp_palette_size = ' + str(bmp_palette_size))
	print('bmp_data_size = ' + str(bmp_data_size))

	ani_pal = convert_bmp_palette_to_ani_palette(bmp_palette)
	ani_pal_size = int(len(ani_pal) / 3)
	ani_bitmap_w = int(bmp_bitmap_w / 2)
	ani_bitmap_h = bmp_bitmap_w
	ani_data = pack_8bpp_to_4bpp(bmp_data)
	ani_data_size = len(ani_data)

	print('ani_pal_size = ' + str(ani_pal_size))
	print('ani_bitmap_w = ' + str(ani_bitmap_w))
	print('ani_bitmap_h = ' + str(ani_bitmap_h))
	print('ani_data_size = ' + str(ani_data_size))

	with open(filename.replace('.bmp', '.ani'), 'wb') as file_out:
		file_out.write(ani_pal_size.to_bytes(1, 'big'))
		file_out.write(ani_pal)
		file_out.write(ani_bitmap_w.to_bytes(1, 'big'))
		file_out.write(ani_bitmap_h.to_bytes(1, 'big'))
		file_out.write(ani_data)

if __name__ == '__main__':
	if len(sys.argv) == 2:
		if sys.argv[1].endswith('.ani'):
			convert_ani_to_bmp(sys.argv[1])
		elif sys.argv[1].endswith('.bmp'):
			convert_bmp_to_ani(sys.argv[1])
		else:
			print_help()
	else:
		print_help()
