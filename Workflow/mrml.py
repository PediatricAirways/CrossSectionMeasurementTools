# Module with utilities for parsing a Slicer MRML file and returns 

import sys

from xml.dom import minidom

#############################################################################
def ElementHasAttributeWithValue(element, attribute, value):
    """Returns true if the element has an attriibute with the given value.
    """
    rValue = False
    try:
        rValue = element.attributes[attribute].value == value
    except:
        pass

    return rValue

#############################################################################
def ElementHasAirwaySegmenterNameAttribute():
    """Returns test function that returns true if element has an attribute
    "name" with value 'Airway Segmentation'.
    """
    def internal(element):
        return ElementHasAttributeWithValue(element, "name", "Airway Segmentation")

    return internal

#############################################################################
def ParseCommandLineModule(xmlFilePath):
    """Returns a list of attribute/value pairs from a CommandLineModule element
    whose name attribute matches the name argument. The returned list format
    is of the form [(attribute1, value1), (attribute2, value2), ...]
    """

    xmldoc = minidom.parse(xmlFilePath)
    elementList = filter(ElementHasAirwaySegmenterNameAttribute(),
                         xmldoc.getElementsByTagName('CommandLineModule'))

    # Filter out children of SceneView
    elementList = filter(lambda x: x.parentNode and x.parentNode.tagName != 'SceneView', elementList)
    if (len(elementList) == 0):
        raise LookupError('No CommandLineModule entry found in MRML file.\nMake sure to run AirwaySegmentation module and then save the MRML file.\n')

    element = elementList[0]

    # For each element extract attribute/value pairs and stuff into output list.
    attributeValueList = {}
    for key in element.attributes.keys():
        attributeValueList[str(key)] = str(element.attributes[key].value)

    # Remove attributes that are common to CommandLineModules
    commonAttributes = ['id', 'name', 'hideFromEditors', 'selectable', 'selected',
                        'attributes', 'title', 'version', 'autorunmode', 'autorun']
    attributeValueList = RemoveAttributeValue(attributeValueList, commonAttributes)

    return attributeValueList

#############################################################################
def RemoveAttributeValue(lst, attributeList):
    """Removes all attributes that have any of the attribute names
       in the attributeList parameter.
    """
    return {k:v for (k,v) in lst.iteritems() if k not in attributeList}

#############################################################################
def main():
    import sys
    args = ParseCommandLineModule(sys.argv[1])
    print args

#############################################################################
if __name__ == '__main__':
    main()
