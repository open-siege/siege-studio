
#include <bit>
#include <filesystem>
#include <cassert>
#include <atomic>
#include <unordered_set>
#include <unordered_map>
#include <system_error>
#include <siege/platform/stream.hpp>
#include <siege/resource/resource_maker.hpp>
#include "views/vol_controller.hpp"

using namespace siege::views;
using storage_info = siege::platform::storage_info;

extern "C" {

std::errc get_supported_extensions(std::size_t count, const siege::fs_char** strings, std::size_t* fetched) noexcept
{
  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  count = std::clamp<std::size_t>(count, 0u, vol_controller::formats.size());

  std::transform(vol_controller::formats.begin(), vol_controller::formats.begin() + count, strings, [](const auto value) {
    return value.data();
  });

  if (fetched)
  {
    *fetched = count;
  }

  return std::errc(0);
}

std::errc get_supported_format_categories(std::size_t count, const char16_t** strings, std::size_t* fetched) noexcept
{
  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  static auto categories = std::array<std::u16string_view, 1>{ { u"All Archives" } };

  count = std::clamp<std::size_t>(count, 0u, categories.size());

  std::transform(categories.begin(), categories.begin() + count, strings, [](const auto value) {
    return value.data();
  });

  if (fetched)
  {
    *fetched = count;
  }

  return std::errc(0);
}

std::errc get_supported_extensions_for_category(const char16_t* category, std::size_t count, const siege::fs_char** strings, std::size_t* fetched) noexcept
{
  if (!category)
  {
    return std::errc::invalid_argument;
  }

  if (!strings)
  {
    return std::errc::invalid_argument;
  }

  std::u16string_view category_str = category;

  if (category_str == u"All Archives")
  {
    count = std::clamp<std::size_t>(count, 0u, vol_controller::formats.size());

    std::transform(vol_controller::formats.begin(), vol_controller::formats.begin() + count, strings, [](const auto value) {
      return value.data();
    });
  }
  else
  {
    count = 0;
  }

  if (fetched)
  {
    *fetched = count;
  }

  return count == 0 ? std::errc::not_supported : std::errc(0);
}

std::errc is_stream_supported(storage_info* data) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  if (data->type == storage_info::file && data->info.path)
  {
    auto extension = std::filesystem::path(data->info.path).extension().native();
    if (!std::any_of(vol_controller::formats.begin(), vol_controller::formats.end(), [&](const auto value) {
        return value == extension;
        }))
    {
      return std::errc::not_supported;
    }
  }

  auto stream = siege::platform::create_istream(*data);

  if (siege::views::vol_controller::is_vol(*stream))
  {
    return std::errc(0);
  }

  return std::errc::not_supported;
}

std::errc stream_is_resource_reader(storage_info* data) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  auto stream = siege::platform::create_istream(*data);

  if (siege::resource::is_resource_reader(*stream))
  {
    return std::errc(0);
  }

  return std::errc::not_supported;
}

namespace fs = std::filesystem;
using fs_char = fs::path::value_type;

struct siege::platform::resource_reader_context
{
  std::streampos start_pos;
  std::unique_ptr<siege::platform::resource_reader> reader;
  std::unique_ptr<std::istream> stream;

  std::unordered_set<fs::path> path_storage;
  std::unordered_map<std::basic_string_view<fs_char>,
    std::vector<siege::platform::resource_reader::content_info>>
    index;
};

std::errc create_reader_context(storage_info* data, siege::platform::resource_reader_context** storage) noexcept
{
  if (!data)
  {
    return std::errc::invalid_argument;
  }

  if (!storage)
  {
    return std::errc::invalid_argument;
  }

  auto stream = siege::platform::create_istream(*data);

  if (siege::resource::is_resource_reader(*stream))
  {
    *storage = new siege::platform::resource_reader_context{
      .start_pos = stream->tellg(),
      .reader = siege::resource::make_resource_reader(*stream),
      .stream = std::move(stream),
    };

    return std::errc(0);
  }

  *storage = nullptr;

  return std::errc::not_supported;
}

