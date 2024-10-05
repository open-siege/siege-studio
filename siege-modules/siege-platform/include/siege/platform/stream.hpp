#ifndef OPEN_SIEGE_STREAM_HPP
#define OPEN_SIEGE_STREAM_HPP

#include <istream>
#include <fstream>
#include <ostream>
#include <optional>
#include <vector>
#include <filesystem>
#include <spanstream>

#if WIN32
#include <siege/platform/win/core/file.hpp>
#endif

namespace siege::platform
{
  inline std::istream& read(std::istream& stream, char* data, std::size_t size)
  {
    return stream.read(data, size);
  }

  inline std::istream& read(std::istream& stream, std::byte* data, std::size_t size)
  {
    return stream.read(reinterpret_cast<char*>(data), size);
  }

  inline std::ostream& write(std::ostream& stream, const std::byte* data, std::size_t size)
  {
    return stream.write(reinterpret_cast<const char*>(data), size);
  }

  inline std::ostream& write(std::ostream& stream, const char* data, std::size_t size)
  {
    return stream.write(data, size);
  }

  struct ofstream_with_path : std::ofstream
  {
    std::filesystem::path path;

    ofstream_with_path(std::filesystem::path path, std::ios_base::openmode mode) : std::ofstream(path, mode), path(std::move(path))
    {
    }
  };

  struct ifstream_with_path : std::ifstream
  {
    std::filesystem::path path;

    ifstream_with_path(std::filesystem::path path, std::ios_base::openmode mode) : std::ifstream(path, mode), path(std::move(path))
    {
    }
  };

  struct fstream_with_path : std::fstream
  {
    std::filesystem::path path;

    fstream_with_path(std::filesystem::path path, std::ios_base::openmode mode) : std::fstream(path, mode), path(std::move(path))
    {
    }
  };

  struct vectorstream : std::spanstream
  {
    std::vector<char> data;

    vectorstream(std::vector<char> data) : std::spanstream(data), data(std::move(data))
    {
      this->span(this->data);
    }
  };

  inline std::optional<std::filesystem::path> get_stream_path(const std::ios& stream)
  {
    if (auto* ofstream = dynamic_cast<const ofstream_with_path*>(&stream); ofstream)
    {
      return ofstream->path;
    }

    if (auto* ifstream = dynamic_cast<const ifstream_with_path*>(&stream); ifstream)
    {
      return ifstream->path;
    }

    if (auto* fstream = dynamic_cast<const fstream_with_path*>(&stream); fstream)
    {
      return fstream->path;
    }

#if WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    if (auto* ispan = dynamic_cast<const std::ispanstream*>(&stream); ispan)
    {
      auto span = ispan->span();
      auto view = win32::file_view((void*)span.data());

      auto result = view.GetMappedFilename();
      view.release();
      return result;
    }

    if (auto* ospan = dynamic_cast<const std::ospanstream*>(&stream); ospan)
    {
      auto span = ospan->span();
      auto view = win32::file_view((void*)span.data());

      auto result = view.GetMappedFilename();
      view.release();
      return result;
    }

    if (auto* span = dynamic_cast<const std::spanstream*>(&stream); span)
    {
      auto view = win32::file_view((void*)span->span().data());
      auto result = view.GetMappedFilename();
      view.release();
      return result;
    }
#endif
#endif

    return std::nullopt;
  }

  inline std::size_t get_stream_size(std::ios& stream)
  {
    if (auto* ofstream = dynamic_cast<ofstream_with_path*>(&stream); ofstream)
    {
      return std::filesystem::file_size(ofstream->path);
    }

    if (auto* ifstream = dynamic_cast<ofstream_with_path*>(&stream); ifstream)
    {
      return std::filesystem::file_size(ifstream->path);
    }

    if (auto* fstream = dynamic_cast<fstream_with_path*>(&stream); fstream)
    {
      return std::filesystem::file_size(fstream->path);
    }

    if (auto* ispan = dynamic_cast<std::ispanstream*>(&stream); ispan)
    {
      return ispan->span().size();
    }

    if (auto* ospan = dynamic_cast<std::ospanstream*>(&stream); ospan)
    {
      return ospan->span().size();
    }

    if (auto* span = dynamic_cast<std::spanstream*>(&stream); span)
    {
      if (auto size = span->span().size(); size)
      {
        return size;
      }
      return span->rdbuf()->in_avail();
    }

    if (auto* istream = dynamic_cast<std::istream*>(&stream); istream)
    {
      auto current_pos = istream->tellg();
      istream->seekg(0, std::ios::end);
      auto size = istream->tellg();
      istream->seekg(current_pos, std::ios::beg);
      return size;
    }

    if (auto* ostream = dynamic_cast<std::ostream*>(&stream); ostream)
    {
      auto current_pos = ostream->tellp();
      ostream->seekp(0, std::ios::end);
      auto size = ostream->tellp();
      ostream->seekp(current_pos, std::ios::beg);
      return size;
    }

    return 0;
  }
  struct storage_info
  {
    enum info_type
    {
      null,
      file,
      buffer
    } type;
    union
    {
      const std::filesystem::path::value_type* path;
      struct
      {
        std::size_t size;
        std::byte* data;
      } data;
    } info;
  };

  inline std::unique_ptr<std::istream> create_istream(storage_info& info, bool owning = false)
  {
    if (info.type == storage_info::file && info.info.path)
    {
      return std::make_unique<ifstream_with_path>(info.info.path, std::ios_base::in | std::ios_base::binary);
    }

    if (info.type == storage_info::buffer && !owning && info.info.data.data)
    {
      std::span temp((char*)info.info.data.data, info.info.data.size);
      return std::make_unique<std::ispanstream>(temp, std::ios_base::in | std::ios_base::binary);
    }

    if (info.type == storage_info::buffer && owning && info.info.data.data)
    {
      std::string temp((char*)info.info.data.data, info.info.data.size);
      return std::make_unique<std::istringstream>(std::move(temp), std::ios_base::in | std::ios_base::binary);
    }

    return std::make_unique<std::istringstream>();
  }

}// namespace siege::platform

#endif// OPEN_SIEGE_STREAM_HPP
