#!/usr/bin/env python

import sys

def swap_bytes(filename, base):
	with open(filename, 'rb') as file_in, open(filename + '_bigendian', 'wb') as file_out:
		while (buff := file_in.read(base)):
			rev_buff = bytes(reversed(buff))
			file_out.write(rev_buff)
			print(buff.hex(' ').upper() + ' => ' + rev_buff.hex(' ').upper())

if __name__ == '__main__':
	if len(sys.argv) == 3:
		swap_bytes(sys.argv[1], int(sys.argv[2]))
	else:
		print('Usage:\n\t./swap_bytes.py [file] [base]')
		print('Example:\n\t./swap_bytes.py dumb.bin 4')
