import os
import sys

#############################################################################
def main():
    if (len(sys.argv) < 3):
        sys.stderr.write('Usage: %s <input file> <output file>\n' % sys.argv[0])
        sys.exit(-1)

    outputTuples = []
    with open(sys.argv[1], 'r') as f:
        lines = [s.strip() for s in f.readlines()]
        for line in lines:
            namePoints = line.split(':')
            name = namePoints[0].strip()
            pointString = namePoints[1].strip()
            point = pointString.split()
            outputTuples.append( (name, ','.join([str(x) for x in point]) ) )

    counter = 1
    with open(sys.argv[2], 'w') as f:
        f.write('# Markups fiducial file version = 4.4\n')
        f.write('# CoordinateSystem = 0\n')
        f.write('# columns = id,x,y,z,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID\n')
        for value in outputTuples:
            name  = value[0]
            point = value[1]

        f.write('vtkMRMLMarkupsFiducialNode_%d,%s,0,0,0,1,1,1,0,%s,,vtkMRMLScalarVolume1\n' % (counter, point, name))
        counter += 1

#############################################################################
if __name__ == '__main__':
    main()
