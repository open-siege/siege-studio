import sys
import json
import struct
import glob

import os

importFilenames = sys.argv[1:]

paletteHeaderFmt = "<4sLL"
headerFmt = "<4sL"
fileHeader = "PBMP"
header = "head"
dataHeader = "data"
dataHeaderFmt = "<5L"


for importFilename in importFilenames:

    print "reading " + importFilename
    try:
        with open(importFilename, "rb") as input_fd:
            rawData = input_fd.read()
        offset = 0
        firstHeader = struct.unpack_from(headerFmt, rawData, offset)
        if firstHeader[0] != fileHeader:
            raise ValueError("File header is not PBMP as expected")
        offset += struct.calcsize(headerFmt)
        (headTag, headSize) = struct.unpack_from(headerFmt, rawData, offset)
        if headTag != header:
            raise ValueError("file does not have head section as expected")

        if headSize != struct.calcsize(dataHeaderFmt):
            raise ValueError("header format is not the correct size")
        offset += struct.calcsize(headerFmt)
        (ver, w, h, bpp, attr) = struct.unpack_from(dataHeaderFmt, rawData, offset)
        offset += struct.calcsize(dataHeaderFmt)
        (dataTag, dataLength) = struct.unpack_from(headerFmt, rawData, offset)
        offset += struct.calcsize(headerFmt)
        print offset
        rawDataFmt = "<" + str(dataLength) + "B"
        rawBytes = struct.unpack_from(rawDataFmt, rawData, offset)
        offset += struct.calcsize(rawDataFmt)

        (dtlTag, dtlLength) = struct.unpack_from(headerFmt, rawData, offset)
        offset += struct.calcsize(headerFmt)
        rawDetailsFmt = "<" + str(dtlLength) + "B"
        rawDetails = struct.unpack_from(rawDetailsFmt, rawData, offset)
        offset += struct.calcsize(rawDetailsFmt)
        print struct.unpack_from(paletteHeaderFmt, rawData, offset)
        offset += struct.calcsize(paletteHeaderFmt)
        print (offset, len(rawData))

    except Exception as e:
        print e
