// WTB - MechWarrior 2 - individual mesh shape

#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::wtb
{
  namespace endian = siege::platform;

  constexpr auto header_tag = platform::to_tag<4>({ 'W', 'T', 'B', 'O' });

  // each tag must represent a type of shading or texturing
  constexpr auto polygon_tag = platform::to_tag<2>({ 0x00, 0x78 });
  constexpr auto polygon_tag_2 = platform::to_tag<2>({ 0xf0, 0x4c });
  constexpr auto polygon_tag_3 = platform::to_tag<2>({ 0x22, 0x54 });
  constexpr auto polygon_tag_4 = platform::to_tag<2>({ 0x1f, 0x74 });
  constexpr auto polygon_tag_5 = platform::to_tag<2>({ 0xf0, 0x44 });

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

  struct surface_header
  {
    std::array<std::byte, 2> tag;
    endian::little_uint16_t number_of_points;
  };

  struct face_3v
  {
    std::array<endian::little_uint16_t, 3> values;
    endian::little_uint16_t padding;
  };

  struct face_4v
  {
    std::array<endian::little_uint16_t, 4> values;
  };

  struct face_5v
  {
    std::array<endian::little_uint16_t, 5> values;
    std::array<endian::little_uint16_t, 2> padding;
  };

  using face_value = std::variant<face_3v, face_4v, face_5v>;

  struct surface
  {
    surface_header header;
    std::vector<face_value> vertices;
  };

  bool is_wtb(std::istream& stream)
  {
    return false;
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
