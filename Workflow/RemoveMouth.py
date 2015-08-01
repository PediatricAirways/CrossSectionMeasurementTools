import os
import subprocess
import sys

#############################################################################
def main():
    if (len(sys.argv) < 7):
        sys.stdout.write('Usage: %s <executable> <segmentation image> <segmentation geometry> <clippings file> <output image> <output geometry>\n' % sys.argv[0])
        sys.exit(-1)

    executable               = sys.argv[1]
    segmentationImagePath    = sys.argv[2]
    segmentationGeometryPath = sys.argv[3]
    clippingsFilePath        = sys.argv[4]
    outputImagePath          = sys.argv[5]
    outputGeometryPath       = sys.argv[6]

    centers = []
    radii   = []

    try:
        clippingsFile = open(clippingsFilePath, 'r')

        for i, line in enumerate(clippingsFile):
            l = line.split(" ")
            if (i % 2 == 0):
                centers.append([-float(l[2]), -float(l[3]), float(l[4])])
            else:
                radii.extend([float(l[2])])
    except:
        # No clippings file available. Copy the input to the output and terminate.
        import shutil
        shutil.copy2(segmentationImagePath, outputImagePath)
        shutil.copy2(segmentationGeometryPath, outputGeometryPath)
        sys.exit(0)

    for i in xrange(len(centers)):
        if (i == 0):
            tmpImageInput = segmentationImagePath
            tmpGeometryInput = segmentationGeometryPath
        else:
            tmpImageInput = outputImagePath
            tmpGeometryInput = outputGeometryPath
            
        input 	 = "--input " + tmpImageInput
        inputGeometry = "--inputGeometry " + tmpGeometryInput
        output   = "--output " + outputImagePath
        outputGeometry = "--outputGeometry " + outputGeometryPath
        Center   = "--Center " + str(centers[i][0]) + "," + str(centers[i][1]) + "," + str(centers[i][2])
        Radius	 = "--Radius " + str(radii[i])
        #call 	 = executable + ' ' + input + ' ' + output + ' ' + Center + ' ' + Radius
        #print call
        try:
            call = [executable, input, inputGeometry, output, outputGeometry, Center, Radius]
            print ' '.join(call)
            subprocess.call( call )
        except Exception, e:
            print e
    
#############################################################################
if __name__ == '__main__':
    main()
