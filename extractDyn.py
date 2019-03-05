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
        filenameFmt = "<13sL"
        header = struct.unpack_from(headerFmt, rawData, offset)
        dynTag = "Dynamix Volume File\0"
        if header[0] != dynTag:
            raise ValueError("File header is not " + dynTag)
        firstFileIndex = -1
        offset += struct.calcsize(headerFmt)
        firstFileOffset = offset
        validTypes = ["OVL", "FMD", "DAT"]
        firstFile = None
        while firstFileIndex == -1:
            firstFileIndex = rawData.find(".", firstFileOffset)
            firstFileExt = rawData[firstFileIndex + 1:firstFileIndex + 4]
            if firstFileExt not in validTypes:
                firstFileOffset = firstFileIndex + 4
                firstFileIndex = -1
                continue
            endOfFileIndex = firstFileIndex + 4 + 1
            while rawData[endOfFileIndex] == "\0":
                endOfFileIndex += 1
            endOfFileIndex += struct.calcsize("<L")
            startOfFileIndex = endOfFileIndex - struct.calcsize(filenameFmt)
            offset = startOfFileIndex
            firstFile = struct.unpack_from(filenameFmt, rawData, offset)
            offset += struct.calcsize(filenameFmt)

        if firstFile is None:
            raise ValueError("Could not find first file to determine first offset")
        files = []
        currentFile = firstFile
        files.append(currentFile)
        for i in range(98):
            offset += currentFile[-1]
            if offset + struct.calcsize(filenameFmt) > len(rawData):
                break
            while rawData[offset] == "\0":
                offset += 1
            if (offset + struct.calcsize(filenameFmt)) >= len(rawData):
                break
            currentFile = struct.unpack_from(filenameFmt, rawData, offset)
            files.append(currentFile)
            offset += struct.calcsize(filenameFmt)

        for file in files:
            print file

    except Exception as e:
        print e
