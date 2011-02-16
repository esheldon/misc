#!/usr/bin/env python
import sys

#ind  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20
#%   0   5   10  15  20  25  30  35  40  45  50  55  60  65  70  75  80  85  90  95  100
let=['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u']
def perc2pieletter(perc):
    # this is the map between percentage and letters
    # a (1) is at 0%
    # k (11) is at 50%
    # u (21) is at 100%

    index = int( round( perc*100./5.) )
    return let[index]

numer=float(sys.argv[1])
denom=float(sys.argv[2])

perc=numer/denom

sys.stdout.write("%s" % perc2pieletter(perc) )
