#ifndef DARKSTARDTSCONVERTER_SHARED_HPP
#define DARKSTARDTSCONVERTER_SHARED_HPP

#include <array>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <filesystem>

namespace studio::shared
{
  namespace fs = std::filesystem;

  template<std::size_t Size>
  constexpr std::array<std::string_view, Size> make_keys(const char*(&&keys)[Size])
  {
    std::array<std::string_view, Size> result;
    for (auto i = 0; i < Size; i++)
    {
      result[i] = keys[i];
    }
    return result;
  }

  constexpr std::size_t get_padding_size(std::size_t count, std::size_t alignment_size)
  {
    auto result = 0u;

    while ((count % alignment_size) != 0)
    {
      count++;
      result++;
    }

    return result;
  }

  template <std::size_t Count>
  constexpr std::array<std::byte, Count> to_tag(const char* values)
  {
    std::array<std::byte, Count> result{};

    for (auto i = 0u; i < Count; i++)
    {
      result[i] = std::byte(values[i]);
    }

    return result;
  }

  template <std::size_t Count>
  constexpr std::array<std::byte, Count> to_tag(const std::array<std::uint8_t, Count> values)
  {
    std::array<std::byte, Count> result{};

    for (auto i = 0u; i < values.size(); i++)
    {
      result[i] = std::byte{ values[i] };
    }

    return result;
  }

  template<char Delimiter>
  class delimited_string : public std::string
  {
  };

  template<char Delimiter>
  std::istream& operator>>(std::istream& is, delimited_string<Delimiter>& output)
  {
    std::getline(is, output, Delimiter);
    return is;
  }

  inline bool ends_with(const std::string& main_str, const std::string& to_match)
  {
    if (main_str.size() >= to_match.size() && main_str.compare(main_str.size() - to_match.size(), to_match.size(), to_match) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }


  template<typename... Strings>
  std::vector<fs::path> find_files(const std::vector<std::string>& file_names, Strings const&... defaults)
  {
    std::vector<fs::path> files;

    std::set<std::string> extensions;

    for (const auto& file_name : file_names)
    {
      if (file_name == "*")
      {
        (extensions.insert(defaults), ...);
        continue;
      }

      /*
       * TODO make * globbing work for the following cases:
       * Some*.dts
       * *File.dts
       * Some*File.dts
       * *.dts
       * *.*
      std::istringstream string_splitter(file_name);

      const std::vector<std::string> results((std::istream_iterator<delimited_string<'*'>>(string_splitter)),
        std::istream_iterator<delimited_string<'*'>>());

      */

      if (auto glob_index = file_name.rfind("*.", 0); glob_index == 0)
      {
        extensions.insert(file_name.substr(glob_index + 1));
        continue;
      }

      if (auto path = fs::current_path().append(file_name); fs::exists(path))
      {
        files.push_back(path);
      }
    }

    if (!extensions.empty())
    {
      for (auto& item : fs::directory_iterator(fs::current_path()))
      {
        for (auto& extension : extensions)
        {
          if (ends_with(item.path().filename().string(), extension) && item.is_regular_file())
          {
            files.push_back(item.path());
          }
        }
      }
    }

    return files;
  }
}// namespace darkstar::dts::shared

#endif//DARKSTARDTSCONVERTER_SHARED_HPP
