import struct

def readArray(offset, rawData, fmt, numItems):
    items = []
    index = 0
    while index < numItems:
        otherData = struct.unpack_from(fmt, rawData, offset)
        offset += struct.calcsize(fmt)
        items.append(otherData)
        index += 1
    return (offset, items)

def readLod(offset, rawData):
    headerFmt = "<4B3l"

    header = struct.unpack_from(headerFmt, rawData, offset)
    offset += struct.calcsize(headerFmt)
    if header[-1] == 1:
        # still need to figure out why this works
        return (offset + 16, header[-1])
    else:
        shortSize = struct.calcsize("<h")
        offset = rawData.index("#RLM") - shortSize
        dataFmt = "<h4Bl"
        data = struct.unpack_from(dataFmt, rawData, offset)
        offset += struct.calcsize(dataFmt)
        numLod = data[4]
        return (offset, numLod)

def readNumOfMeshes(offset, rawData):
    otherFmt = "<2flLB"
    otherData = struct.unpack_from(otherFmt, rawData, offset)
    print struct.calcsize(otherFmt)
    offset += struct.calcsize(otherFmt)
    numMeshes = otherData[-1]
    return (offset, numMeshes)

def readNumOfVerts(offset, rawData):
    otherFmt = "<Ll"
    otherData = struct.unpack_from(otherFmt, rawData, offset)
    offset += struct.calcsize(otherFmt)
    numVerts = otherData[1]
    return (offset, numVerts)

def readVerts(offset, rawData, numVerts):
    vertFmt = "<3f"
    return readArray(offset, rawData, vertFmt, numVerts)

def readUvVerts(offset, rawData):
    uvVerts = []
    lengthFmt = "<l"
    length = struct.unpack_from(lengthFmt, rawData, offset)
    offset += struct.calcsize(lengthFmt)
    uvFmt = "<2f"
    return readArray(offset, rawData, uvFmt, length[0])

def readTextureName(offset, rawData):
    name = ""
    lengthFmt = "<lB20Bl"
    length = struct.unpack_from(lengthFmt, rawData, offset)
    offset += struct.calcsize(lengthFmt)
    stringFmt = ">" + str(length[-1]) + "s"
    name = struct.unpack_from(stringFmt, rawData, offset)
    offset += struct.calcsize(stringFmt)

    return (offset, name[0])

def readNumFaces(offset, rawData):
    faceFmt = "<LBl"
    numFaces = struct.unpack_from(faceFmt, rawData, offset)
    offset += struct.calcsize(faceFmt)

    return (offset, numFaces[-1] / 3)

def readFaces(offset, rawData, numFaces):
    faceFmt = "<3B"
    return readArray(offset, rawData, faceFmt, numFaces)

def readNumPlanePoints(offset, rawData, numFaces):
    lengthFmt = "<l"
    numPlanePoints = struct.unpack_from(lengthFmt, rawData, offset)
    offset += struct.calcsize(lengthFmt)

    return (offset, numPlanePoints[0])

def readNumPlanePoints(offset, rawData):
    lengthFmt = "<l"
    numPlanePoints = struct.unpack_from(lengthFmt, rawData, offset)
    offset += struct.calcsize(lengthFmt)

    return (offset, numPlanePoints[0])

def readPlanePoints(offset, rawData, numPlanePoints):
    planeFmt = "<4f"
    return readArray(offset, rawData, planeFmt, numPlanePoints)

def readNumNormalPoints(offset, rawData):
    lengthFmt = "<ll"
    numNormals = struct.unpack_from(lengthFmt, rawData, offset)
    offset += struct.calcsize(lengthFmt)

    return (offset, numNormals[1])

def readNormalPoints(offset, rawData, numNormals):
    normalFmt = "<3f"
    return readArray(offset, rawData, normalFmt, numNormals)
