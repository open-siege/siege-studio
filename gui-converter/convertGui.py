from collections import namedtuple
import struct
import json
import sys

importFilenames = sys.argv[1:]

with open(importFilenames[0], "rb") as input_fd:
    rawData = input_fd.read()

knownTags = {
    b"SGct": "Control",
    b"SGAc": "ArrayControl",
    b"SGtc": "TestControl",
    b"SGbm": "BitmapControl",
    b"SGpl": "PaletteControl",
    b"SGac": "ActiveControl",
    b"SGst": "SimpleText",
    b"SGtb": "TestButton",
    b"SGtr": "TestRadial",
    b"SGtk": "TestCheck",
    b"SGts": "TSControl",
    b"SGte": "TextEdit",
    b"SGtm": "TimerControl",
    b"SGbb": "BitmapBox",
    b"SGtl": "TextList",
    b"SGtw": "TextWrap",
    b"SGtf": "TextFormat",
    b"SGsl": "Slider",
    b"SGcb": "ComboBox",
    b"SGsc": "ScrollControl",
    b"SGsC": "ScrollContentControl",
    b"SGmc": "MatrixControl",
}

offset = 0

try:
    #TODO actually start writing some real logic
    print("Hello GUI")
except Exception as e:
    print(e, offset)

