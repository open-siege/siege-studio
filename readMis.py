from collections import namedtuple
import struct
import json
import sys

importFilenames = sys.argv[1:]

with open(importFilenames[0], "rb") as input_fd:
    rawData = input_fd.read()

skipExtra = {
    b"ESpt",
    b"SIMG",
    b"HERC",
    b"TANK"
}

def readGroupString(rawData, offset, index):
    (offset, result) = readString(rawData, offset)

    if (index == 0 and result == ""):
        (offset, result) = readString(rawData, offset)
    return (offset, result)

def readString(rawData, offset):
    stringStruct = "<B"
    terminatorStruct = "<s"
    (length, ) = struct.unpack_from(stringStruct, rawData, offset)
    offset += struct.calcsize(stringStruct)
    if length == 0:
        return (offset, "")

    finalStruct = "<" + str(length) + "s"
    (someString,) = struct.unpack_from(finalStruct, rawData, offset)

    offset += struct.calcsize(finalStruct)

    if offset != len(rawData):
        (terminator, ) = struct.unpack_from(terminatorStruct, rawData, offset)

        if (terminator == b"\0"):
            offset += struct.calcsize(terminatorStruct)

    return (offset, someString)


object = "<4s2L"

def readSimVol(objectTypes, rawData, offset):
    simVol = "<4s7L"
    header = struct.unpack_from(simVol, rawData, offset)
    offset += struct.calcsize(simVol)
    (offset, volumeString) = readString(rawData, offset)
    print("SimVol in use is " + volumeString.decode("utf8"))
    return (offset, volumeString)

def readSimGroup(objectTypes, rawData, offset):
    simGroup = "<4s3L"
    header = struct.unpack_from(simGroup, rawData, offset)

    offset += struct.calcsize(simGroup)
    children = []
    i = 0
    while i < header[-1]:
        objectInfo = struct.unpack_from(object, rawData, offset)
        print("Unpacked a " + objectInfo[0].decode("utf8"))
        children.append(objectInfo)
        if objectInfo[0] in objectTypes:
            (newOffset, moreChildren) = objectTypes[objectInfo[0]](objectTypes, rawData, offset)
            offset = newOffset
        else:
            offset += objectInfo[1] + 8
            if rawData[offset] == 0 and objectInfo[0] in skipExtra:
                offset += 1
        i += 1

    i = 0
    print("This group has " + str(header[-1]) + " children")
    while i < header[-1]:
        (newOffset, someString) = readGroupString(rawData, offset, i)
        offset = newOffset
        if someString == "":
            break
        i += 1
        print("Child: " + someString.decode("utf8"))

    return (offset, children)


objectTypes = {
    b"SIMG": readSimGroup,
    b"SVol": readSimVol
}

offset = 0

try:
    print(readSimGroup(objectTypes, rawData, offset))
except Exception as e:
    print(e)

