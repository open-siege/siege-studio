#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::wtb
{
  namespace endian = siege::platform;

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