std::errc get_reader_folder_listing(siege::platform::resource_reader_context* context, const fs_char* parent_path, std::size_t count, const fs_char** folders, std::size_t* expected)
{
  if (!context)
  {
    return std::errc::invalid_argument;
  }

  if (!context->index.contains(parent_path))
  {
    auto new_path = context->path_storage.emplace(parent_path);

    context->stream->seekg(context->start_pos);
    auto contents = context->reader->get_content_listing(*context->stream, siege::platform::listing_query{ .archive_path = siege::platform::get_stream_path(*context->stream).value_or(std::filesystem::path{}), .folder_path = parent_path });
    context->index.emplace(new_path.first->c_str(), std::move(contents));
  }

  auto& contents = context->index[parent_path];

  auto folder_count = std::count_if(contents.begin(), contents.end(), [](auto& content) {
    return std::get_if<siege::platform::folder_info>(&content) != nullptr;
  });

  if ((count == 0 || folders == nullptr) && expected)
  {
    *expected = folder_count;
    return std::errc(0);
  }

  if (count > 0 && folders != nullptr)
  {
    auto current = 0u;
    for (auto& content : contents)
    {
      if (current >= count)
      {
        break;
      }
      if (auto* folder = std::get_if<siege::platform::folder_info>(&content); folder)
      {
        folders[current] = folder->full_path.c_str();
        current++;
      }
    }
  }

  if (expected)
  {
    *expected = folder_count;
  }

  return std::errc(0);
}

std::errc get_reader_file_listing(siege::platform::resource_reader_context* context, const fs_char* parent_path, std::size_t count, const fs_char** files, std::size_t* expected)
{
  if (!context)
  {
    return std::errc::invalid_argument;
  }

  if (!context->index.contains(parent_path))
  {
    auto new_path = context->path_storage.emplace(parent_path);

    context->stream->seekg(context->start_pos);
    auto contents = context->reader->get_content_listing(*context->stream, siege::platform::listing_query{ .archive_path = siege::platform::get_stream_path(*context->stream).value_or(std::filesystem::path{}), .folder_path = parent_path });
    context->index.emplace(new_path.first->c_str(), std::move(contents));
  }

  auto& contents = context->index[parent_path];

  auto file_count = std::count_if(contents.begin(), contents.end(), [](auto& content) {
    return std::get_if<siege::platform::file_info>(&content) != nullptr;
  });

  if ((count == 0 || files == nullptr) && expected)
  {
    *expected = file_count;
    return std::errc(0);
  }

  if (count > 0 && files != nullptr)
  {
    auto current = 0u;
    for (auto& content : contents)
    {
      if (current >= count)
      {
        break;
      }
      if (auto* file = std::get_if<siege::platform::file_info>(&content); file)
      {
        auto new_path = context->path_storage.emplace(file->folder_path / file->filename);
        files[current] = new_path.first->c_str();
        current++;
      }
    }
  }

  if (expected)
  {
    *expected = file_count;
  }

  return std::errc(0);
}

std::errc extract_file_contents(siege::platform::resource_reader_context* context, const fs_char* file_path, std::size_t size, std::byte* buffer, std::size_t* written)
{
  if (!context)
  {
    return std::errc::invalid_argument;
  }

  auto real_path = std::filesystem::path(file_path);
  auto parent_path = real_path.parent_path();

  if (!context->index.contains(parent_path.c_str()))
  {
    auto new_path = context->path_storage.emplace(parent_path);

    context->stream->seekg(context->start_pos);
    auto contents = context->reader->get_content_listing(*context->stream, siege::platform::listing_query{ .archive_path = siege::platform::get_stream_path(*context->stream).value_or(std::filesystem::path{}), .folder_path = parent_path });
    context->index.emplace(new_path.first->c_str(), std::move(contents));
  }

  auto& contents = context->index[parent_path.c_str()];

  auto file_info = std::find_if(contents.begin(), contents.end(), [&](auto& content) {
    if (auto* info = std::get_if<siege::platform::file_info>(&content); info)
    {
      return info->folder_path == parent_path && info->filename == real_path.filename();
    }

    return false;
  });

  if (file_info == contents.end())
  {
    return std::errc::not_a_stream;
  }

  auto& real_info = std::get<siege::platform::file_info>(*file_info);
  
  if ((size == 0 || buffer == nullptr) && written)
  {
    *written = real_info.size;
    return std::errc(0);
  }

  std::span<char> temp((char*)buffer, size);
  std::ospanstream output(temp, std::ios::binary);
  context->reader->extract_file_contents(*context->stream, real_info, output);

  if (written)
  {
    *written = std::clamp<std::size_t>(size, 0, real_info.size);
  }

  return std::errc(0);
}

std::errc destroy_reader_context(siege::platform::resource_reader_context* context)
{
  if (context)
  {
    delete context;
    return std::errc(0);
  }

  return std::errc::invalid_argument;
}

}