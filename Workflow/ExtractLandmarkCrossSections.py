import fcsv
import os
import subprocess as sub
import sys
import tempfile
import traceback

#############################################################################
def ParseLandmarks(landmarksFilePath):
    """Reads landmarks file from Slicer FCSV file and stores them in a
    dictionary keyed by landmark name.
    """

    landmarks = fcsv.ReadAllPoints(landmarksFilePath)
    landmarksDict = {}

    for l in landmarks:
        landmarksDict[l[0]] = [float(l[2]), float(l[3]), float(l[4])]

    # HACK - not all scans have the inferior subglottis marked, so
    # we'll substitue in the subglottic landmark if that is the case.
    if (landmarksDict.has_key('Subglottis') and not landmarksDict.has_key('InferiorSubglottis')):
        landmarksDict['InferiorSubglottis'] = landmarksDict['Subglottis']

    return landmarksDict

#############################################################################
def ExtractCrossSections(landmarks, executable, allCrossSectionsGeometryPath, outputGeometryPath, outputCSVPath):
    """Extract cross-sections that are closest to a set of query points in
    the landmarks dictionary parameter. Saves the extracted cross
    sections to the file path given by the outputGeometryPath.
    """

    call = [executable]
    queryPointNames = [key for key in landmarks]
    for name in queryPointNames:
        landmark = landmarks[name]
        call.append("--queryPoints " + str(landmark[0]) + "," + str(landmark[1]) + "," + str(landmark[2]))

    # Append landmarks names
    call.append('--queryPointNames')
    call.append(','.join(queryPointNames))

    try:
        sub.call( call + [allCrossSectionsGeometryPath, outputGeometryPath, outputCSVPath])
    except Exception as e:
        print "Could not call executable", executable
        print e
        traceback.print_exc(file=sys.stderr)

#############################################################################
def GetSliceIndices(executable, landmarkSegmentsGeometryPath, scanID):
    """Gets the indices of the TVC, InferiorSubglottis, and TracheaCarina
    slices. Saves some intermediate results to a temporary file that
    uses the scan ID so that we can run multiple workflows on the same
    machine in parallel.
    """

    tempFile = '/tmp/Slice%s.txt' % scanID
    sub.call( [executable, landmarkSegmentsGeometryPath, tempFile] )

    l = []
    try:
        f = open(tempFile, 'r')
        lines = list(f)
        l = [ int(lines[0][:-1]), int(lines[1][:-1]) , int(lines[2][:-1]) ]
    except Exception as e:
        print "Could not open temporary file"
        print e
        traceback.print_exc(file=sys.stderr)
    finally:
        sub.call( ['rm', tempFile])
        return l

#############################################################################
def PlaceMinXASlice(indexBounds, sliceFilePath):
    """Returns the center of mass of the mininimum cross-sectional area
    between given contour index bounds.
    """

    sliceFile = open( sliceFilePath, 'r' )
    lines = list( sliceFile )
    header = lines[0].strip().split(',')

    areaIndex = header.index('"area"')
    centerOfMassIndices = [header.index('"center of mass:0"'),
                           header.index('"center of mass:1"'),
                           header.index('"center of mass:2"')]

    minXA = 1000000
    l = ''
    centerOfMass = []
    for i,line in enumerate(lines[ indexBounds[0] - 1:indexBounds[1] - 1 ]):
        columns = line.strip().split(',')
        if float(columns[areaIndex]) < minXA:
            minXA = float(columns[areaIndex])
            centerOfMass = [-float(columns[centerOfMassIndices[0]]),
                            -float(columns[centerOfMassIndices[1]]),
                            float(columns[centerOfMassIndices[2]])]
    return centerOfMass

