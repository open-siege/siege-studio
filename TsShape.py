
import sys
import json
import readDts

from collections import namedtuple

with open('structures.json') as f:
    structures = json.load(f)

importFilename = sys.argv[1]
exportFilename = importFilename.replace(".dts", ".obj").replace(".DTS", ".obj")

print "reading " + importFilename

input_fd = open(importFilename, "rb")

# first get the parsed shape datastructures
rawData = input_fd.read()
shape = readDts.readDtsData(structures, rawData)
input_fd.close()

# then map them for conversation later

Sequence = namedtuple("Sequence", "name sequence frameTriggers")
SubSequence = namedtuple("SubSequence", "subSequence sequence keyFrames")
Object = namedtuple("Object", "name object node mesh subSequences")
Node = namedtuple("Node", "name node defaultTransform subSequences parentNode childNodes object")

mappedSequences = []
mappedSubSequences = []
mappedNodes = []
mappedObjects = []

for sequence in shape.sequences:
    sequenceName = shape.names[sequence.fName].name
    sequenceName = sequenceName.split("\0")[0]
    frameTrigs = []
    i = 0
    while i < sequence.fNumFrameTriggers:
        index = sequence.fFirstFrameTrigger + i
        frameTrigs.append(shape.frameTriggers[index])
        i += 1
    mappedSequences.append(Sequence._make((sequenceName, sequence, frameTrigs)))

for subSequence in shape.subSequences:
    sequence = mappedSequences[subSequence.fSequenceIndex]
    keyFrams = []
    i = 0
    while i < subSequence.fnKeyframes:
        index = subSequence.fFirstKeyframe + i
        keyFrams.append(shape.keyframes[index])
        i += 1

    mappedSubSequences.append(SubSequence._make((subSequence, sequence, keyFrams)))

for node in shape.nodes:
    nodeName = shape.names[node.fName].name
    nodeName = nodeName.split("\0")[0]
    someTransform = shape.transforms[node.fDefaultTransform]
    subSeqs = []
    i = 0
    while i < node.fnSubSequences:
        index = node.fFirstSubSequence + i
        subSeqs.append(mappedSubSequences[index])
        i += 1

    parentNode = None
    if node.fParent > -1:
        parentNode = mappedNodes[node.fParent]

    finalNode = Node._make((nodeName, node, someTransform, subSeqs, parentNode, [], []))

    if node.fParent > -1:
        parentNode.childNodes.append(finalNode)

    mappedNodes.append(finalNode)


for object in shape.objects:
    someNode = mappedNodes[object.fNodeIndex]
    someObjectName = shape.names[object.fName].name
    someObjectName = someObjectName.split("\0")[0]
    someMesh = shape.meshes[object.fMeshIndex]
    subSeqs = []
    i = 0
    while i < object.fnSubSequences:
        index = object.fFirstSubSequence + i
        subSeqs.append(mappedSubSequences[index])
        i += 1

    finalObject = Object._make((someObjectName, object, someNode, someMesh, subSeqs))
    someNode.object.append(finalObject)
    mappedObjects.append(finalObject)

# save a new file

shapeFile = open(exportFilename,"w")
faceOffset = 0
for object in mappedObjects:
    expandedVertices = []
    shapeFile.write("o " + object.name + "\r\n")

    someTransform = object.node.defaultTransform
    someMesh = object.mesh

    firstFrame = someMesh.frames[0]
    scaleX = firstFrame.scaleX * someTransform.fScaleX
    scaleY = firstFrame.scaleY * someTransform.fScaleY
    scaleZ = firstFrame.scaleZ * someTransform.fScaleZ

    originX = firstFrame.originX + someTransform.fTranslateX
    originY = firstFrame.originY + someTransform.fTranslateY
    originZ = firstFrame.originZ + someTransform.fTranslateZ

    currentObject = None
    if object.node.parentNode is not None:
        if len(object.node.parentNode.object) > 0:
            currentObject = object.node.parentNode.object[0]

    if len(object.subSequences) > 0:
        while currentObject is not None:

            if len(currentObject.subSequences) == 0:
                break
            originX += currentObject.node.defaultTransform.fTranslateX
            originY += currentObject.node.defaultTransform.fTranslateY
            originZ += currentObject.node.defaultTransform.fTranslateZ
            if currentObject.node.parentNode is not None:
                if len(currentObject.node.parentNode.object) > 0:
                    currentObject = currentObject.node.parentNode.object[0]
                    continue
            currentObject = None

    for vert in someMesh.vertices:
        newVert = (vert.x * scaleX + originX, vert.y * scaleY + originY, vert.z * scaleZ + originZ, structures["normalTable"][vert.normal])
        expandedVertices.append(newVert)
    for vert in expandedVertices:
            shapeFile.write("\tv " + str(vert[0]) + " " + str(vert[1]) + " " + str(vert[2]) + "\r\n")

    for vert in expandedVertices:
            shapeFile.write("\tvn " + str(vert[3][0]) + " " + str(vert[3][1]) + " " + str(vert[3][2]) + "\r\n")

    for face in someMesh.faces:
        firstVert = someMesh.frames[0].firstVert
        idx0 = face.vi1 + firstVert + faceOffset
        idx1 = face.vi2 + firstVert + faceOffset
        idx2 = face.vi3 + firstVert + faceOffset
        shapeFile.write("\tf " + str(idx0 + 1) + " " + str(idx1 + 1) + " " + str(idx2 + 1) + "\r\n")
    faceOffset += len(expandedVertices)

shapeFile.close()
