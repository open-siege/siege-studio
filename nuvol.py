import sys
import struct
from os import listdir
from os.path import isfile, join

importFilenames = sys.argv[1:]

for importFilename in importFilenames:
    print("packing " + importFilename)

    # thank you stackoverflow - https://stackoverflow.com/questions/3207219/how-do-i-list-all-files-of-a-directory
    filesToPack = [f for f in listdir(importFilename) if isfile(join(importFilename, f))]
    rawBytes = []
    fileDescriptors = []
    mainHeaderFmt = "<4sL"
    fileHeaderFmt = "<4s4B"
    startOffset = struct.calcsize(mainHeaderFmt)
    currentOffset = startOffset

    for file in filesToPack:
        with open(join(importFilename, file), "rb") as outputFile:
            rawData = outputFile.read()
        rawBytes.extend(file.encode("utf-8") + b"\0")
        # id is never used in any of the vol files I have seen
        # name is actually a char* which has random data in a real vol. can be 0
        id = 0
        name = 0
        compressionType = 0
        fileDescriptors.append((id, name, currentOffset, len(rawData), compressionType, rawData))
        currentOffset += len(rawData) + struct.calcsize(fileHeaderFmt)

    header = (b"PVOL", currentOffset)
    finalResult = bytearray(struct.pack(mainHeaderFmt, *header))
    stringSection = (b"vols", len(rawBytes))
    rawHeader = bytearray(struct.pack("<4sL", *stringSection))
    rawHeader.extend(rawBytes)
    finalResult.extend(rawHeader)
    print(finalResult)



