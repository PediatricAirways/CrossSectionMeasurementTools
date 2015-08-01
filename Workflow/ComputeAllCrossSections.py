import os
import subprocess as sub
import sys

from Timeout import *

#############################################################################
def main():
    if len(sys.argv) < 6:
        sys.stdout.write('Usage: %s <executable> <heatflow image> <input geometry file> <output geometry file> <output CSV file>\n' % sys.argv[0])
        sys.exit(-1)

    executable         = sys.argv[1]
    heatflowImagePath  = sys.argv[2]
    inputGeometryPath  = sys.argv[3]
    outputGeometryPath = sys.argv[4]
    outputCSVPath      = sys.argv[5]

    try:
        with time_limit(3600):
            call = [executable, heatflowImagePath, inputGeometryPath, outputGeometryPath, outputCSVPath]
            sub.call(call)
    except Exception as e:
        print "Failed to calculate all cross sections"
        print e

#############################################################################
if __name__ == '__main__':
    main()
