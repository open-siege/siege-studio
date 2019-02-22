
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

def getMaxMinY(items):
    mappedItems = list(map(lambda x: x[1]))
    minY = min(mappedItems)
    maxY = max(mappedItems)
    return (maxY, minY)

def getMaxMinX(items):
    mappedItems = list(map(lambda x: x[0]))
    minX = min(mappedItems)
    maxX = max(mappedItems)
    return (maxY, minY)

for importFilename in importFilenames:
    exportFilename = importFilename.replace(".erf", ".obj").replace(".erf", ".obj")
    try:
        print "reading " + importFilename
        with open(importFilename, "rb") as input_fd:
            # first get the parsed shape datastructures
            rawData = input_fd.read()
            offset, numLod = readErf.readLod(0, rawData)
            offset, numMeshes = readErf.readNumOfMeshes(offset, rawData)
            i = 0
            meshes = []
            while i < numMeshes:
                offset, numVerts = readErf.readNumOfVerts(offset, rawData)
                offset, verts = readErf.readVerts(offset, rawData, numVerts)
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

        with open(exportFilename,"w") as shapeFile:
            vertIndex = 0
            for index, mesh in enumerate(meshes):
                name, verts, uvVerts, faces = mesh
                shapeFile.write("g " + str(index) + "\r\n")
                for vertex in verts:
                    shapeFile.write("\tv " + str(vertex[0]) + " " + str(vertex[1]) + " " + str(vertex[2]) + "\r\n")

                for face in faces:
                    shapeFile.write("\tf ")
                    for value in face:
                        shapeFile.write(str(value + 1 + vertIndex) + " ")
                    shapeFile.write("\r\n")
                vertIndex += len(verts)
    except Exception as e:
        print e
