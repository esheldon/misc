#!/usr/bin/env python

import os
import sys
from Numeric import *

if len(sys.argv) < 4:
    print 'Usage: ' + sys.argv[0] + ' param1 param2 wtsfile'
    sys.exit(10)

infile1 = open(sys.argv[1], 'r')
infile2 = open(sys.argv[2], 'r')

lines1 = infile1.readlines()
lines2 = infile2.readlines()

NLAYER_1 = 0
NLAYER_2 = 0

NFILTER_1 = 0
NFILTER_2 = 0

NLH1_1 = 0
NLH1_2 = 0

NLH2_1 = 0
NLH2_2 = 0

NLH3_1 = 0
NLH3_2 = 0

NLH4_1 = 0
NLH4_2 = 0

for line in lines1:
    l = line.strip().split()
    if len(l) == 3:
        if l[0] == 'NLAYER':
            NLAYER_1 = int(l[2])
        if l[0] == 'NFILTER':
            NFILTER_1 = int(l[2])
        if l[0] == 'NLH1':
            NLH1_1 = int(l[2])
        if l[0] == 'NLH2':
            NLH2_1 = int(l[2])
        if l[0] == 'NLH3':
            NLH3_1 = int(l[2])
        if l[0] == 'NLH4':
            NLH4_1 = int(l[2])

NL = 0
MN = -10
for line in lines2:
    l = line.strip().split()
    if len(l) == 3:
        if l[0] == 'NLAYER':
            NLAYER_2 = int(l[2])
        if l[0] == 'NFILTER':
            NFILTER_2 = int(l[2])
        if l[0] == 'NLH1':
            NLH1_2 = int(l[2])
            if NLH1_2 > MN:
                MN = NLH1_2
            NL = NL + 1
        if l[0] == 'NLH2':
            NLH2_2 = int(l[2])
            if NLH2_2 > MN:
                MN = NLH2_2
            NL = NL + 1
        if l[0] == 'NLH3':
            NLH3_2 = int(l[2])
            if NLH3_2 > MN:
                MN = NLH3_2
            NL = NL + 1
        if l[0] == 'NLH4':
            NLH4_2 = int(l[2])
            if NLH4_2 > MN:
                MN = NLH4_2
            NL = NL + 1

print 'NFILTER_1 = ' + str(NFILTER_1)
print 'NFILTER_2 = ' + str(NFILTER_2)

NL1 = NLAYER_1 - 2
NL2 = NLAYER_2 - 2

print 'MN = ' + str(MN)
print 'NL1 = ' + str(NL1) #NL is the number of hidden layers
print 'NL2 = ' + str(NL2) #NL is the number of hidden layers

nw1 = zeros(NLAYER_1, Int32)
nw1[0] = NFILTER_1
if NLH1_1 > 0:
    nw1[1] = NLH1_1
if NLH1_2 > 0:
    nw1[2] = NLH2_1
if NLH1_2 > 0:
    nw1[3] = NLH3_1
if NLH1_2 > 0:
    nw1[4] = NLH4_1
nw1[NLAYER_1-1] = 1

nw2 = zeros(NLAYER_2, Int32)
nw2[0] = NFILTER_2
if NLH1_2 > 0:
    nw2[1] = NLH1_2
if NLH1_2 > 0:
    nw2[2] = NLH2_2
if NLH1_2 > 0:
    nw2[3] = NLH3_2
if NLH1_2 > 0:
    nw2[4] = NLH4_2
nw2[NLAYER_2-1] = 1

w = zeros([NLAYER_2-1, MN, MN], Float32)

print nw1
print nw2

wfile = open(sys.argv[3], 'r')
lines = wfile.readlines()
for i in range(NLAYER_1-1):
    l = lines[i+2].strip().split()
    c = 0
    for j in range(nw1[i]):
        for k in range(nw1[i+1]):
            w[i][j][k] = float(l[c])
            c = c+1

for i in range(NLAYER_2-1):
    for j in range(nw2[i]):
        for k in range(nw2[i+1]):
            print str(i) + ' ' + str(j) + ' ' + str(k) + ' ' + str(w[i][j][k])

outfile = open('out.wts', 'w')
outfile.write(str(NLAYER_2) + '\n')
for i in range(NLAYER_2):
    outfile.write(str(nw2[i]) + ' ')
outfile.write('\n')

for i in range(NLAYER_2-1):
    for j in range(nw2[i]):
        for k in range(nw2[i+1]):
            outfile.write(str(w[i][j][k]) + ' ')
    outfile.write('\n')

outfile.close()

