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

  inline std::optional<std::filesystem::path> get_stream_path(std::ios& stream)
  {
    if (auto* ofstream = dynamic_cast<ofstream_with_path*>(&stream); ofstream)
    {
      return ofstream->path;
    }

    if (auto* ifstream = dynamic_cast<ofstream_with_path*>(&stream); ifstream)
    {
      return ifstream->path;
    }

    if (auto* fstream = dynamic_cast<fstream_with_path*>(&stream); fstream)
    {
      return fstream->path;
    }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    if (auto* ispan = dynamic_cast<std::ispanstream*>(&stream); ispan)
    {
      auto span = ispan->span();
      auto view = win32::file_view((void*)span.data());

      return view.GetMappedFilename();
    }

    if (auto* ospan = dynamic_cast<std::ospanstream*>(&stream); ospan)
    {
      auto span = ospan->span();
      auto view = win32::file_view((void*)span.data());

      return view.GetMappedFilename();
    }

    if (auto* span = dynamic_cast<std::spanstream*>(&stream); span)
    {
      auto view = win32::file_view((void*)span->span().data());

      return view.GetMappedFilename();
    }
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
      return span->span().size();
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


  inline std::unique_ptr<std::istream> shallow_clone(std::istream& stream)
  {
    auto filename = get_stream_path(stream);
    auto offset = stream.tellg();

    if (filename)
    {
      auto result = std::unique_ptr<std::istream>(new std::ifstream(*filename, std::ios::binary));
      result->seekg(offset);

      return result;
    }

    if (auto* ispan = dynamic_cast<std::ispanstream*>(&stream); ispan)
    {
      auto result = std::unique_ptr<std::istream>(new std::ispanstream(ispan->span()));
      result->seekg(offset);
      return result;
    }

    if (auto* span = dynamic_cast<std::spanstream*>(&stream); span)
    {
      auto result = std::unique_ptr<std::istream>(new std::spanstream(span->span()));
      result->seekg(offset);
      return result;
    }

    // TODO wrap the stream_buf so that each stream can have its own independent pointer into the data
    return std::unique_ptr<std::istream>(new std::istream(stream.rdbuf()));
  }
}// namespace siege::platform

#endif// OPEN_SIEGE_STREAM_HPP
