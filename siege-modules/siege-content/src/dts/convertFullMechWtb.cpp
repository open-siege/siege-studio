
// import sys
// import json
// import glob

// import readWtb

// with open("mw2-mechs.json") as f:
//     mechs = json.load(f)

// importPath = "./WTB/"

// def readFile(part, parts):
//     objectName = part["@mw2prj"].replace(".WTB", "")
//     importFilename = importPath + part["@mw2prj"]
//     print "reading " + importFilename
//     try:
//         with open(importFilename, "rb") as input_fd:
//             # first get the parsed shape datastructures
//             rawData = input_fd.read()
//             shape = readWtb.loadWTB(rawData)
//             parts[objectName] = (part, shape)
//     except:
//         print "could not open " + importFilename
//         raise ValueError("could not open " + importFilename)

// for mech in mechs:
//     exportFilename = mech["@name"] + ".obj"
//     parts = {}
//     try:
//         for key, part in mech.iteritems():
//             if "@mw2prj" in part:
//                 readFile(part, parts)
//             else:
//                 for partItem in part:
//                     if "@mw2prj" in partItem:
//                         readFile(partItem, parts)

//         with open(exportFilename,"w") as shapeFile:
//             faceOffset = 0
//             for partName, partData in parts.iteritems():
//                 part, shape = partData
//                 shapeFile.write("o " + partName + "\r\n")
//                 numVerts = 0
//                 for vertex in shape[0]:
//                     numVerts += 1
//                     x = float(vertex[0])
//                     y = float(vertex[1])
//                     z = float(vertex[2])
//                     if "@tx" in part:
//                         x += float(part["@tx"])
//                     if "@ty" in part:
//                         y += float(part["@ty"])
//                     if "@tz" in part:
//                         z += float(part["@tz"])
//                     shapeFile.write("\tv " + str(x) + " " + str(y) + " " + str(z) + "\r\n")

//                 for polygon in shape[1]:
//                     shapeFile.write("\tf ")

//                     for value in polygon:
//                         shapeFile.write(str(value + faceOffset + 1) + " ")
//                     shapeFile.write("\r\n")
//                 faceOffset += numVerts
//     except Exception as e:
//         print e
