def writeObj(mappedObjects, normalTable, shapeFile):
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