#############################################################################
def PlaceMidTracheaSlice(indexBounds, sliceFilePath):
    """Returns the center of mass of the cross-section at the MidTrachea,
    which is defined as the midpoint along the center line between
    the TVC and TracheaCarina.  Parameter indexBounds is required to
    be the indices of the TVC and TracheaCarina, respectively."""
    sliceFile = open( sliceFilePath, 'r' )
    lines = list( sliceFile )

    start = indexBounds[0]
    end   = indexBounds[1]

    cLength = [0]
    cumulative = 0

    vals = lines[start].strip().split(',')
    point0 = [ float( vals[0] ), float( vals[1] ), float( vals[2] ) ]
    point1 = [0, 0, 0]

    for i, line in enumerate(lines[start+1:end]):
        vals = line.strip().split(',')
        point1 = [ float( vals[0] ), float( vals[1] ), float( vals[2] ) ]
        length = ( ( point1[0] - point0[0] )**2 + ( point1[1] - point0[1] )**2 + ( point1[2] - point0[2] )**2 )**.5
        cumulative = cumulative + length
        cLength.extend([cumulative])
        point0 = point1

    midLength = cLength[-1]/2.0

    i = 0
    while cLength[i] < midLength:
        i = i + 1

    index = i - 1
    if abs(cLength[i] - midLength) < abs(cLength[i-1] - midLength):
        index = i

    columns = lines[start + index].strip().split(',')
    centerOfMass = [ -float(columns[0]), -float(columns[1]), float(columns[2]) ]
    return centerOfMass

#############################################################################
def main():
    if len(sys.argv) < 9:
        sys.stdout.write('Usage: %s <extract cross-section executable> <landmarks indices extractor executable> <all cross-sections geometry file> <all cross-sections CSV file> <landmarks file> <landmark cross-section geometry file> <landmark CSV file> <scanID>\n' % sys.argv[0])
        sys.exit(-1)

    extractCrossSectionsExe      = sys.argv[1] # ExtractCrossSections
    extractLandmarkIndicesExe    = sys.argv[2] # ExtractLandmarkSliceIndices
    allCrossSectionsGeometryPath = sys.argv[3] # ????_ALL_CROSS_SECTIONS.vtp
    allCrossSectionsCSVPath      = sys.argv[4] # ????_ALL_CROSS_SECTIONS.csv
    landmarksFCSVPath            = sys.argv[5] # ????_LANDMARKS.fcsv
    landmarksGeometryPath        = sys.argv[6] # ????_CROSS_SECTIONS_AT_LANDMARKS.vtp
    landmarksCSVPath             = sys.argv[7] # ????_CROSS_SECTIONS_AT_LANDMARKS.csv
    scanID                       = sys.argv[8] # ????

    # Set up landmarks
    try:
        landmarks = ParseLandmarks( landmarksFCSVPath )

        # Check to see if some landmarks are missing
        landmarksToCheck = ['InferiorSubglottis', 'Subglottis', 'TVC', 'TracheaCarina']
        for landmarkName in landmarksToCheck:
            if (not landmarks.has_key(landmarkName)):
                sys.stderr.write('%s not set\n' % landmarkName)

    except Exception as e:
        print "Could not extract args"
        print e
        traceback.print_exc(file=sys.stderr)

    # Extract requested cross sections
    try:
        ExtractCrossSections( landmarks, extractCrossSectionsExe,
                              allCrossSectionsGeometryPath, landmarksGeometryPath, landmarksCSVPath )
    except Exception as e:
        print "Could not extract cross sections"
        print e
        traceback.print_exc(file=sys.stderr)

    try:
        sliceIndices = GetSliceIndices( extractLandmarkIndicesExe, landmarksGeometryPath, scanID )
    except Exception as e:
        print "Could not get slice indices"
        print e
        traceback.print_exc(file=sys.stderr)

    try:
        if len(sliceIndices) > 0:
            indexBounds = sliceIndices[0:2]
            if indexBounds.count(-1) == 0:
                landmarks['MinimumXABetweenTVCandMidTrachea'] = PlaceMinXASlice( indexBounds, allCrossSectionsCSVPath )
            indexBounds = [sliceIndices[0], sliceIndices[2]]
            if indexBounds.count(-1) == 0:
                landmarks['MidTrachea'] = PlaceMidTracheaSlice( indexBounds, allCrossSectionsCSVPath )

            #ExtractCrossSections( landmarks, extractCrossSectionsExe,
            #                      allCrossSectionsGeometryPath, landmarksGeometryPath, landmarksCSVPath )

    except Exception as e:
        print "Could not extract data"
        print e
        traceback.print_exc(file=sys.stderr)

#############################################################################
#############################################################################
if __name__ == '__main__':
    main()
