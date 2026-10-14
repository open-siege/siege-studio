// WTB - MechWarrior 2 - individual mesh shape
export module siege.content.wtb;

import std;

import siege.platform.endian;
import siege.platform.shared;
import siege.platform.stream;

namespace siege::content::wtb
{
  namespace endian = siege::platform;

  // each tag must represent a type of shading or texturing
  export enum polygon_type : std::uint8_t {
    tag1 = 0x10,
    tag2 = 0x14,
    tag3 = 0x18,
    tag4 = 0x19,
    tag5 = 0x1c,
    tag6 = 0x47,
    tag7 = 0x44,
    tag8 = 0x49,
    tag9 = 0x4c,
    tag10 = 0x4d,
    tag11 = 0x4e,
    tag12 = 0x54,
    tag13 = 0x58,
    tag14 = 0x64,
    tag15 = 0x68,
    tag16 = 0x70,
    tag17 = 0x74,
    tag18 = 0x78,
    tag19 = 0x7d,
    tag20 = 0x7f,
  };

  export struct packed_vertex
  {
    endian::little_int32_t x;
    endian::little_int32_t y;
    endian::little_int32_t z;
    endian::little_uint16_t normal_index;
    endian::little_uint16_t unknown;
  };

  export struct surface_header
  {
    std::byte unknown;
    polygon_type type;
    endian::little_uint16_t number_of_points;
  };

  export using face_4v = std::array<endian::little_uint16_t, 4>;
  export using face_7v = std::array<endian::little_uint16_t, 7>;

  export struct surface
  {
    surface_header header;
    std::vector<endian::little_uint16_t> values;
  };

  export struct wtb_shape
  {
    float scale;
    float translation;

    std::vector<packed_vertex> vertices;
    std::vector<surface> faces;
  };
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

  export bool is_wtb(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));
    return tag == header_tag;
  }

  export wtb_shape load_wtb(std::istream& stream)
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
