#!/usr/bin/env python

import math
import random
import os
import sys

NTIMES = 500000
irange = range(NTIMES)

filename = sys.argv[1]
file = open(filename, 'r')

lines = file.readlines()

N = int(len(lines))

random.seed(13234230)

for i in irange:
    j = int(random.random()*N)
    k = int(random.random()*N)
    templine = lines[j]
    lines[j] = lines[k]
    lines[k] = templine

out = open('GOODS_mixed.train', 'w')
out.writelines(lines)
