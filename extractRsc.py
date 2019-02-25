
import sys
import json
import struct
import glob

from collections import namedtuple


importFilenames = sys.argv[1:]

for importFilename in importFilenames:

    print "reading " + importFilename
    try:
        with open("FRONT.OVL", "rb") as input_fd:
            rawData = input_fd.read()

        frontLen = len(rawData)
        with open(importFilename, "rb") as input_fd:
            rawData = input_fd.read()
        # then map them for conversation later
        offset = 0
        (numFiles, ) = struct.unpack_from("<L", rawData, offset)
        offset += struct.calcsize("<L")
        infoFmt = "<2l4h"
        files = []
        for i in range(numFiles):
            filelength = rawData.index("\0", offset) - offset  
              
            fmt = "<" + str(16) + "s"
            (fileName, ) = struct.unpack_from(fmt, rawData, offset)
            fileName =  fileName.split("\0")[0]
            offset += struct.calcsize(fmt)
            fileinfo = struct.unpack_from(infoFmt, rawData, offset)
            offset += struct.calcsize(infoFmt)
            files.append((fileName, fileinfo[0]))
        
        for index, file in enumerate(files):
            filename, fileOffset = file
            nextFileOffset = len(rawData)
            if index + 1 < numFiles:
                nextFilename, nextFileOffset = files[index + 1]
            with open("extracted/" + filename,"w") as shapeFile:
                newFileByteArray = bytearray(rawData[fileOffset:nextFileOffset])
                shapeFile.write(newFileByteArray)
   

    except Exception as e:
        print e
