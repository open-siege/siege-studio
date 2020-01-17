# from https://math.stackexchange.com/questions/40164/how-do-you-rotate-a-vector-by-a-unit-quaternion
def quaternion_mult(q,r):
    return [r[0]*q[0]-r[1]*q[1]-r[2]*q[2]-r[3]*q[3],
            r[0]*q[1]+r[1]*q[0]-r[2]*q[3]+r[3]*q[2],
            r[0]*q[2]+r[1]*q[3]+r[2]*q[0]-r[3]*q[1],
            r[0]*q[3]-r[1]*q[2]+r[2]*q[1]+r[3]*q[0]]

def point_rotation_by_quaternion(point,q):
    r = [0]+point
    q_conj = [q[0],-1*q[1],-1*q[2],-1*q[3]]
    return quaternion_mult(quaternion_mult(q,r),q_conj)[1:]

def writeObject(object, normalTable, shapeFile, faceOffset):
    magicValue = 32767
    expandedVertices = []
    shapeFile.write("o " + str(object.name) + "\r\n")

    someTransform = object.node.defaultTransform
    someMesh = object.mesh

    firstFrame = someMesh.frames[0]
    scaleX = firstFrame.scaleX * someTransform.fScaleX
    scaleY = firstFrame.scaleY * someTransform.fScaleY
    scaleZ = firstFrame.scaleZ * someTransform.fScaleZ

    originX = firstFrame.originX
    originY = firstFrame.originY
    originZ = firstFrame.originZ

    originX += someTransform.fTranslateX
    originY += someTransform.fTranslateY
    originZ += someTransform.fTranslateZ

    quat = [float(someTransform.fRotateW) / magicValue, float(someTransform.fRotateX) / magicValue, float(someTransform.fRotateY) / magicValue, float(someTransform.fRotateZ) / magicValue]
    #originX, originY, originZ = point_rotation_by_quaternion([originX, originY, originZ], quat)

    currentObject = None
    if object.node.parentNode is not None:
        if object.node.parentNode.object["instance"] is not None:
            currentObject = object.node.parentNode.object["instance"]

    if len(object.sequences) > 0:
        while currentObject is not None:

            if len(currentObject.sequences) == 0:
                break
            parentTransform = currentObject.node.defaultTransform
            originX += parentTransform.fTranslateX
            originY += parentTransform.fTranslateY
            originZ += parentTransform.fTranslateZ
            quat = [float(parentTransform.fRotateW) / magicValue, float(parentTransform.fRotateX) / magicValue, float(parentTransform.fRotateY) / magicValue, float(parentTransform.fRotateZ) / magicValue]
            #originX, originY, originZ = point_rotation_by_quaternion([originX, originY, originZ], quat)
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
    for key, node in rootNode.childNodes.items():
        object = node.object["instance"]
        if object is not None:
            faceOffset = writeObject(object, normalTable, shapeFile, faceOffset)
        for key, childNode in node.childNodes.items():
            faceOffset = writeWriteNode(childNode, normalTable, shapeFile, faceOffset)
    return faceOffset

def writeObj(mappedDetails, normalTable, shapeFile):
    faceOffset = 0
    rootNode = mappedDetails[0].rootNode
    writeWriteNode(rootNode, normalTable, shapeFile, faceOffset)
