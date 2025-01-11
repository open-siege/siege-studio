// WTB - MechWarrior 2 - individual mesh shape

#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/content/dts/wtb.hpp>

namespace siege::content::wtb
{
  namespace endian = siege::platform;

  constexpr auto header_tag = platform::to_tag<4>({ 'W', 'T', 'B', 'O' });

  struct wtb_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint16_t unknown1;
    endian::little_uint16_t unknown2;
    float scale;
    float translation;
    endian::little_uint32_t padding1;
    endian::little_uint32_t padding2;
    endian::little_uint16_t vertex_count;
    endian::little_uint16_t surface_count;
    endian::little_uint32_t unknown3;
  };

  bool is_wtb(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));
    return tag == header_tag;
  }

  wtb_shape load_wtb(std::istream& stream)
  {
    wtb_shape result;

    wtb_header header;
    stream.read((char*)&header, sizeof(header));

    if (header.tag != header_tag)
    {
      return result;
    }

    result.scale = header.scale;
    result.translation = header.translation;
    result.vertices.resize(header.vertex_count);
    stream.read((char*)result.vertices.data(), result.vertices.size() * sizeof(packed_vertex));

    result.faces.reserve(header.surface_count);

    surface_header face_header;
    for (auto i = 0u; i < header.surface_count; ++i)
    {
      stream.read((char*)&face_header, sizeof(face_header));

      if (face_header.number_of_points < 1 || face_header.number_of_points > 7)
      {
        break;
      }

      auto& surface = result.faces.emplace_back();
      surface.header = face_header;

      if (face_header.number_of_points <= 4)
      {
        face_4v face;
        stream.read((char*)&face, sizeof(face));
        surface.values.resize(face_header.number_of_points);
        std::memcpy(surface.values.data(), face.data(), sizeof(face[0]) * surface.values.size());
      }
      else if (face_header.number_of_points <= 7)
      {
        face_7v face;
        stream.read((char*)&face, sizeof(face));
        surface.values.resize(face_header.number_of_points);
        std::memcpy(surface.values.data(), face.data(), sizeof(face[0]) * surface.values.size());
      }
      else
      {
        break;
      }
    }

    return result;
  }
}// namespace siege::content::wtb

// from collections import namedtuple
// import struct

// def loadWTB(rawData):
//     offset = 0
//     # Do not change polygon winding
//     flipPolygons = False
//     # Magic numbers eat away at my soul
//     pointRecordFmt = "<4l"
//     pointBaseOffset = 32
//     polyHeaderFmt = "<2H"
//     shortPolyFmt = "<6H"
//     longPolyFmt = "<9H"
//     longPolyRecordVertices = 5
//     fileHeader =  struct.unpack_from(">4s", rawData, offset)[0]
//     polygons = []
//     points = []
//     if len(rawData) > 32 and fileHeader == "WTBO":
//         offset = 24 #TODO why do we skip so far into the file?
//         pointCount, polygonCount = struct.unpack_from("<2H", rawData, offset)
//         print (pointCount, polygonCount)
//          # Read points
//         offset = pointBaseOffset
//         for i in range(pointCount):
//             points.append(struct.unpack_from(pointRecordFmt, rawData, offset))
//             offset += struct.calcsize(pointRecordFmt)
//          # Read polygons

//         print (pointCount, offset)
//         while len(polygons) < polygonCount:
//             polygon = struct.unpack_from(shortPolyFmt, rawData, offset)
//             someNumber = polygon[0]
//             count = polygon[1]
//             fmt = shortPolyFmt
//             if count >= longPolyRecordVertices:
//                 polygon = struct.unpack_from(longPolyFmt, rawData, offset)
//                 fmt = longPolyFmt
//             faceFormat = ">" + str(count) + "H"
//             if count > 0:
//                 polygons.append(polygon[2:][:count])

//             offset += struct.calcsize(fmt)


//     return (points, polygons)
