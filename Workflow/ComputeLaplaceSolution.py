import fcsv
import numpy as np
import os
import subprocess as sub
import sys

from Timeout import *

#############################################################################
def ExtractArgs(LandmarksFile):
    markups = fcsv.ReadAllPoints(LandmarksFile)

    landmarks = {}
    neededLandmarks = ['TracheaCarina', 'NoseTip', 'Columella',
                       'RightAlaRim', 'LeftAlaRim', 'NasalSpine']

    # Set up needed landmarks
    for l in markups:
        if (neededLandmarks.count(l[0]) > 0):
            # Warn if a key has already been set
            if (landmarks.has_key(l[0])):
                sys.stderr.write('Landmark "%s" was already set. Using last value\n' % l[0])
            landmarks[l[0]] = [-float(l[2]),-float(l[3]),float(l[4])]

    # Check that needed landmarks were set
    for landmarkName in neededLandmarks:
        if (not landmarks.has_key(landmarkName)):
            sys.stderr.write('Missing landmark "%s"\n' % landmarkName)

    columellaToNoseTip = np.subtract(landmarks['NoseTip'], landmarks['Columella'])
    rightToLeftAlaRim  = np.subtract(landmarks['LeftAlaRim'], landmarks['RightAlaRim'])
    noseVectorHead     = np.cross(columellaToNoseTip, rightToLeftAlaRim)
    noseVectorHead     = np.add(noseVectorHead/np.linalg.norm(noseVectorHead),
                                landmarks['NasalSpine'])
    trachVectorHead    = np.add(landmarks['TracheaCarina'], [0, 0, 1])
    
    args = []
    args.extend([landmarks['NasalSpine'], list(noseVectorHead),
                 landmarks['TracheaCarina'], list(trachVectorHead)])
    return args

#############################################################################
def main():
    if len(sys.argv) < 5:
        sys.stdout.write('Usage: %s <executable> <mouth removed image> <landmarks file> <output image> \n' % sys.argv[0])
        sys.exit(-1)

    executable        = sys.argv[1]
    inputImagePath    = sys.argv[2]
    landmarksFilePath = sys.argv[3]
    outputImagePath   = sys.argv[4]

    try:
        args = ExtractArgs(landmarksFilePath)

        call = []
        call.extend([
            executable,
            "--input", inputImagePath,
            "--output", outputImagePath,
            "--nasalPoint", ','.join([str(x) for x in args[0][:3]]),
            "--nasalVectorHead", ','.join([str(x) for x in args[1][:3]]),
            "--trachealPoint", ','.join([str(x) for x in args[2][:3]]),
            "--tracheaVectorHead", ','.join([str(x) for x in args[3][:3]])
        ])

        with time_limit(3600):
            sub.call( call )

    except Exception, e:
        print e
    
#############################################################################
if __name__ == '__main__':
    main()


