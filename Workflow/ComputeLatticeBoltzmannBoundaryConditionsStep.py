import fcsv
import subprocess
import sys

#############################################################################
def main():
    if (len(sys.argv) < 6):
        sys.stdout.write('Usage: %s <executable> <input file> <resampled segmentation> <threshold file> <nose sphere file> <outflow cutoff position> <LBM image> ' % sys.argv[0])
        sys.exit(-1)

    executable                    = sys.argv[1]
    inputFilePath                 = sys.argv[2]
    resampledSegmentationFilePath = sys.argv[3]
    thresholdFilePath             = sys.argv[4]
    noseSphereFilePath            = sys.argv[5]
    outflowCutoffPositionFile     = sys.argv[6]
    outputFilePath                = sys.argv[7]

    # Read threshold from file
    with open(thresholdFilePath, 'r') as f:
        value = f.read().strip()
        print 'Threshold:', value
        thresholdValue = value

    # Read nose sphere center from file
    markups = fcsv.ReadAllPoints(noseSphereFilePath)
    if (len(markups) < 1):
        sys.stderr.write('No nose sphere center in %s' % noseSphereFilePath)
        return

    noseSphereCenter = str(-float(markups[0][2])) + ',' + str(-float(markups[0][3])) + ',' + markups[0][4]
    print 'NoseSphereCenter:', noseSphereCenter

    # Read outflow cutoff position
    markups = fcsv.ReadAllPoints(outflowCutoffPositionFile)
    if (len(markups) < 1):
        sys.stderr.write('No cutoff position in %s' % outflowCutoffPositionFile)
        return

    outflowCutoffPosition = str(-float(markups[0][2])) + ',' + str(-float(markups[0][3])) + ',' + markups[0][4]
    print 'Outflow cutoff position:', outflowCutoffPosition

    # Fix nose sphere radius at 18mm
    noseSphereRadius = str(18.0)

    try:
        call = [executable, '--segmentationThreshold', thresholdValue,
                '--noseSphereCenter', noseSphereCenter,
                '--noseSphereRadius', noseSphereRadius,
                '--outflowCutoffPosition', outflowCutoffPosition,
                inputFilePath, resampledSegmentationFilePath, outputFilePath]

        subprocess.call(call)
    except Exception, e:
        print e

#############################################################################
if __name__ == '__main__':
    main()
