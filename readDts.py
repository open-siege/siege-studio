from collections import namedtuple
import struct

Shape = namedtuple("Shape", "header data nodes sequences subSequences keyframes transforms names objects details transitions frameTriggers meshes footer")
Mesh = namedtuple("Mesh", "header vertices textureVertices faces frames")

def readInstance(structs, rawData, offset):
    header = struct.unpack_from(structs["header"]["opening"], rawData, offset)
    offset += struct.calcsize(structs["header"]["opening"])
    (fileLength, classNameLength) = struct.unpack_from(structs["header"]["fileInfo"], rawData, offset)
    offset += struct.calcsize(structs["header"]["fileInfo"])
    classNameFmt = ">" +  str(classNameLength) + "s"
    (className) = struct.unpack_from(classNameFmt, rawData, offset)
    offset += struct.calcsize(classNameFmt)
    if classNameLength < 16:
            offset += 1

    version = struct.unpack_from(structs["header"]["version"], rawData, offset)
    offset += struct.calcsize(structs["header"]["version"])

    ShapeHeader = namedtuple("ObjectHeader", structs["header"]["keys"])
    return (offset, ShapeHeader._make((header[0], fileLength, className[0], version[0])))

def readData(structs, rawData, offset, objectHeader, memberName):
    ShapeHeaderStruct = structs["structures"][objectHeader.className]["versions"][str(objectHeader.version)]["members"][memberName]
    ShapeHeader = namedtuple(memberName, ShapeHeaderStruct["keys"])
    result = ShapeHeader._make(struct.unpack_from(ShapeHeaderStruct["format"], rawData, offset))
    offset += struct.calcsize(ShapeHeaderStruct["format"])
    return (offset, result)

def readArrayData(structs, rawData, offset, objectHeader, memberName, length):
    result = []
    i = 0
    while i <  length:
        rawstructs = readData(structs, rawData, offset, objectHeader, memberName)
        offset = rawstructs[0]
        result.append(rawstructs[1])
        i += 1
    return (offset, result)

def readDtsData(structures, rawData):
    offset = 0

    header = readInstance(structures, rawData, offset)
    offset = header[0]

    shapeHeader = readData(structures, rawData, offset, header[1], "Header")
    offset = shapeHeader[0]

    shapeData = readData(structures, rawData, offset, header[1], "Data")
    offset = shapeData[0]

    nodes = readArrayData(structures, rawData, offset, header[1], "Node", shapeHeader[1].nNodes)
    offset = nodes[0]

    sequences = readArrayData(structures, rawData, offset, header[1], "Sequence", shapeHeader[1].nSeqs)
    offset = sequences[0]

    subSequences = readArrayData(structures, rawData, offset, header[1], "SubSequence", shapeHeader[1].nSubSeqs)
    offset = subSequences[0]

    keyframes = readArrayData(structures, rawData, offset, header[1], "Keyframe", shapeHeader[1].nKeyframes)
    offset = keyframes[0]

    transforms = readArrayData(structures, rawData, offset, header[1], "Transform", shapeHeader[1].nTransforms)
    offset = transforms[0]

    names = readArrayData(structures, rawData, offset, header[1], "Name", shapeHeader[1].nNames)
    offset = names[0]

    objects = readArrayData(structures, rawData, offset, header[1], "Object", shapeHeader[1].nObjects)
    offset = objects[0]

    details = readArrayData(structures, rawData, offset, header[1], "Detail", shapeHeader[1].nDetails)
    offset = details[0]

    transitions = readArrayData(structures, rawData, offset, header[1], "Transition", shapeHeader[1].nTransitions)
    offset = transitions[0]

    frameTriggers = readArrayData(structures, rawData, offset, header[1], "FrameTrigger", shapeHeader[1].nFrameTriggers)
    offset = frameTriggers[0]

    footer = readData(structures, rawData, offset, header[1], "Footer")
    offset = footer[0]

    meshes = []
    i = 0
    faceOffset = 0

    while i < shapeHeader[1].nMeshes:
        #offset = dat.index("PERS", offset)
        meshHeader = readInstance(structures, rawData, offset)
        offset = meshHeader[0]
        mesh = readData(structures, rawData, offset, meshHeader[1], "Header")
        meshOffset = mesh[0]
        vertices = []
        textureVertices = []
        faces = []
        frames = []
        x = 0
        while x < mesh[1].numVerts:
            vertex = readData(structures, rawData, meshOffset, meshHeader[1], "Vertex")
            meshOffset = vertex[0]
            vertices.append(vertex[1])
            x += 1

        x = 0
        while x < mesh[1].numTexVerts:
            vertex = readData(structures, rawData, meshOffset, meshHeader[1], "TextureVertex")
            meshOffset = vertex[0]
            textureVertices.append(vertex[1])
            x += 1

        x = 0
        while x < mesh[1].numFaces:
            vertex = readData(structures, rawData, meshOffset, meshHeader[1], "Face")
            meshOffset = vertex[0]
            faces.append(vertex[1])
            x += 1
        x = 0
        while x < mesh[1].numFrames:
            vertex = readData(structures, rawData, meshOffset, meshHeader[1], "Frame")
            meshOffset = vertex[0]
            frames.append(vertex[1])
            x += 1

        meshes.append(Mesh._make((mesh[1], vertices, textureVertices, faces, frames)))
        offset = meshOffset
        i += 1

    return Shape._make((shapeHeader[1], shapeData[1], nodes[1], sequences[1], subSequences[1], keyframes[1], transforms[1], names[1], objects[1], details[1], transitions[1], frameTriggers[1], meshes, footer[1]))
