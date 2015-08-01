#############################################################################
def ReadAllPoints(fcsvFilePath):
    """Read a list of tuples from a Slicer FCSV file. Tuple order is
    (name, description, x, y, z).
    """
    lst = []
    with open(fcsvFilePath, 'r') as f:
        for line in f:
            line = line.strip()
            if (line[0] == '#'):
                continue
            tokens = line.split(',')
            if (tokens[0].startswith('vtkMRMLMarkupsFiducialNode')):
                lst.append((tokens[11], tokens[12], tokens[1], tokens[2], tokens[3]))

    return lst
