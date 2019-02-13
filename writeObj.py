def writeObject(object, normalTable, shapeFile, faceOffset):
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
        if object.node.parentNode.object["instance"] is not None:
            currentObject = object.node.parentNode.object["instance"]

    if len(object.sequences) > 0:
        print object.name + " " +  object.sequences[0].name
        while currentObject is not None:

            if len(currentObject.sequences) == 0:
                break
            originX += currentObject.node.defaultTransform.fTranslateX
            originY += currentObject.node.defaultTransform.fTranslateY
            originZ += currentObject.node.defaultTransform.fTranslateZ
            if currentObject.node.parentNode is not None:
                if currentObject.node.parentNode.object["instance"] is not None:
                    currentObject = currentObject.node.parentNode.object["instance"]
                    continue
            currentObject = None

    for vert in someMesh.vertices:
        newVert = (vert.x * scaleX + originX, vert.y * scaleY + originY, vert.z * scaleZ + originZ, normalTable[vert.normal])
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
    return faceOffset

def writeWriteNode(rootNode, normalTable, shapeFile, faceOffset):
    object = rootNode.object["instance"]
    if object is not None:
        faceOffset = writeObject(object, normalTable, shapeFile, faceOffset)

    for node in rootNode.childNodes:
        object = node.object["instance"]
        if object is not None:
            faceOffset = writeObject(object, normalTable, shapeFile, faceOffset)
        for childNode in node.childNodes:
            print childNode.name
            faceOffset = writeWriteNode(childNode, normalTable, shapeFile, faceOffset)
    return faceOffset

def writeObj(mappedDetails, normalTable, shapeFile):
    faceOffset = 0
    rootNode = mappedDetails[0].rootNode
    writeWriteNode(rootNode, normalTable, shapeFile, faceOffset)
