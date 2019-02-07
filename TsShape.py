import struct
import sys
import json

from collections import namedtuple

with open('structures.json') as f:
    data = json.load(f)

importFilename = sys.argv[1]
exportFilename = importFilename.replace(".dts", ".obj").replace(".DTS", ".obj")

print "reading " + importFilename

input_fd = open(importFilename, "rb")

dat = input_fd.read()
offset = 0

def readInstance(data, offset):
    header = struct.unpack_from(data["header"]["opening"], dat, offset)
    offset += struct.calcsize(data["header"]["opening"])
    (fileLength, classNameLength) = struct.unpack_from(data["header"]["fileInfo"], dat, offset)
    offset += struct.calcsize(data["header"]["fileInfo"])
    classNameFmt = ">" +  str(classNameLength) + "s"
    (className) = struct.unpack_from(classNameFmt, dat, offset)
    offset += struct.calcsize(classNameFmt)
    if classNameLength < 16:
            offset += 1

    version = struct.unpack_from(data["header"]["version"], dat, offset)
    offset += struct.calcsize(data["header"]["version"])

    ShapeHeader = namedtuple("ObjectHeader", data["header"]["keys"])
    return (offset, ShapeHeader._make((header[0], fileLength, className[0], version[0])))

def readData(data, offset, objectHeader, memberName):
    ShapeHeaderStruct = data["structures"][objectHeader.className]["versions"][str(objectHeader.version)]["members"][memberName]
    ShapeHeader = namedtuple(memberName, ShapeHeaderStruct["keys"])
    result = ShapeHeader._make(struct.unpack_from(ShapeHeaderStruct["format"], dat, offset))
    offset += struct.calcsize(ShapeHeaderStruct["format"])
    return (offset, result)

def readArrayData(data, offset, objectHeader, memberName, length):
    result = []
    i = 0
    while i <  length:
        rawData = readData(data, offset, objectHeader, memberName)
        offset = rawData[0]
        result.append(rawData[1])
        i += 1
    return (offset, result)

header = readInstance(data, offset)
offset = header[0]

shapeHeader = readData(data, offset, header[1], "Header")
offset = shapeHeader[0]

shapeData = readData(data, offset, header[1], "Data")
offset = shapeData[0]

nodes = readArrayData(data, offset, header[1], "Node", shapeHeader[1].nNodes)
offset = nodes[0]

sequences = readArrayData(data, offset, header[1], "Sequence", shapeHeader[1].nSeqs)
offset = sequences[0]

subSequences = readArrayData(data, offset, header[1], "SubSequence", shapeHeader[1].nSubSeqs)
offset = subSequences[0]

keyframes = readArrayData(data, offset, header[1], "Keyframe", shapeHeader[1].nKeyframes)
offset = keyframes[0]

transforms = readArrayData(data, offset, header[1], "Transform", shapeHeader[1].nTransforms)
offset = transforms[0]

names = readArrayData(data, offset, header[1], "Name", shapeHeader[1].nNames)
offset = names[0]

objects = readArrayData(data, offset, header[1], "Object", shapeHeader[1].nObjects)
offset = objects[0]

details = readArrayData(data, offset, header[1], "Detail", shapeHeader[1].nDetails)
offset = details[0]

transitions = readArrayData(data, offset, header[1], "Transition", shapeHeader[1].nTransitions)
offset = transitions[0]

frameTriggers = readArrayData(data, offset, header[1], "FrameTrigger", shapeHeader[1].nFrameTriggers)
offset = frameTriggers[0]

footer = readData(data, offset, header[1], "Footer")
offset = footer[0]

meshes = []
i = 0
faceOffset = 0
Mesh = namedtuple("Mesh", "header vertices faces frames")
while i < shapeHeader[1].nMeshes:
    #offset = dat.index("PERS", offset)
    meshHeader = readInstance(data, offset)
    offset = meshHeader[0]
    mesh = readData(data, offset, meshHeader[1], "Header")
    meshOffset = mesh[0]
    vertices = []
    faces = []
    frames = []
    x = 0
    while x < mesh[1].numVerts:
        vertex = readData(data, meshOffset, meshHeader[1], "Vertex")
        meshOffset = vertex[0]
        vertices.append(vertex[1])
        x += 1

    x = 0
    while x < mesh[1].numTexVerts:
        vertex = readData(data, meshOffset, meshHeader[1], "TextureVertex")
        meshOffset = vertex[0]
        x += 1

    x = 0
    while x < mesh[1].numFaces:
        vertex = readData(data, meshOffset, meshHeader[1], "Face")
        meshOffset = vertex[0]
        faces.append(vertex[1])
        x += 1
    x = 0
    while x < mesh[1].numFrames:
        vertex = readData(data, meshOffset, meshHeader[1], "Frame")
        meshOffset = vertex[0]
        frames.append(vertex[1])
        x += 1

    meshes.append(Mesh._make((meshHeader[1], vertices, faces, frames)))
    offset = meshOffset
    i += 1


shapeFile = open(exportFilename,"w")
faceOffset = 0
for object in objects[1]:
    expandedVertices = []
    someNode = nodes[1][object.fNodeIndex]
    someNodeName = names[1][someNode.fName].name
    someNodeName = someNodeName.split("\0")[0]

    shapeFile.write("o " + someNodeName + "\r\n")
    someTransform = transforms[1][someNode.fDefaultTransform]
    someMesh = meshes[object.fMeshIndex]

    for vert in someMesh.vertices:
        firstFrame = someMesh.frames[0]
        scaleX = firstFrame.scaleX * someTransform.fScaleX
        scaleY = firstFrame.scaleY * someTransform.fScaleY
        scaleZ = firstFrame.scaleZ * someTransform.fScaleZ
        originX = firstFrame.originX + someTransform.fTranslateX
        originY = firstFrame.originY + someTransform.fTranslateY
        originZ = firstFrame.originZ + someTransform.fTranslateZ
        newVert = (vert.x * scaleX + originX, vert.y * scaleY + originY, vert.z * scaleZ + originZ)
        expandedVertices.append(newVert)

    for vert in expandedVertices:
            shapeFile.write("\tv " + str(vert[0]) + " " + str(vert[1]) + " " + str(vert[2]) + "\r\n")

    for face in someMesh.faces:
        firstVert = someMesh.frames[0].firstVert
        idx0 = face.vi1 + firstVert + faceOffset
        idx1 = face.vi2 + firstVert + faceOffset
        idx2 = face.vi3 + firstVert + faceOffset
        shapeFile.write("\tf " + str(idx0 + 1) + " " + str(idx1 + 1) + " " + str(idx2 + 1) + "\r\n")
    faceOffset += len(expandedVertices)

shapeFile.close()
