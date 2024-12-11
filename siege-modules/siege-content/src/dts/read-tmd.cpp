
// import sys
// import json
// import struct
// import glob

// from collections import namedtuple

// importFilenames = sys.argv[1:]

// with open('tmdStructures.json') as f:
//     structures = json.load(f)

// for importFilename in importFilenames:
//     exportFilename = importFilename.replace(".tmd", ".obj").replace(".TMD", ".obj")
//     print "reading " + importFilename
//     try:
//         with open(importFilename, "rb") as input_fd:
//             rawData = input_fd.read()

//         offset = 0
//         headerFmt = "<3L"
//         objectFmt = "<6Ll"
//         primitiveFmt = "<4B"
//         vertexFmt = "<4h"
//         normalFmt = "<4h"
//         (header, flags, numObjects) = struct.unpack_from(headerFmt, rawData, offset)

//         objects = []
//         for i in range(numObjects):
//             verts = []
//             normals = []
//             primitives = []
//             offset += struct.calcsize(headerFmt)
//             (topVert, numVerts, topNormal, numNormals, topPrimitive, numPrimitives, scale) = struct.unpack_from(objectFmt, rawData, offset)
//             originalOffset = offset
//             offset += struct.calcsize(objectFmt)
//             print (numVerts, numNormals, numPrimitives)
//             for x in range(numVerts):
//                 vert = struct.unpack_from(vertexFmt, rawData, offset)
//                 offset += struct.calcsize(vertexFmt)
//                 print vert
//                 verts.append(vert)

//             for x in range(numNormals):
//                 normal = struct.unpack_from(normalFmt, rawData, offset)
//                 offset += struct.calcsize(normalFmt)
//                 #print normal
//                 normals.append(normal)

//             for x in range(numPrimitives):
//                 primitive = struct.unpack_from(primitiveFmt, rawData, offset)
//                 offset += struct.calcsize(primitiveFmt)
//                 bodyFmt = structures[str(primitive[0])][str(primitive[1])]
//                 body = struct.unpack_from(bodyFmt, rawData, offset)
//                 offset += struct.calcsize(bodyFmt)
//                 primitives.append((primitive, body))
//                 print (primitive, body)
//             objects.append((verts, normals, primitives))

//         with open(exportFilename,"w") as shapeFile:
//             faceIndex = 0
//             for index, object in enumerate(objects):
//                 shapeFile.write("o " + str(index))
//                 shapeFile.write("\n")
//                 verts, normals, faces = object
//                 for vertex in verts:
//                     shapeFile.write("\tv ")
//                     shapeFile.write(str(float(vertex[0])) + " ")
//                     shapeFile.write(str(float(vertex[1])) + " ")
//                     shapeFile.write(str(float(vertex[2])) + " ")
//                     shapeFile.write("\n")

//                 for header, polygon in faces:
//                     shapeFile.write("\tf ")
//                     shapeFile.write(str(polygon[5] + 1 + faceIndex) + " ")
//                     shapeFile.write(str(polygon[6] + 1 + faceIndex) + " ")
//                     shapeFile.write(str(polygon[7] + 1 + faceIndex) + " ")
//                     shapeFile.write("\n")
//                     shapeFile.write("\tf ")
//                     shapeFile.write(str(polygon[6] + 1 + faceIndex) + " ")
//                     shapeFile.write(str(polygon[7] + 1 + faceIndex) + " ")
//                     shapeFile.write(str(polygon[8] + 1 + faceIndex) + " ")
//                     shapeFile.write("\n")
//                 faceIndex += len(verts)

//     except Exception as e:
//         print e
