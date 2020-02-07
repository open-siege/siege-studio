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
    for file in filesToPack:
        rawBytes.extend(file.encode("utf-8") + b"\0")

    header = (b"vols", len(rawBytes))
    rawHeader = bytearray(struct.pack("<4sL", *header))
    rawHeader.extend(rawBytes)
    print(rawHeader)


