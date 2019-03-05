import sys
import json
import struct
import glob

import os

filenameFmt = "<13sL"

def getFileinfo(rawData, offset):
    firstFile = struct.unpack_from(filenameFmt, rawData, offset)
    print firstFile
    offset += struct.calcsize(filenameFmt)
    files = []
    currentFile = firstFile
    files.append((currentFile[0], offset, currentFile[1]))
    while offset < len(rawData):
        offset += currentFile[-1]
        if offset + struct.calcsize(filenameFmt) > len(rawData):
            break
        while rawData[offset] == "\0":
            offset += 1
        if (offset + struct.calcsize(filenameFmt)) >= len(rawData):
            break
        currentFile = struct.unpack_from(filenameFmt, rawData, offset)
        offset += struct.calcsize(filenameFmt)
        files.append((currentFile[0], offset, currentFile[1]))
    return files
