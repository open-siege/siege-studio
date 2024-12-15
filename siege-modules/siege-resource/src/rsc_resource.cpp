#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/platform/resource.hpp>
#include <siege/resource/rsc_resource.hpp>

namespace siege::resource::rsc
{
  namespace endian = siege::platform;

  // actually the number of files in the file
  constexpr static auto rsc_v1_tag = platform::to_tag<4>({ 0xf6, 0x01, 0x00, 0x00 });
  constexpr static auto rsc_v2_tag = platform::to_tag<4>({ 0x2e, 0x02, 0x00, 0x00 });
  constexpr static auto rsc_v3_tag = platform::to_tag<4>({ 0xf8, 0x5e, 0x00, 0x00 });

  struct rsc_v1_file_entry
  {
    std::array<char, 16> path;
    endian::little_uint32_t offset;
    std::array<char, 12> padding;
  };

  struct rsc_v2_file_entry
  {
    std::array<char, 16> path;
    endian::little_uint32_t offset;
  };

  struct rsc_v3_group_entry
  {
    endian::little_uint32_t file_name_entry_offset;
    endian::little_uint32_t num_files;
  };

  struct rsc_v3_name_entry
  {
    std::array<char, 16> path;
    endian::little_uint16_t size_entry_index;
    std::byte padding;
    std::uint8_t group_entry_index;
  };

  struct rsc_v3_size_entry
  {
    endian::little_uint32_t file_data_offset;
    endian::little_uint32_t group_entry_index;
  };

  bool rsc_resource_reader::is_supported(std::istream& stream)
  {
    auto path = siege::platform::get_stream_path(stream);

    if (path)
    {
      return path->extension() == ".rsc" || path->extension() == ".RSC";
    }

    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    return tag == rsc_v1_tag || tag == rsc_v2_tag || tag == rsc_v3_tag;
  }

  bool rsc_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<rsc_resource_reader::content_info> rsc_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<rsc_resource_reader::content_info> results;

    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    std::optional<int> version{};

    results.reserve(128);

    endian::little_uint32_t file_count;
    std::memcpy(&file_count, tag.data(), sizeof(file_count));


    auto current_pos = stream.tellg();
    rsc_v1_file_entry first{};
    stream.read((char*)&first, sizeof(first));

    if ((std::isalpha(first.path[0]) || std::isdigit(first.path[0])) && (std::isalpha(first.path[1]) || std::isdigit(first.path[1])))
    {
      if (std::isalpha(first.padding[0]) || std::isdigit(first.padding[0]))
      {
        stream.seekg((std::size_t)current_pos + sizeof(rsc_v2_file_entry), std::ios::beg);
        auto& entry = std::get<rsc_resource_reader::file_info>(results.emplace_back(rsc_resource_reader::file_info{}));
        entry.archive_path = query.archive_path;
        entry.folder_path = query.archive_path;
        entry.filename = first.path.data();
        entry.offset = first.offset;
        version = 2;
      }
      else
      {
        auto& entry = std::get<rsc_resource_reader::file_info>(results.emplace_back(rsc_resource_reader::file_info{}));
        entry.archive_path = query.archive_path;
        entry.folder_path = query.archive_path;
        entry.filename = first.path.data();
        entry.offset = first.offset;
        version = 1;
      }
    }
    else
    {
      stream.seekg((std::size_t)current_pos, std::ios::beg);
      version = 3;
    }


