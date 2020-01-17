
import sys
import json
import glob

import readDts
import mapObjects
import writeObj

from collections import namedtuple

with open('structures.json') as f:
    structures = json.load(f)

importFilenames = sys.argv[1:]

for importFilename in importFilenames:
    exportFilename = importFilename.replace(".dts", ".obj").replace(".DTS", ".obj")

    print("reading " + importFilename)
    try:
        with open(importFilename, "rb") as input_fd:
            # first get the parsed shape datastructures
            rawData = input_fd.read()
            shape = readDts.readDtsData(structures, rawData)
        # then map them for conversation later
        mappedDetails = mapObjects.mapObjects(shape, False)

        print("writing " + exportFilename)
        # save a new file
        with open(exportFilename,"w") as shapeFile:
            #TODO move the normal table out of the obj writer into the object mapper
            writeObj.writeObj(mappedDetails, structures["normalTable"], shapeFile)
        print("completed writing " + exportFilename)
    except Exception as e:
        print(e)
