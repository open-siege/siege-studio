import sys
import json
import struct
import glob

import os

importFilenames = sys.argv[1:]

compressionTypes = {
    0: "none",
    1: "rle",
    2: "lz",
    3: "lzh"
}

for importFilename in importFilenames:

    print("reading " + importFilename)
    try:
        with open(importFilename, "rb") as input_fd:
            rawData = input_fd.read()

        destDir = importFilename.replace(".vol", "").replace(".VOL", "")
        offset = 0
        headerFmt = "<4sL"
        fileFmt = "<4s4s"
        itemHeaderFmt = "<4sL"
        itemFmt = "<4LB"
        footerFmt = "<4sL4sL4sL"
        (header, totalFileLength) = struct.unpack_from(headerFmt, rawData, offset)

        if "VOL".encode("utf-8") not in header:
            raise ValueError("File header is not VOL as expected")

        offset = totalFileLength
        footer = struct.unpack_from(footerFmt, rawData, offset)

        if footer[0] != "vols".encode("utf-8"):
            raise ValueError("vols section not found")

        offset += struct.calcsize(footerFmt)
        fileListEndIndex = offset + footer[-1]
        filenames = []
        fileInfo = []
        while offset < fileListEndIndex:
            endIndex = rawData.index("\0".encode("utf-8"), offset)
            filenames.append(rawData[offset:endIndex])
            offset += (endIndex - offset + 1)

        offset = rawData.index("voli".encode("utf-8"), offset)
        itemsHeader = struct.unpack_from(itemHeaderFmt, rawData, offset)

        if itemsHeader[0] != "voli".encode("utf-8"):
            raise ValueError("voli section not found")

        offset += struct.calcsize(itemHeaderFmt)
        finalOffset = offset + itemsHeader[1]
        while offset < finalOffset:
            item = struct.unpack_from(itemFmt, rawData, offset)
            print(item)
            fileInfo.append(item)
            offset += struct.calcsize(itemFmt)
        offset = 0
        files = []

        if not os.path.exists(destDir):
            os.makedirs(destDir)

        for index, info in enumerate(fileInfo):
            offset = info[2]
            (fileHeader, fileLengthRaw) = struct.unpack_from(fileFmt, rawData, offset)
            (fileLength,) = struct.unpack("<L", fileLengthRaw[:-1] + "\0".encode("utf-8"))
            offset += struct.calcsize(fileFmt)
            if fileHeader == "VBLK".encode("utf-8"):
                print("writing " + destDir + "/" + filenames[index].decode("utf-8"))
                with open(destDir + "/" + filenames[index].decode("utf-8"), "wb") as shapeFile:
                        newFileByteArray = bytearray(rawData[offset:offset + info[3]])
                        shapeFile.write(newFileByteArray)
    except Exception as e:
        print(e)
