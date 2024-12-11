
// import sys
// import json
// import glob

// import readWtb

// importFilenames = sys.argv[1:]

// for importFilename in importFilenames:
//     exportFilename = importFilename.replace(".wtb", ".obj").replace(".WTB", ".obj")

//     print "reading " + importFilename
//     try:
//         with open(importFilename, "rb") as input_fd:
//             # first get the parsed shape datastructures
//             rawData = input_fd.read()
//             shape = readWtb.loadWTB(rawData)

//         with open(exportFilename,"w") as shapeFile:
//             for vertex in shape[0]:
//                 shapeFile.write("v " + str(float(vertex[0])) + " " + str(float(vertex[1])) + " " + str(float(vertex[2])) + "\n")

//             for polygon in shape[1]:
//                 shapeFile.write("f ")
//                 for value in polygon:
//                     shapeFile.write(str(value + 1) + " ")
//                 shapeFile.write("\n")
//     except Exception as e:
//         print e
