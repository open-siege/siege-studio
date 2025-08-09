#include <filesystem>
#include <optional>
#include <fstream>
#include <siege/platform/win/file.hpp>
#include "vol_shared.hpp"

namespace fs = std::filesystem;

namespace siege::views
{
  std::optional<fs::path> create_self_extracting_resource(std::any& self, fs::path unvol_exe_path, std::optional<fs::path> output_path, std::optional<std::vector<std::string>> post_extract_commands)
  {
    auto data = get_raw_resource_data(self);

    if (!data)
    {
      return {};
    }

    bool should_append = false;

    start:
    if (auto handle = ::BeginUpdateResourceW(unvol_exe_path.c_str(), FALSE); handle)
    {
      bool path_saved = true;

      if (output_path)
      {
        auto output_path_str = output_path->string();
        path_saved = ::UpdateResourceW(handle, RT_RCDATA, L"output_path", LANG_SYSTEM_DEFAULT, output_path_str.data(), (DWORD)output_path_str.size());
      }

      bool post_extract_saved = true;
      if (post_extract_commands && !post_extract_commands->empty())
      {
        std::string combined_commands;

        std::size_t size = 0;

        std::string_view eol = "\r\n";

        for (auto& item : *post_extract_commands)
        {
          size += item.size() + eol.size();
        }

        combined_commands.reserve(size);

        for (auto& item : *post_extract_commands)
        {
          combined_commands.append(item);
          combined_commands.append(eol);
        }

        post_extract_saved = ::UpdateResourceW(handle, RT_RCDATA, L"post_extract", LANG_SYSTEM_DEFAULT, combined_commands.data(), (DWORD)combined_commands.size());
      }

      std::span<char> raw_data = data;

      bool embedded_saved = false;

      if (!should_append)
      {
        embedded_saved = ::UpdateResourceW(handle, RT_RCDATA, L"embedded", LANG_SYSTEM_DEFAULT, raw_data.data(), raw_data.size());
      }
      
      auto should_undo = (embedded_saved || should_append) && path_saved && post_extract_saved ? FALSE : TRUE;
      ::EndUpdateResourceW(handle, should_undo);


      if (should_append)
      {
        auto offset = (std::uint32_t)fs::file_size(unvol_exe_path);

        auto handle = ::BeginUpdateResourceW(unvol_exe_path.c_str(), FALSE);
        post_extract_saved = ::UpdateResourceW(handle, RT_RCDATA, L"embedded_offset", LANG_SYSTEM_DEFAULT, &offset, (DWORD)sizeof(offset));
        ::EndUpdateResourceW(handle, FALSE);

        std::ofstream exe_stream(unvol_exe_path, std::ios::binary | std::ios::app);

        exe_stream.write(raw_data.data(), raw_data.size());
        embedded_saved = !exe_stream.bad();
      }

      if (!embedded_saved && !should_append)
      {
        should_append = true;
        goto start;
      }

      if (embedded_saved)
      {
        return unvol_exe_path;
      }
    }

    return std::nullopt;
  }
}// namespace siege::views