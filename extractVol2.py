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

        destDir = importFilename.replace(".vol", "").replace(".VOL", "")
        offset = 0
        volIndexHeaderFmt = "<4sL"
        volIndexHeaderTag = "voli"
        headerFmt = "<4sL4sL4sLL"
        filenameFmt = "<13s"
        fileHeaderFmt = "<4s4s"
        itemFmt = "<7L"

        header = struct.unpack_from(headerFmt, rawData, offset)
        if "VOL" not in header[0]:
            raise ValueError("File header is not VOL as expected")
        if "volh" not in header[2]:
            raise ValueError("File header does not have volh as expected")
        if "vols" not in header[4]:
            raise ValueError("File header does not have vols as expected")
        offset += struct.calcsize(headerFmt)

        print len(rawData)
        for val in header:
            print val
        files = []
        nextIndex = rawData.find("\0", offset)
        while nextIndex != -1:
            length = nextIndex - offset
            if length <= 0 or length > 15:
                break
            fileFmt = "<" + str(length) + "s"
            (filename, ) = struct.unpack_from(fileFmt, rawData, offset)
            files.append(filename)
            offset += struct.calcsize(fileFmt) + 1
            nextIndex = rawData.find("\0", offset)
        offset = rawData.find(volIndexHeaderTag, offset)
        (volIndexHeader, unk1) = struct.unpack_from(volIndexHeaderFmt, rawData, offset)

        if volIndexHeader != volIndexHeaderTag:
            raise ValueError("File header does not have voli as expected")
        offset += struct.calcsize(volIndexHeaderFmt)
        totalLength = len(files)
        headerLength = totalLength / 2
        isEven = totalLength % 2 == 0
        fileInfo = []
        nameIndex = 0
        for index in range(headerLength):
            info = struct.unpack_from(itemFmt, rawData, offset)
            fileOffset = info[1]
            fileLength = info[2]
            fileInfo.append((files[nameIndex], fileOffset))
            if nameIndex + 1 < totalLength:
                nextFileOffset = fileOffset + fileLength + struct.calcsize(fileHeaderFmt)
                fileInfo.append((files[nameIndex + 1], nextFileOffset))
            nameIndex += 2
            offset += struct.calcsize(itemFmt)
        if not os.path.exists(destDir):
    	    os.makedirs(destDir)
        for index, info in enumerate(fileInfo):

            if index > 10:
                break
            offset = info[1]
            (fileHeader, fileLengthRaw) = struct.unpack_from(fileHeaderFmt, rawData, offset)
            (fileLength,) = struct.unpack("<L", fileLengthRaw[:-1] + "\0")
            offset += struct.calcsize(fileHeaderFmt)
            if fileHeader == "VBLK":
                print "writing " + destDir + "/" + info[0] + " " + str(fileLength)
                endOffset = len(rawData)
                if index + 1 < len(fileInfo):
                    endOffset = fileInfo[index + 1][1]
                print (offset - struct.calcsize(fileHeaderFmt))
                print (endOffset, offset + fileLength)
                with open(destDir + "/" + info[0],"w") as shapeFile:
        		        newFileByteArray = bytearray(rawData[offset:endOffset])
        		        shapeFile.write(newFileByteArray)


    except Exception as e:
        print e
