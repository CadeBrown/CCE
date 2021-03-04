#!/usr/bin/env python3
""" gentable.py - Generates a table from an equation

Used to generate weight maps for each tile of the chess board

@author: Cade Brown <cade@cade.site>
"""

vals = [[0] * 8 for i in range(8)]
for i in range(8):
    for j in range(8):
        vi = ((i - 3.5) / 3.5) ** 2
        vj = ((j - 3.5) / 3.5) ** 2
        
        score = 1 / (1 + vi + vj)
        vals[i][j] = score


# Output C code

print('static float db_NAME[64] = {')
for i in range(8):
    print('    ', ', '.join(map(lambda x: '{:.2f}'.format(x), vals[i])), ',')
print('};')


