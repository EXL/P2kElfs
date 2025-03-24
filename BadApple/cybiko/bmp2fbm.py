#!/usr/bin/env python

"""
Generate frame bitmaps map *.fbm 1bpp file from *.bmp frames.

python -m pip install --upgrade pip
python -m pip install --upgrade Pillow
"""

import os
import sys
import glob

import PIL
from PIL import Image

endinan = 'big'
#endinan = 'little'
bmp_width = 160
bmp_height = 100
out = 'BadApple.fbm'
directory = 'flipped_bitmaps'

# Thanks to Chat-GPT4o and GitHub Copilot.
def convert_grayscale_to_2bpp(image_path):
	# Open the image
	img = Image.open(image_path)

	# Ensure the image is the correct size
	if img.size != (bmp_width, bmp_height):
		raise ValueError(f"Image must be {bmp_width}x{bmp_height} pixels")

	# Get the raw pixel data
	pixel_data = list(img.getdata())

	# Initialize the output buffer
	output = bytearray((bmp_width * bmp_height * 2) // 8)

	output_index = 0

	for i in range(0, bmp_width * bmp_height, 4):
		byte = 0
		for bit in range(4):
			if i + bit < len(pixel_data):
				pixel_value = pixel_data[i + bit]
				# Convert grayscale to 2bpp
				if pixel_value < 64:
					bpp2_pixel = 0x03  # Black
				elif pixel_value < 128:
					bpp2_pixel = 0x02  # Dark Gray
				elif pixel_value < 192:
					bpp2_pixel = 0x01  # Light Gray
				else:
					bpp2_pixel = 0x00  # White

				byte |= (bpp2_pixel << (6 - (bit * 2)))
		output[output_index] = byte
		output_index += 1

	return output

# Thanks to Chat-GPT4o and GitHub Copilot.
def extract_number(filename):
	return int(filename.split('_')[1].split('.')[0])

def convert_bmp_to_fbm(bitmaps_directory):
	max_compressed_size = 0
	with open(out, 'wb') as file_out:
		bitmap_names = glob.glob(bitmaps_directory + '/*.png')
		bitmap_names.sort(key=extract_number)
		os.makedirs(directory, exist_ok=True)
		first_time = True
		idx = 0
		for bitmap in bitmap_names:
			bmp = Image.open(bitmap)
#			.transpose(PIL.Image.FLIP_TOP_BOTTOM)

			if first_time:
				file_out.write(bmp_width.to_bytes(2, byteorder=endinan))
				file_out.write(bmp_height.to_bytes(2, byteorder=endinan))
				file_out.write(len(bitmap_names).to_bytes(2, byteorder=endinan))
				file_out.write((0).to_bytes(2, byteorder=endinan)) # max_compressed_size
				file_out.write((2).to_bytes(2, byteorder=endinan)) # 1bpp

			flip_bmp = bmp.resize((bmp_width, bmp_height))
			flip_bmp = flip_bmp.convert('L')
			flip_bmp.save(directory + '/' + os.path.basename(bitmap))

			buff = convert_grayscale_to_2bpp(directory + '/' + os.path.basename(bitmap))
			if first_time:
				file_out.write((len(buff)).to_bytes(2, byteorder=endinan))
				first_time = False
			compressed = buff
			print('Converting [' + str(idx+1) + '/' + str(len(bitmap_names)) + '] '
				+ bitmap + ': ' + str(len(buff)) + ' => ' + str(len(compressed))
				+ ' bytes (compressed)')
			if max_compressed_size < len(compressed):
				max_compressed_size = len(compressed)
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
