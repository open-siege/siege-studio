
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
    mappedItems = list(map(lambda x: x[1], items))
    minY = min(mappedItems)
    maxY = max(mappedItems)
    return (minY, maxY)

def getMaxMinX(items):
    mappedItems = list(map(lambda x: x[0], items))
    minX = min(mappedItems)
    maxX = max(mappedItems)
    return (minX, maxX)

for importFilename in importFilenames:
    exportFilename = importFilename.replace(".erf", ".obj").replace(".ERF", ".obj")
    try:
        mechBody = {}
        bodyPartName = importFilename.replace(".erf", "").replace(".ERF", "")
        if bodyPartName.endswith("hip") or bodyPartName.endswith("leg"):
            print "reading " + importFilename
            minMaxX = (0, 0)
            minMaxY = (0, 0)
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
            mechBody[bodyPartName] = meshes
            if bodyPartName.endswith("hip"):
                minMaxX = getMaxMinX(allVerts)
                minMaxY = getMaxMinY(allVerts)
                print minMaxX


    except Exception as e:
        print e
