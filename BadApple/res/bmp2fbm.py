#!/usr/bin/env python

"""
Generate frame bitmaps map *.fbm 1bpp file from *.bmp.

python -m pip install --upgrade pip
python -m pip install --upgrade Pillow
"""

import os
import sys
import glob
import zlib

import PIL
from PIL import Image

bmp_offset = 0x3E
bmp_width = 176
bmp_height = 220
out = 'BadApple.fbm'
directory = 'fipped_bitmaps'

def convert_bmp_to_fbm(bitmaps_directory):
	max_compressed_size = 0
	with open(out, 'wb') as file_out:
		bitmap_names = glob.glob(bitmaps_directory + '/*.bmp')
		bitmap_names.sort()
		os.makedirs(directory, exist_ok=True)
		first_time = True
		idx = 0
		for bitmap in bitmap_names:
			bmp = Image.open(bitmap).transpose(PIL.Image.FLIP_TOP_BOTTOM)

			if first_time:
				file_out.write(bmp_width.to_bytes(2, byteorder='big'))
				file_out.write(bmp_height.to_bytes(2, byteorder='big'))
				file_out.write(len(bitmap_names).to_bytes(2, byteorder='big'))
				file_out.write((0).to_bytes(2, byteorder='big')) # max_compressed_size
				file_out.write((1).to_bytes(2, byteorder='big')) # 1bpp

			flip_bmp = bmp.resize((bmp_width, bmp_height))
			flip_bmp = flip_bmp.convert('1')
			flip_bmp.save(directory + '/' + os.path.basename(bitmap))

			with open(directory + '/' + os.path.basename(bitmap), 'rb') as file_in:
				file_in.seek(bmp_offset)
				buff = file_in.read()
				if first_time:
					file_out.write((len(buff)).to_bytes(2, byteorder='big'))
					first_time = False
				compressed = zlib.compress(buff, level=-1, wbits=-zlib.MAX_WBITS)
				print('Converting [' + str(idx+1) + '/' + str(len(bitmap_names)) + '] '
					+ bitmap + ': ' + str(len(buff)) + ' => ' + str(len(compressed))
					+ ' bytes (compressed)')
				if max_compressed_size < len(compressed):
					max_compressed_size = len(compressed)
				file_out.write(len(compressed).to_bytes(4, byteorder='big'))
				file_out.write(compressed)
			idx += 1
		print('max_compressed_size=' + str(max_compressed_size))
		file_out.seek(2*3, 0)
		file_out.write(max_compressed_size.to_bytes(2, byteorder='big'))

if __name__ == '__main__':
	if len(sys.argv) == 2:
		convert_bmp_to_fbm(sys.argv[1])
	else:
		print('Usage:\n\t./bmp2fbm.py [bitmaps_directory]')
		print('Example:\n\t./bmp2fbm.py black_and_white_bitmaps/')
