
import sys
import json
import readDts
import mapObjects
import writeObj

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
mappedObjects = mapObjects.mapObjects(shape)

# save a new file
shapeFile = open(exportFilename,"w")

#TODO move the normal table out of the obj writer into the object mapper
writeObj.writeObj(mappedObjects, structures["normalTable"], shapeFile)

shapeFile.close()
