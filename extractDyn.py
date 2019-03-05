import sys
import json
import struct
import glob

import os

importFilenames = sys.argv[1:]

for importFilename in importFilenames:

    print "reading " + importFilename
    try:
        with open(importFilename, "rb") as input_fd:
            rawData = input_fd.read()

        destDir = importFilename.replace(".dyn", "").replace(".DYN", "")
        offset = 0
        headerFmt = "<20s"
        filenameFmt = "<h13sL"
        header = struct.unpack_from(headerFmt, rawData, offset)
        dynTag = "Dynamix Volume File\0"
        if header[0] != dynTag:
            raise ValueError("File header is not " + dynTag)
        firstFileIndex = -1
        offset += struct.calcsize(headerFmt)
        firstFileOffset = offset
        validTypes = ["OVL", "FMD", "DAT"]
        while firstFileIndex == -1:
            firstFileIndex = rawData.find(".", firstFileOffset)
            firstFile = rawData[firstFileIndex + 1:firstFileIndex + 4]
            if firstFile not in validTypes:
                firstFileOffset = firstFileIndex + 4
                firstFileIndex = -1

        firstFile = "SXCODE.OVL"
        secondFile = "SQ_MARK.DAT"
        thirdFile = "KEYS.DAT"
        fourthFile = "SHADES.DAT"



        secondIndex = rawData.index(firstFile)
        thirdIndex = rawData.index(secondFile)
        print (secondIndex, thirdIndex, thirdIndex - secondIndex)


    except Exception as e:
        print e
