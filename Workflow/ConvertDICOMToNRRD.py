import os
import subprocess
import sys

def main():
    if (len(sys.argv) < 4):
        sys.stdout.write('Usage: %s <executable> <Dicom Directory> <Output Image Path>\n' % sys.argv[0])
        sys.exit(-1)

    executable     = sys.argv[1]
    dicomDirectory = sys.argv[2]
    outputFilePath = sys.argv[3]

    try:
        call = [executable, dicomDirectory, outputFilePath]
        subprocess.call( call )
    except Exception, e:
        print e

#############################################################################
if __name__ == '__main__':
    main()
