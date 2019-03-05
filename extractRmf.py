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

        destDir = importFilename.replace(".rmf", "").replace(".RMF", "")
        offset = 0
        headerFmt = "<3H"
        filenameFmt = "<13s"
        header = struct.unpack_from(headerFmt, rawData, offset)
        rmfTag = 256
        versionTagMin = 1700
        if header[0] == 256 and header[1] > 1700:
            print ("number of files", header[2])


    except Exception as e:
        print e
