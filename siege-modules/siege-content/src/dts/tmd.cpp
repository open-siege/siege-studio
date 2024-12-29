#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::tmd
{
  namespace endian = siege::platform;

  struct tmd_header
  {
    endian::little_uint32_t magic_number;
    endian::little_uint32_t padding;
    endian::little_uint32_t object_count;
  };

  struct tmd_object_header
  {
    endian::little_uint32_t vertex_offset;
    endian::little_uint32_t vertex_count;
    endian::little_uint32_t normal_offset;
    endian::little_uint32_t normal_count;
    endian::little_uint32_t primitive_offset;
    endian::little_uint32_t primitive_count;
    endian::little_uint32_t padding;
  };

  struct tmd_vertex
  {
    endian::little_uint16_t x;
    endian::little_uint16_t y;
    endian::little_uint16_t z;
    endian::little_uint16_t padding;
  };

  struct tmd_primitive_header
  {
    std::byte padding;
    std::uint8_t size;
    std::byte padding2;
    enum primitive_type : std::uint8_t
    {
      single_colour_triangle = 0x20,
      single_colour_triangle_alt = 0x22,
      gouraud_triangle = 0x24,
      single_colour_quad = 0x28,
      flat_triangle = 0x30,
      textured_triangle = 0x34,
      flat_quad = 0x38,
    } type;
  };


  bool is_tmd(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    tmd_header main_header{};
    stream.read((char*)&main_header, sizeof(main_header));

    if (main_header.magic_number == 65 && main_header.object_count > 0)
    {
      tmd_object_header object_header{};
      stream.read((char*)&object_header, sizeof(object_header));

      auto computed_normal_offset = object_header.vertex_offset + (sizeof(tmd_vertex) * object_header.vertex_count);
      auto computed_primitive_offset = computed_normal_offset + (sizeof(tmd_vertex) * object_header.normal_count);

      return object_header.normal_offset == computed_normal_offset && object_header.primitive_offset == computed_primitive_offset;
    }

    return false;
  }

  void load_tmd(std::istream& stream)
  {

  }

}// namespace siege::content::tmd
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
