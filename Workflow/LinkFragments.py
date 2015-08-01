import fcsv
import os
import subprocess
import sys

#############################################################################
def main():
    if (len(sys.argv) < 5):
        sys.stderr.write('Usage: %s <executable> <links file> <input file> <output file>\n' % sys.argv[0])
        sys.exit(-1)

    executable     = sys.argv[1]
    linksFilePath  = sys.argv[2]
    inputFilePath  = sys.argv[3]
    outputFilePath = sys.argv[4]

    # Read links file
    try:
        markups = fcsv.ReadAllPoints(linksFilePath)
    except Exception as e:
        markups = []

    call = []
    call.extend([
        executable,
        '--value', '-1024',
        '--radius', '1.5'])

    # Add markups
    for markup in markups:
        call.extend([
            '--endpoint', markup[2] + ',' + markup[3] + ',' + markup[4]])

    call.extend([
        inputFilePath,
        outputFilePath])

    try:
        subprocess.call( call )
    except Exception, e:
        print e

#############################################################################
if __name__ == '__main__':
    main()
