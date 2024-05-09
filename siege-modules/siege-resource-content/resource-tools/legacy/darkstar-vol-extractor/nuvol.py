import sys
import struct
import json
import glob
import binascii
from os import listdir, rename
from os.path import isfile, join, exists, split

importFolders = []

for importFolder in sys.argv[1:]:
    files = glob.glob(importFolder)
    importFolders.extend(files)

for importFolder in importFolders:
    importFolderVolJson = None
    # if a glob for .vol.json is used, take the directory part of it
    if importFolder.endswith(".vol.json") or importFolder.endswith(".VOL.json"):
        importFolder = split(importFolder)
        importFolderVolJson = importFolder[-1]
        importFolder = importFolder[:-1]
        importFolder = join("", *importFolder)
    else:
        # we want the directory name so that we can specify what the vol json filename should be
        importFolderVolJson = split(importFolder)[-1] + ".vol.json"

    print("packing " + importFolder)

    # thank you stackoverflow - https://stackoverflow.com/questions/3207219/how-do-i-list-all-files-of-a-directory
    filesToPack = [f for f in listdir(importFolder) if isfile(join(importFolder, f))]

    finalVol = importFolder + ".vol"
    finalVolOld = importFolder + ".vol.old"
    volumeHeader = binascii.hexlify(b"PVOL").decode("utf8")

    if importFolderVolJson in filesToPack:
        filesToPack.remove(importFolderVolJson)
        with open(join(importFolder, importFolderVolJson), "r") as volumeFile:
            parsedInfo = json.loads(volumeFile.read())
            if "volumeHeader" in parsedInfo:
                volumeHeader = parsedInfo["volumeHeader"]
            filesToPack = parsedInfo["files"]

    volumeHeader = binascii.unhexlify(volumeHeader)

    stringBuffer = []
    fileDescriptors = []
    mainHeaderFmt = "<4sL"
    fileInfoFmt = "<4LB"
    startOffset = struct.calcsize(mainHeaderFmt)
    currentOffset = startOffset
    nameOffset = 0

    for file in filesToPack:
        print("reading " + join(importFolder, file))
        with open(join(importFolder, file), "rb") as outputFile:
            rawData = bytearray(outputFile.read())
        stringBuffer.extend(file.encode("utf-8") + b"\0")
        # id is never used in any of the vol files I have seen
        # name is a char* but is actually used as an offset into the string array that gets generated
        # we don't do any form of compression ever, because unvol can't handle it
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
        while len(rawData) % 4 != 0:
            rawData.extend(b"\0")
            currentOffset += 1

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

    header = (volumeHeader, currentOffset)

    if volumeHeader == b"PVOL":
        stringSection = (b"vols", len(stringBuffer))
        while len(stringBuffer) % 2 != 0:
            stringBuffer.extend(b"\0")

        rawHeader = bytearray(struct.pack("<4sL", *stringSection))
        rawHeader.extend(stringBuffer)
    else:
        stringSection = (b"vols", 0, b"voli", 0,  b"vols", len(stringBuffer))
        while len(stringBuffer) % 2 != 0:
            stringBuffer.extend(b"\0")

        rawHeader = bytearray(struct.pack("<4sL4sL4sL", *stringSection))
        rawHeader.extend(stringBuffer)

    if exists(finalVol) and not exists(finalVolOld):
        rename(finalVol, finalVolOld)

    with open(finalVol, "wb") as outputFile:
        outputFile.write(struct.pack(mainHeaderFmt, *header))
        outputFile.write(combinedFileData)
        outputFile.write(rawHeader)
        outputFile.write(fileInfoData)

