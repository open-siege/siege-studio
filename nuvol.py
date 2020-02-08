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
    fileInfoFmt = "<4LB"
    startOffset = struct.calcsize(mainHeaderFmt)
    currentOffset = startOffset
    nameOffset = 0

    for file in filesToPack:
        print("reading " + join(importFilename, file))
        with open(join(importFilename, file), "rb") as outputFile:
            rawData = bytearray(outputFile.read())
        rawBytes.extend(file.encode("utf-8") + b"\0")
        # id is never used in any of the vol files I have seen
        # name is a char* but is actually used as an offset into the string array that gets generated
        fileDescriptors.append({
            "id": 0,
            "name": nameOffset,
            "offset": currentOffset,
            "size": len(rawData),
            "compression": 0,
            "data": rawData
        })
        currentOffset += len(rawData) + struct.calcsize(mainHeaderFmt)
        nameOffset += len(file) + len(b"\0")
        if len(rawData) % 4 != 0:
            currentOffset += 2
            rawData.extend(b"\0\0")

    combinedFileData = bytearray()
    for descriptor in fileDescriptors:
        fileHeader = (b"VBLK", descriptor["size"])
        fileHeader = bytearray(struct.pack(mainHeaderFmt, *fileHeader))
        (rawByte) = struct.unpack("<B", b"\x80")
        fileHeader[-1] = rawByte[0]
        combinedFileData.extend(fileHeader)
        combinedFileData.extend(descriptor["data"])

    fileInfoHeader = (b"voli", struct.calcsize(fileInfoFmt) * len(fileDescriptors))
    fileInfoData = bytearray(struct.pack(mainHeaderFmt, *fileInfoHeader))
    for descriptor in fileDescriptors:
        fileInfo = (descriptor["id"], descriptor["name"], descriptor["offset"], descriptor["size"],
                    descriptor["compression"])
        fileInfoData.extend(struct.pack(fileInfoFmt, *fileInfo))

    header = (b"PVOL", currentOffset)
    stringSection = (b"vols", len(rawBytes))
    rawHeader = bytearray(struct.pack("<4sL", *stringSection))
    rawHeader.extend(rawBytes)

    with open(importFilename + ".new.vol", "wb") as outputFile:
        outputFile.write(struct.pack(mainHeaderFmt, *header))
        outputFile.write(combinedFileData)
        outputFile.write(rawHeader)
        outputFile.write(fileInfoData)



