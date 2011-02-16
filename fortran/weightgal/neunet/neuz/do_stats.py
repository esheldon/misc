#!/usr/bin/env python

import os
import sys
import math

OUTLIMIT = 0.4

if len(sys.argv) < 2:
    print 'Usage: ' + sys.argv[0] + ' inptabfile'
    sys.exit(1)
    
infile = open(sys.argv[1], 'r')
lines = infile.readlines()

NR = len(lines)
NC = len(lines[0].strip().split())

sig = 0.0
s68 = 0.0
mean = 0.0
ss = []
NOUTLIER = 0
for i in range(NR):
    line = lines[i].strip().split()
    sig = sig + (float(line[0]) - float(line[1]))*(float(line[0]) - float(line[1]))
    ss.append(math.fabs(float(line[0]) - float(line[1])))
    mean = mean + float(line[0]) - float(line[1])
    if math.fabs(float(line[0]) - float(line[1])) > OUTLIMIT:
	NOUTLIER = NOUTLIER + 1

sig = math.sqrt(sig / NR)
mean = mean / NR

ss.sort()
s68 = ss[(2*NR)/3]

print sys.argv[1]
print 'Mean = ' + str(mean)
print 'Sigma = ' + str(sig)
print 'Sigma_68 = ' + str(s68)
print 'Outlier Fraction = ' + str(float(NOUTLIER) / NR)