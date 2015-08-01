import fcsv
import mrml
import os
import subprocess
import sys
import tokenize

#############################################################################
def main():
    if (len(sys.argv) < 7):
        sys.stderr.write('Usage: %s <executable> <MRML file> <input image file> <output image file> <output geometry file> <output threshold file>\n' % sys.argv[0])
        sys.exit(-1)

    executable              = sys.argv[1]
    lowerSeedFilePath       = sys.argv[2]
    upperSeedFilePath       = sys.argv[3]
    mrmlFilePath            = sys.argv[4]
    inputImageFilePath      = sys.argv[5]
    outputImageFilePath     = sys.argv[6]
    outputGeometryFilePath  = sys.argv[7]
    outputThresholdFilePath = sys.argv[8]

    # Read lower seed position
    markups = fcsv.ReadAllPoints(lowerSeedFilePath)
    if (len(markups) < 1):
        sys.stderr.write('No lower seed point in %s' % lowerSeedFilePath)
        return

    lowerSeedPoint = markups[0][2] + ',' + markups[0][3] + ',' + markups[0][4]

    # Read upper seed position
    markups = fcsv.ReadAllPoints(upperSeedFilePath)
    if (len(markups) < 1):
        sys.stderr.write('No upper seed point in %s' % upperSeedFilePath)
        return

    upperSeedPoint = markups[0][2] + ',' + markups[0][3] + ',' + markups[0][4]

    args = mrml.ParseCommandLineModule(mrmlFilePath)

    # See if we need to use airway fragments
    fragmentSeedStrings = []
    if (args['bAddAirwayFragments'] == "true"):
        # Assume fragments file is in the same path as the MRML file
        fragmentsFileDir = os.path.dirname(mrmlFilePath)

        # Get scan ID from last four characters in file path
        scanId = fragmentsFileDir[-4:]
        fragmentsFilePath = os.path.join(fragmentsFileDir, scanId + '_FRAGMENTS.fcsv')
        markups = fcsv.ReadAllPoints(fragmentsFilePath)
        fragmentSeedStrings = [','.join([tup[2],tup[3],tup[4]]) for tup in markups]

    try:
        call = []
        call.extend([
            executable,
            '--createOutputGeometry',
            '--upperSeedRadius', args['upperSeedRadius'],
            '--lowerSeedRadius', args['lowerSeedRadius'],
            '--upperSeed', upperSeedPoint,
            '--lowerSeed', lowerSeedPoint,
            '--threshold',
            '--thresholdFile', outputThresholdFilePath,
            '--maxAirwayRadius', args['dMaxAirwayRadius'],
            '--erodeDistance', args['dErodeDistance']])

        if (len(fragmentSeedStrings) > 0):
            call.append('--addAirwayFragments')
            for seed in fragmentSeedStrings:
                call.append('--airwayFragmentSeed')
                call.append(seed)

        if (args['bRemoveBreathingMask'] == "true"):
            call.extend(['--removeBreathingMask',
                         '--breathingMaskThickness',
                         args['dBreathingMaskThickness']])

        call.extend([
            inputImageFilePath,
            outputImageFilePath,
            outputGeometryFilePath,
            "None"])

        subprocess.call( call )
    except Exception as e:
        print e

#############################################################################
#############################################################################
if __name__ == '__main__':
    main()
