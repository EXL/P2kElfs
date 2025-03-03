#!/usr/bin/env python

"""
Generate frame bitmaps map *.fbm 1bpp file from *.bmp frames.

python -m pip install --upgrade pip
python -m pip install --upgrade Pillow
"""

import os
import sys
import glob
import zlib

import PIL
from PIL import Image

compress = True
endinan = 'big'
#endinan = 'little'
bmp_offset = 0x3E
bmp_width = 176
bmp_height = 220
nvidia_goforce = False
out = 'BadApple.fbm'
directory = 'fipped_bitmaps'

# Thanks to Chat-GPT4o and GitHub Copilot.
def extract_number(filename):
	return int(filename.split('_')[1].split('.')[0])

def nvidia_swap_8xY_cols(bitmap, w, h):
	width = 192
	height = 220
	if w == 240:
		width = 256
		height = h

	if len(bitmap) != (width * height // 8):
		raise ValueError(f'Bitmap must be {width}x{height} in size!')

	# Convert bytes to bytearray for modification.
	bitmap = bytearray(bitmap)

	def get_byte_index(x, y):
		return y * (width // 8) + (x // 8)

	def get_bit(byte, bit_index):
		return (byte >> (7 - bit_index)) & 1

	def set_bit(byte, bit_index, value):
		if value:
			return byte | (1 << (7 - bit_index))
		else:
			return byte & ~(1 << (7 - bit_index))

	for col_block in range(0, width, 16):
		for col_offset in range(8):
			for row in range(height):
				byte1_index = get_byte_index(col_block + col_offset, row)
				byte2_index = get_byte_index(col_block + col_offset + 8, row)

				bit1_index = (col_block + col_offset) % 8
				bit2_index = (col_block + col_offset + 8) % 8

				byte1 = bitmap[byte1_index]
				byte2 = bitmap[byte2_index]

				bit1 = get_bit(byte1, bit1_index)
				bit2 = get_bit(byte2, bit2_index)

				byte1 = set_bit(byte1, bit1_index, bit2)
				byte2 = set_bit(byte2, bit2_index, bit1)

				bitmap[byte1_index] = byte1
				bitmap[byte2_index] = byte2

	# Convert back to bytes before returning.
	return bytes(bitmap)

def convert_bmp_to_fbm(bitmaps_directory):
	max_compressed_size = 0
	with open(out, 'wb') as file_out:
		bitmap_names = glob.glob(bitmaps_directory + '/*.bmp')
		bitmap_names.sort(key=extract_number)
		os.makedirs(directory, exist_ok=True)
		first_time = True
		idx = 0
		for bitmap in bitmap_names:
			bmp = Image.open(bitmap).transpose(PIL.Image.FLIP_TOP_BOTTOM)

			if first_time:
				file_out.write(bmp_width.to_bytes(2, byteorder=endinan))
				file_out.write(bmp_height.to_bytes(2, byteorder=endinan))
				file_out.write(len(bitmap_names).to_bytes(2, byteorder=endinan))
				file_out.write((0).to_bytes(2, byteorder=endinan)) # max_compressed_size
				file_out.write((1).to_bytes(2, byteorder=endinan)) # 1bpp

			flip_bmp = bmp.resize((bmp_width, bmp_height))
			flip_bmp = flip_bmp.convert('1')
			flip_bmp.save(directory + '/' + os.path.basename(bitmap))

			with open(directory + '/' + os.path.basename(bitmap), 'rb') as file_in:
				file_in.seek(bmp_offset)
				buff = file_in.read()
				if nvidia_goforce:
					buff = nvidia_swap_8xY_cols(buff, bmp_width, bmp_height)
				if first_time:
					file_out.write((len(buff)).to_bytes(2, byteorder=endinan))
					first_time = False
				if compress:
					# Available only in Python 3.11
					# compressed = zlib.compress(buff, level=-1, wbits=-zlib.MAX_WBITS)
					compressor = zlib.compressobj(level=-1, wbits=-zlib.MAX_WBITS)
					compressed = compressor.compress(buff)
					compressed += compressor.flush()
				else:
					compressed = buff
				print('Converting [' + str(idx+1) + '/' + str(len(bitmap_names)) + '] '
					+ bitmap + ': ' + str(len(buff)) + ' => ' + str(len(compressed))
					+ ' bytes (compressed)')
				if max_compressed_size < len(compressed):
					max_compressed_size = len(compressed)
				if compress:
					file_out.write(len(compressed).to_bytes(4, byteorder=endinan))
				file_out.write(compressed)
			idx += 1
		print('max_compressed_size=' + str(max_compressed_size))
		file_out.seek(2*3, 0)
		file_out.write(max_compressed_size.to_bytes(2, byteorder=endinan))

if __name__ == '__main__':
	if len(sys.argv) == 2:
		convert_bmp_to_fbm(sys.argv[1])
	else:
		print('Usage:\n\t./bmp2fbm.py [bitmaps_directory]')
		print('Example:\n\t./bmp2fbm.py black_and_white_bitmaps/')
