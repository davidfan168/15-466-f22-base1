#!/usr/bin/env python3

from PIL import Image

# process color palette
with open('./Resources/palette.csv') as f:
    contents = f.readlines()

magic = [ord('p'),ord('a'),ord('l'),ord('e')]
output_bytes = bytearray(magic)
output_bytes.append(4 * 14)
output_bytes.append(0)
output_bytes.append(0)
output_bytes.append(0)
for row in contents:
    value = row.split(",")
    output_bytes.append(int(value[1]))
    output_bytes.append(int(value[2]))
    output_bytes.append(int(value[3]))
    output_bytes.append(int(value[4]))

# https://www.geeksforgeeks.org/python-write-bytes-to-file/
with open('./dist/palette', 'wb') as fp:
    fp.write(output_bytes)

# process sprite
for i in range(1, 5):
    bg = Image.open('./Resources/sprite{i}.png'.format(i = i), 'r')
    width, height = bg.size
    if (width != 8 or height != 8):
        print("problem with sprite dimension, it is not 8x8")
        exit()
    magic = [ord('t'),ord('i'),ord('l'),ord('e')]
    output_bytes = bytearray(magic)
    size = 8
    output_bytes.append(8)
    output_bytes.append(0)
    output_bytes.append(0)
    output_bytes.append(0)
    pixel_values = list(bg.getdata())
    for row in range(8):
        bit_value = 0
        for column in range(8):
            bit_value = bit_value << 1
            if (pixel_values[row * 8 + column][0] == 255):
                bit_value = bit_value | 1
        output_bytes.append(bit_value)

    with open('./dist/sprite{i}'.format(i = i), 'wb') as fp:
        fp.write(output_bytes)

print("Preprocessing Complete!")