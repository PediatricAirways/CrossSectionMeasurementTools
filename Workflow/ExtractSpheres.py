# Script that converts a ParaView state file with sphere
# clippings to a clippings file needed by the airway
# processing pipeline

import os
import sys
from xml.dom import minidom

#############################################################################
def itemIsSphere(item):
    rValue = False
    try:
        rValue = item.attributes['type'].value == 'Sphere'
    except:
        pass
  
    return rValue

#############################################################################
def main():
    if (len(sys.argv) < 3):
        sys.stdout.write('Usage: %s <clippings.pvsm> <clippings.txt>\n' % sys.argv[0])
        sys.exit(-1)


    xmldoc = minidom.parse(sys.argv[1])

    # Get the sphere implicit_function proxies
    itemlist = filter(itemIsSphere, xmldoc.getElementsByTagName('Proxy'))

    outputFile = open(sys.argv[2], 'w')
    for item in itemlist:
        for child in item.childNodes:
            if child.attributes:
                childName = child.attributes['name'].value
                if childName == 'Radius':
                    radiusElements = child.getElementsByTagName('Element')
                    radius = radiusElements[0].attributes['value'].value
                    outputFile.write('ClipSphereRadius : ' + radius + '\n')
                elif childName == 'Center':
                    centerElements = child.getElementsByTagName('Element')
                    center = [0, 0, 0]
                    for element in centerElements:
                        index = int(element.attributes['index'].value)
                        value = float(element.attributes['value'].value)
                        center[index] = value
                    outputFile.write('ClipSphereCenter : ')
                    outputFile.write('%f %f %f\n' % (center[0], center[1], center[2]))

#############################################################################
if __name__ == '__main__':
    main()
