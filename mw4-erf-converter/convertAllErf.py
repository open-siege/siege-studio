
import sys
import json
import glob

import readErf

importFilenames = sys.argv[1:]

# lower half of mech
# hip
    # (l/r)uleg (u for upper)
    # (l/r)mleg (m for middle)
    # (l/r)dleg (d for down)
        # (l/r)foot
        # (l/r)ftoe (f for front)
        # (l/r)btoe (b for back)

def getMaxMin(items, index):
    mappedItems = list(map(lambda x: x[index], items))
    minX = min(mappedItems)
    maxX = max(mappedItems)
    return (minX, maxX)

def addBottomTransform(mechBody, names, index):
    value = 0
    for index, name in enumerate(names):
        if name in mechBody:
            bodyPart = mechBody[name][0][1]
            minY, maxY = getMaxMin(bodyPart, index)
            if index == 0:
                value = minY
            else:
                value += minY
    return value

mechName = None
exportFilename = None
mechBody = {}
mechBodyOffsets = {}
for importFilename in importFilenames:
    partNameFilename = importFilename.split("/")[-1]
    partName = partNameFilename.replace(".erf", "")
    if mechName is None:
        mechName = importFilename.split("/")[-2]
        print "mech name is " + mechName
    if exportFilename is None:
        exportFilename = importFilename.replace(partNameFilename, mechName + ".obj")
        print "export name is " + exportFilename
    try:
        with open(importFilename, "rb") as input_fd:
            # first get the parsed shape datastructures
            rawData = input_fd.read()
            offset, numLod = readErf.readLod(0, rawData)
            offset, numMeshes = readErf.readNumOfMeshes(offset, rawData)
            i = 0
            meshes = []
            allVerts = []
            while i < numMeshes:
                offset, numVerts = readErf.readNumOfVerts(offset, rawData)
                offset, verts = readErf.readVerts(offset, rawData, numVerts)
                allVerts = allVerts + verts
                offset, uvVerts = readErf.readUvVerts(offset, rawData)
                offset, name = readErf.readTextureName(offset, rawData)
                offset, numFaces = readErf.readNumFaces(offset, rawData)
                offset, faces = readErf.readFaces(offset, rawData, numFaces)
                offset, numPlanePoints = readErf.readNumPlanePoints(offset, rawData)
                offset, planePoints = readErf.readPlanePoints(offset, rawData, numPlanePoints)
                offset, numNormals = readErf.readNumNormalPoints(offset, rawData)
                offset, normals = readErf.readNormalPoints(offset, rawData, numNormals)
                meshes.append((name, verts, uvVerts, faces));
                i += 1
        mechBody[partName] = meshes


    except Exception as e:
        print e

print "writing " + exportFilename
with open(exportFilename,"w") as shapeFile:
    vertIndex = 0
    for meshName in mechBody:
        x = 0
        y = 0
        z = 0
        if "arm" in meshName:
            y = 2
            x = 4
        if "gun" in meshName:
            y = 2
            x = 3
        if "mleg" in meshName:
            uleg = meshName.replace("mleg", "uleg")
            y = addBottomTransform(mechBody, [uleg], 1)
        if "dleg" in meshName:
            mleg = meshName.replace("dleg", "mleg")
            uleg = meshName.replace("dleg", "uleg")
            y = addBottomTransform(mechBody, [mleg, uleg], 1)

        if "foot" in meshName:
            dleg = meshName.replace("foot", "dleg")
            mleg = meshName.replace("foot", "mleg")
            uleg = meshName.replace("foot", "uleg")
            y = addBottomTransform(mechBody, [dleg, mleg, uleg], 1)

        if "toe" in meshName:
            dleg = meshName.replace("toe", "dleg")
            mleg = meshName.replace("toe", "mleg")
            uleg = meshName.replace("toe", "uleg")
            y = addBottomTransform(mechBody, [dleg, mleg, uleg], 1)

        if "_l" in meshName:
            x = 2

        if "_r" in meshName:
            x = -2
        mechBodyOffsets[meshName] = (x, y, z)
    for meshName in mechBody:
        if "cage" in meshName or "_dam" in meshName:
            continue
        offsets = mechBodyOffsets[meshName]
        meshes = mechBody[meshName]
        shapeFile.write("o " + meshName + "\r\n")
        for index, mesh in enumerate(meshes):
            name, verts, uvVerts, faces = mesh
            shapeFile.write("\tg " + partName + "-" +  str(index) + "\r\n")
            for vertex in verts:
                shapeFile.write("\t\tv " + str(vertex[0] + offsets[0]) + " " + str(vertex[1]  + offsets[1]) + " " + str(vertex[2]  + offsets[2]) + "\r\n")

            for face in faces:
                shapeFile.write("\t\tf ")
                for value in face:
                    shapeFile.write(str(value + 1 + vertIndex) + " ")
                shapeFile.write("\r\n")
            vertIndex += len(verts)
