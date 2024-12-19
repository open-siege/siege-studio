#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::bnd
{
  namespace endian = siege::platform;

  constexpr static auto body_tag = platform::to_tag<4>("BODY");
  constexpr static auto tmds_tag = platform::to_tag<4>("TMDS");
  constexpr static auto vram_tag = platform::to_tag<4>("VRAM");
  constexpr static auto tims_tag = platform::to_tag<4>("TIMS");

  bool is_bnd(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag{};
    std::array<std::byte, 4> second_tag{};

    stream.read((char*)&tag, sizeof(tag));
    stream.seekg(sizeof(tag));
    stream.read((char*)&second_tag, sizeof(second_tag));
    return tag == body_tag && second_tag == tmds_tag;
  }
}// namespace siege::content::bnd

// import sys
// import json
// import struct
// import glob

// from collections import namedtuple

// importFilenames = sys.argv[1:]

// for importFilename in importFilenames:
//     exportFilename = importFilename.replace(".bnd", ".obj").replace(".BND", ".obj")
//     print "reading " + importFilename
//     try:
//         with open(importFilename, "rb") as input_fd:
//             rawData = input_fd.read()
//         bndHeaderFmt = "<4sL4s2L4sL"
//         offset = 0
//         objectFmt = "<4L3L2L"
//         primitiveFmt = "<6H12B"
//         primitiveFmt2 = "<20B"
//         primitiveFmt3 = "<24B"
//         vertexFmt = "<4h"
//         normalFmt = "<4h"
//         (fileHeader, fileLength, dataHeader, num1, num2, tmdHeader, tmdLength) = struct.unpack_from(bndHeaderFmt, rawData, offset)
//         offset += struct.calcsize(bndHeaderFmt)
//         objects = []
//         for i in range(1):
//             verts = []
//             normals = []
//             primitives = []

//             (topVert, numVerts, topNormal, numNormals, unk1, unk2, unk3, topPrimitive, numPrimitives) = struct.unpack_from(objectFmt, rawData, offset)
//             originalOffset = offset
//             offset += struct.calcsize(objectFmt)
//             offset = originalOffset + topVert
//             for x in range(numVerts):
//                 vert = struct.unpack_from(vertexFmt, rawData, offset)
//                 offset += struct.calcsize(vertexFmt)
//                 verts.append(vert)

//             offset = originalOffset + topNormal
//             for x in range(numNormals):
//                 normal = struct.unpack_from(normalFmt, rawData, offset)
//                 offset += struct.calcsize(normalFmt)
//                 normals.append(normal)
//             #offset += topPrimitive
//             remainingBytes = tmdLength - offset + topPrimitive
//             for x in range(numPrimitives):
//                 fmt = primitiveFmt2
//                 if x > 2:
//                     fmt = primitiveFmt3
//                 prim = struct.unpack_from(fmt, rawData, offset)
//                 offset += struct.calcsize(fmt)
//                 print prim
//                 primitives.append(prim)
//             objects.append((verts, normals, primitives))
//         print offset

//         with open(exportFilename,"w") as shapeFile:
//             faceIndex = 0
//             for index, object in enumerate(objects):
//                 shapeFile.write("o " + str(index))
//                 shapeFile.write("\n")
//                 verts, normals, faces = object
//                 for vertex in verts:
//                     shapeFile.write("\tv " + str(float(vertex[0])) + " " + str(float(vertex[1])) + " " + str(float(vertex[2])) + "\n")

//                 for polygon in faces:
//                     shapeFile.write("\tf ")
//                     shapeFile.write(str(polygon[0] + 1 + faceIndex) + " ")
//                     shapeFile.write(str(polygon[1] + 1 + faceIndex) + " ")
//                     shapeFile.write(str(polygon[2] + 1 + faceIndex) + " ")
//                     shapeFile.write("\n")


//                 faceIndex += len(object[0])

//     except Exception as e:
//         print e
