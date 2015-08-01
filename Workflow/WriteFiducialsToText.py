import sys

def ExtractFiducials(filepath):
    fiducialSet = dict()
    f = open(filepath, 'r')
    for line in f:
        if line[0] == '#':
            continue
        line = line.split(',')

        FiducialName = c = ''.join(line[11].split(' '))
        xyz = line[1] + ' ' + line[2] + ' ' + line[3]

        fiducialSet[FiducialName] = FiducialName + " : " + xyz + "\n"

    return fiducialSet


def WriteFiducialsToFile(fiducialSet, outputPath):
    f = open(outputPath, 'w')
    for key in fiducialSet:
        f.write(fiducialSet[key])
    f.close()


def main():
    if len(sys.argv) < 3:
        print "correct usage: python ExtractFiducialsToText.py <INPUT FIDUCIALS FILE> <LANDMARKS FILE>"
        return

    inputFile  = sys.argv[1]
    outputFile = sys.argv[2]

    fiducialSet = ExtractFiducials(inputFile)
    WriteFiducialsToFile(fiducialSet, outputFile)

##################################################

if __name__ == '__main__':
    main()