    if (version == 1)
    {
      for (auto i = 0; i < file_count; ++i)
      {
        rsc_v1_file_entry next{};
        stream.read((char*)&next, sizeof(next));

        auto& previous = std::get<rsc_resource_reader::file_info>(results.back());

        if (next.path[0] != '\0')
        {
          auto& entry = std::get<rsc_resource_reader::file_info>(results.emplace_back(rsc_resource_reader::file_info{}));
          entry.archive_path = query.archive_path;
          entry.folder_path = query.archive_path;
          entry.filename = next.path.data();
          entry.offset = next.offset;
        }

//        previous.size = next.offset - previous.offset - 2;
        previous.size = next.offset - previous.offset;
      }
    }
    else if (version == 2)
    {
      for (auto i = 0; i < file_count; ++i)
      {
        rsc_v2_file_entry next{};
        stream.read((char*)&next, sizeof(next));

        auto& previous = std::get<rsc_resource_reader::file_info>(results.back());

        if (next.path[0] != '\0')
        {
          auto& entry = std::get<rsc_resource_reader::file_info>(results.emplace_back(rsc_resource_reader::file_info{}));

          entry.archive_path = query.archive_path;
          entry.folder_path = query.archive_path;
          entry.filename = next.path.data();
          entry.offset = next.offset;
        }

        previous.size = next.offset - previous.offset;
      }
    }
    else if (version == 3)
    {
      endian::little_uint32_t header_size;
      std::memcpy(&header_size, tag.data(), sizeof(header_size));
      std::array<rsc_v3_group_entry, 16> groups;
      stream.read((char*)groups.data(), sizeof(rsc_v3_group_entry) * groups.size());

      header_size = header_size - sizeof(header_size) - sizeof(rsc_v3_group_entry) * groups.size();

      auto file_count = header_size / sizeof(rsc_v3_name_entry);

      std::vector<rsc_v3_name_entry> entries;
      entries.resize(file_count);
      stream.read((char*)entries.data(), sizeof(rsc_v3_name_entry) * entries.size());
    }

    return results;
  }

  void rsc_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void rsc_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::rsc
//

// import sys
// import struct
// import os

// from collections import namedtuple

// def readFiles(numFiles, infoFmt, offset, adjustExtension):
// 	files = []
// 	for i in range(numFiles):
// 		rawValues = struct.unpack_from(infoFmt, rawData, offset)
// 		fileName = rawValues[0]
// 		fileOffset = rawValues[1]
// 		fileName = fileName.split("\0".encode("ascii"))[0]
// 		offset += struct.calcsize(infoFmt)
// 		if adjustExtension:
// 			fileName = fileName.replace("_", ".")
// 		files.append((fileName, fileOffset))

// 	return files


// importFilenames = sys.argv[1:]

// for importFilename in importFilenames:

// 	if not os.path.exists("extracted"):
// 		os.makedirs("extracted")

// 	print("reading " + importFilename)
// 	try:
// 		with open(importFilename, "rb") as input_fd:
// 			rawData = input_fd.read()
// 		# if there is a CREDITS_PAL, it's the Colony Wars Red Run RSC
// 		if rawData.find("CREDITS_PAL".encode("ascii")) != -1:
// 			# Can't find where the total number of files are in the file
// 			# and this is so bad :(
// 			numFiles = 1209
// 			offset = rawData.find("CREDITS_PAL".encode("utf-8"))
// 			infoFmt = "<16sl"
// 			tempFiles = readFiles(numFiles, infoFmt, offset, True)
// 			offset += numFiles * struct.calcsize(infoFmt)

// 			moreInfoFmt = "<2L"
// 			# This is even worse.
// 			# I would love to figure out why we jump so far
// 			# This is the starting point of the file offsets
// 			# And still might be wrong
// 			offset = 27592
// 			files = []
// 			for i in range(numFiles):
// 				fileInfo = struct.unpack_from(moreInfoFmt, rawData, offset)
// 				offset += struct.calcsize(moreInfoFmt)
// 				files.append((tempFiles[i][0], fileInfo[1]))

// 			print(files[0], files[1])
// 		else:
// 			offset = 0
// 			(numFiles, ) = struct.unpack_from("<L", rawData, offset)
// 			offset += struct.calcsize("<L")
// 			infoFmt = "<16s2l4h"
// 			files = readFiles(numFiles, infoFmt, offset, False)

// 			# check if the second filename has been parsed properly
// 			if ".".encode("utf-8") not in files[1][0]:
// 				infoFmt = "<16sl"
// 				files = readFiles(numFiles, infoFmt, offset, False)

// 		for index, file in enumerate(files):
// 			filename, fileOffset = file
// 			print("extracting " + filename.decode("utf-8") + " " + str(fileOffset))
// 			nextFileOffset = len(rawData)
// 			if index + 1 < numFiles:
// 				nextFilename, nextFileOffset = files[index + 1]
// 			with open("extracted/" + filename.decode("utf-8"), "wb") as shapeFile:
// 				newFileByteArray = bytearray(rawData[fileOffset:nextFileOffset])
// 				shapeFile.write(newFileByteArray)

// 	except Exception as e:
// 		print(e)
