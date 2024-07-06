#ifndef SIEGE_STORAGE_MODULE_HPP
#define SIEGE_STORAGE_MODULE_HPP

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <expected>
#include <list>
#include <string>
#include <vector>
#include <set>
#include <functional>
#include <system_error>
#include <siege/platform/stream.hpp>
#include <siege/platform/resource.hpp>
#include <siege/platform/win/desktop/window_module.hpp>

namespace siege::platform
{
  using fs_char = std::filesystem::path::value_type;
  using stream_is_resource_reader = std::errc __stdcall(storage_info* data);
  using create_reader_context = std::errc __stdcall(storage_info* data, resource_reader_context** storage);
  using get_reader_folder_listing = std::errc __stdcall(resource_reader_context* context, const fs_char* parent_path, std::size_t count, const fs_char** folders, std::size_t*);
  using get_reader_file_listing = std::errc __stdcall(resource_reader_context* context, const fs_char* parent_path, std::size_t, const fs_char** folders, std::size_t*);
  using extract_file_contents = std::errc __stdcall(resource_reader_context* context, const fs_char* file_path, std::size_t size, std::byte* buffer, std::size_t* written);
  using destroy_reader_context = std::errc __stdcall(resource_reader_context* context);

  class storage_module : public win32::module
  {
    using base = win32::module;

    stream_is_resource_reader* stream_is_resource_reader_proc = nullptr;
    create_reader_context* create_reader_context_proc = nullptr;
    get_reader_folder_listing* get_reader_folder_listing_proc = nullptr;
    get_reader_file_listing* get_reader_file_listing_proc = nullptr;
    extract_file_contents* extract_file_contents_proc = nullptr;
    destroy_reader_context* destroy_reader_context_proc = nullptr;

  public:
    storage_module(std::filesystem::path module_path) : base(module_path)
    {
      stream_is_resource_reader_proc = GetProcAddress<decltype(stream_is_resource_reader_proc)>("stream_is_resource_reader");
      create_reader_context_proc = GetProcAddress<decltype(create_reader_context_proc)>("create_reader_context");
      get_reader_folder_listing_proc = GetProcAddress<decltype(get_reader_folder_listing_proc)>("get_reader_folder_listing");
      get_reader_file_listing_proc = GetProcAddress<decltype(get_reader_file_listing_proc)>("get_reader_file_listing");
      extract_file_contents_proc = GetProcAddress<decltype(extract_file_contents_proc)>("extract_file_contents");
      destroy_reader_context_proc = GetProcAddress<decltype(destroy_reader_context_proc)>("destroy_reader_context");

      if (!(create_reader_context_proc || destroy_reader_context_proc))
      {
        throw std::runtime_error("Could not find module functions");
      }
    }

    static std::list<storage_module> load_modules(std::filesystem::path search_path)
    {
      std::list<storage_module> loaded_modules;

      for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
      {
        if (dir_entry.path().extension() == ".dll")
        {
          try
          {
            loaded_modules.emplace_back(dir_entry.path());
          }
          catch (...)
          {
          }
        }
      }

      return loaded_modules;
    }

    bool stream_is_resource_reader(storage_info data) const noexcept
    {
      return stream_is_resource_reader_proc(&data) == std::errc(0);
    }

    using context_ptr = std::unique_ptr<resource_reader_context,
      std::move_only_function<void(resource_reader_context*)>>;

    inline std::expected<context_ptr, std::error_code> create_reader_context(storage_info data) const noexcept
    {
      resource_reader_context* result = nullptr;

      auto errc = create_reader_context_proc(&data, &result);

      if (errc != std::errc(0))
      {
        return std::unexpected(std::make_error_code(errc));
      }

      return context_ptr(result, [destroy_reader_context_proc = destroy_reader_context_proc](auto* context) {
        destroy_reader_context_proc(context);
      });
    }

    inline std::expected<std::vector<std::filesystem::path>, std::error_code> get_reader_folder_listing(resource_reader_context& context, const std::filesystem::path& parent_path)
    {
      std::vector<const wchar_t*> temp;

      std::size_t expected = 0;
      auto errc = get_reader_folder_listing_proc(&context, parent_path.c_str(), 0, nullptr, &expected);

      if (errc != std::errc(0))
      {
        return std::unexpected(std::make_error_code(errc));
      }

      if (expected == 0)
      {
        return std::vector<std::filesystem::path>{};
      }

      temp.resize(expected);

      errc = get_reader_folder_listing_proc(&context, parent_path.c_str(), temp.size(), temp.data(), &expected);

      if (errc != std::errc(0))
      {
        return std::unexpected(std::make_error_code(errc));
      }

      std::vector<std::filesystem::path> results;
      results.reserve(temp.size());

      std::transform(temp.begin(), temp.end(), std::back_inserter(results), [](auto& path) {
        return path;
      });

      return results;
    }

    inline std::expected<std::vector<std::filesystem::path>, std::error_code> get_reader_file_listing(resource_reader_context& context, const std::filesystem::path& parent_path)
    {
      std::vector<const wchar_t*> temp;

      std::size_t expected = 0;
      auto errc = get_reader_file_listing_proc(&context, parent_path.c_str(), 0, nullptr, &expected);

      if (errc != std::errc(0))
      {
        return std::unexpected(std::make_error_code(errc));
      }

      if (expected == 0)
      {
        return std::vector<std::filesystem::path>{};
      }

      temp.resize(expected);

      errc = get_reader_file_listing_proc(&context, parent_path.c_str(), temp.size(), temp.data(), &expected);

      if (errc != std::errc(0))
      {
        return std::unexpected(std::make_error_code(errc));
      }

      std::vector<std::filesystem::path> results;
      results.reserve(temp.size());

      std::transform(temp.begin(), temp.end(), std::back_inserter(results), [](auto& path) {
        return path;
      });

      return results;
    }

    inline std::expected<std::vector<char>, std::error_code> extract_file_contents(resource_reader_context& context, const std::filesystem::path& file_path)
    {
      std::vector<char> buffer;
      std::size_t expected = 0;

      auto errc = extract_file_contents_proc(&context, file_path.c_str(), 0, nullptr, &expected);

      if (errc != std::errc(0))
      {
        return std::unexpected(std::make_error_code(errc));
      }

      if (expected == 0)
      {
        return buffer;
      }

      buffer.resize(expected);

      errc = extract_file_contents_proc(&context, file_path.c_str(), buffer.size(), (std::byte*)buffer.data(), &expected);

      if (errc != std::errc(0))
      {
        return std::unexpected(std::make_error_code(errc));
      }

      return buffer;
    }
  };

}// namespace siege::platform


#endif// !SIEGEPLUGINHPP
