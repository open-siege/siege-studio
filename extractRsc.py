
import sys
import json
import struct
import glob

from collections import namedtuple


def readFiles(numFiles, infoFmt, offset):
	files = []
        for i in range(numFiles):
            rawValues = struct.unpack_from(infoFmt, rawData, offset)
            fileName = rawValues[0]
	    fileOffset = rawValues[1]
            fileName =  fileName.split("\0")[0]
            offset += struct.calcsize(infoFmt)
            files.append((fileName, fileOffset))
	return files

importFilenames = sys.argv[1:]

for importFilename in importFilenames:

    print "reading " + importFilename
    try:
        with open(importFilename, "rb") as input_fd:
            rawData = input_fd.read()

        offset = 0
        (numFiles, ) = struct.unpack_from("<L", rawData, offset)
        offset += struct.calcsize("<L")
        infoFmt = "<16sl"
	v1infoFmt = "<16s2l4h"
	
        files = readFiles(numFiles, v1infoFmt, offset)
	
	# this would be true for CW Vengeance of the resource file
	if "." not in files[1][0]:
		files = readFiles(numFiles, infoFmt, offset)
	
	
        for index, file in enumerate(files):
            filename, fileOffset = file
	    print "extracting " + filename
            nextFileOffset = len(rawData)
            if index + 1 < numFiles:
                nextFilename, nextFileOffset = files[index + 1]
            with open("extracted/" + filename,"w") as shapeFile:
                newFileByteArray = bytearray(rawData[fileOffset:nextFileOffset])
                shapeFile.write(newFileByteArray)
   

    except Exception as e:
        print e
