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
        headerFmt = "<4sL4sL4sLL"
        filenameFmt = "<13s"
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
            if length <= 0 or length > 13:
                break
            fileFmt = "<" + str(length) + "s"
            (filename, ) = struct.unpack_from(fileFmt, rawData, offset)
            files.append(filename)
            print filename
            offset += struct.calcsize(fileFmt) + 1
            nextIndex = rawData.find("\0", offset)
        nextFile = rawData.find("VBLK", offset)
        filecount = 0
        while nextFile != -1:
            if len(files) > filecount:
                if "dts" in files[filecount]:
                    print files[filecount]
                    nextFile = rawData.find("VBLK", nextFile + 5)
                    filecount += 1
                    print rawData[nextFile + 4]
                    continue
            filecount += 1
            nextFile = rawData.find("VBLK", nextFile + 5)
        print (filecount, len(files))


    except Exception as e:
        print e
