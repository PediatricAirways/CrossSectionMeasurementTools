def ExtractFiducialsFromTxt(filepath):
    fiducialSet = dict()
    f = open(filepath, 'r')
    for line in f:
        if line[0] == '#':
            continue
        line = line.split(' ')

        FiducialName = line[0]
        xyz = line[2] + ',' + line[3] + ',' + line[4][:-1] + ','
        fiducialSet[FiducialName] = ["vtkMRMLMarkupsFiducialNode_", xyz, "0,0,0,1,1,1,0,", FiducialName,",,vtkMRMLScalarVolumeNode2\n"]

    return fiducialSet


def WriteFiducialsToFile(fiducialSet, outputPath):
    f = open(outputPath, 'w')
    f.write("# Markups fiducial file version = 4.4\n# CoordinateSystem = 0\n# columns = id,x,y,z,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID\n")
    for i, key in enumerate(fiducialSet):
        line = fiducialSet[key]
        f.write(line[0] + str(i) + ',' + line[1] + line[2] + line[3] + line[4])
    f.close()


def main():
    if len(sys.argv) < 3:
        print "correct usage: python ExtractFiducialsToText.py <LANDMARKS FILE> <FCSV FILE>"
        return

    inputFile  = sys.argv[1]
    outputFile = sys.argv[2]

    fiducialSet = ExtractFiducials(inputFile)
    WriteFiducialsToFile(fiducialSet, outputFile)

##################################################

if __name__ == '__main__':
    main()
