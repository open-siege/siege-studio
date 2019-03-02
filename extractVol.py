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
        headerFmt = "<4s2L"
        fileHeaderFmt = "<LH"
        filenameFmt = "<14sL"
        (header, possiblyVersion, possiblyNumFiles) = struct.unpack_from(headerFmt, rawData, offset)

        if "VOLN" not in header:
            raise ValueError("File header is not VOLN as expected")
        offset += struct.calcsize(headerFmt)
        folders = []
        files = []
        nextIndex = rawData.find("\\\0", offset)
        while nextIndex != -1:
            length = nextIndex - offset
            if length <= 0 or length > 8:
                break
            folderFmt = "<" + str(length) + "s"
            (folderName, ) = struct.unpack_from(folderFmt, rawData, offset)
            folders.append(folderName)
            print folderName
            offset += struct.calcsize(folderFmt) + 2
            nextIndex = rawData.find("\\\0", offset)

        (numFiles, padding) = struct.unpack_from(fileHeaderFmt, rawData, offset)
        offset += struct.calcsize(fileHeaderFmt)

        nextIndex = rawData.find(".", offset)
        nextEosIndex = rawData.find("\0", offset)
        extensionLength = nextEosIndex - nextIndex
        print extensionLength
        while (extensionLength) > 0 and (extensionLength) <= 4:
            (filename, fileOffset) = struct.unpack_from(filenameFmt, rawData, offset)
            offset += struct.calcsize(filenameFmt)
            nextIndex = rawData.find(".", offset)
            nextEosIndex = rawData.find("\0", offset)
            extensionLength = nextEosIndex - nextIndex
            filename = filename.split("\0")[0]
            print (filename, fileOffset)
            files.append((filename, fileOffset))

        if not os.path.exists(destDir):
    	    os.makedirs(destDir)

        for folder in folders:
            newFolder = destDir + "/" + folder
            folderUpper = "." + folder.upper()
            for index, file in enumerate(files):
                if folderUpper in file[0]:
                    if not os.path.exists(newFolder):
                        print "making " + newFolder
                        os.makedirs(newFolder)
                    print "writing " + newFolder + "/" + file[0]
                    with open(newFolder + "/" + file[0],"w") as newFile:
                            offset = file[1]
                            nextOffset = len(rawData)
                            if index + 1 < len(files):
                                nextOffset = files[index + 1][1]

                            newFileByteArray = bytearray(rawData[offset:nextOffset])
                            newFile.write(newFileByteArray)

    except Exception as e:
        print e
