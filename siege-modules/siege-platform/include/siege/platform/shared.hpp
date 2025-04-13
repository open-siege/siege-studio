#ifndef DARKSTARDTSCONVERTER_SHARED_HPP
#define DARKSTARDTSCONVERTER_SHARED_HPP

#include <array>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <variant>
#include <optional>
#include <filesystem>
#include <algorithm>

#if WIN32
#define FSL L""
#else
#define FSL
#endif

namespace siege
{
  using fs_char = std::filesystem::path::value_type;
  using fs_string = std::filesystem::path::string_type;
  using fs_string_view = std::basic_string_view<fs_char>;
}// namespace siege

namespace siege::platform
{
  struct istream_pos_resetter
  {
    std::istream& stream;
    std::istream::pos_type position;

    istream_pos_resetter(std::istream& stream) : stream(stream), position(stream.tellg())
    {
    }

    ~istream_pos_resetter()
    {
      stream.seekg(position, std::ios::beg);
    }
  };

  namespace fs = ::std::filesystem;

  // A big thanks to https://stackoverflow.com/questions/56246573/how-to-copy-an-element-of-stdvariant-to-a-variable-of-another-variant-type
  template<typename MainType>
  struct Exactly
  {
    template<typename OtherType,
      std::enable_if_t<std::is_same_v<MainType, OtherType>, int> = 0>
    operator OtherType() const;
  };

  template<typename To, typename From>
  bool can_variant_cast(From&& from)
  {
    return std::visit([](auto&& elem) -> bool {
      using ElemType = std::decay_t<decltype(elem)>;
      if constexpr (std::is_constructible_v<To, Exactly<ElemType>>)
      {
        return true;
      }
      else
      {
        return false;
      }
    },
      std::forward<From>(from));
  }

  template<typename To, typename From>
  To variant_cast_def(From&& from)
  {
    return std::visit([](auto&& elem) -> To {
      using ElemType = std::decay_t<decltype(elem)>;
      if constexpr (std::is_constructible_v<To, Exactly<ElemType>>)
      {
        return To(std::forward<decltype(elem)>(elem));
      }
      else
      {
        return To();
      }
    },
      std::forward<From>(from));
  }


  template<typename To, typename From>
  std::optional<To> variant_cast_opt(From&& from)
  {
    return std::visit([](auto&& elem) -> std::optional<To> {
      using ElemType = std::decay_t<decltype(elem)>;
      if constexpr (std::is_constructible_v<To, Exactly<ElemType>>)
      {
        return To(std::forward<decltype(elem)>(elem));
      }
      else
      {
        return std::nullopt;
      }
    },
      std::forward<From>(from));
  }

  template<typename To, typename From>
  std::vector<To> transform_variants(const std::vector<From>& items)
  {
    std::vector<To> new_items;
    new_items.reserve(std::count_if(new_items.first(), new_items.last(), [](auto& item) { return siege::platform::can_variant_cast<To>(item); }));

    for (auto& item : items)
    {
      if (siege::platform::can_variant_cast<To>(item))
      {
        new_items.emplace_back(siege::platform::variant_cast_def<To>(item));
      }
    }

    return new_items;
  }

  // I wish this wasn't needed. But the constexpr issue with 14.28/19.28 still exists:
  // https://developercommunity2.visualstudio.com/t/internal-compiler-error-with-constexpr-code/1325445?from=email
  // Couldn't get msys2 to work because inside the conan build, deep down, pacman fails after 10 seconds of downloading, and the fix for it is to apply a specific command line parameters.
  // But, finding which package to fix/report on is more work than I have time for on a dark Sunday morning.
#if _MSC_VER >= 1928
#define KEYS_CONSTEXPR inline
  template<std::size_t Size>
  std::array<const char*, Size> make_keys(const char* (&&keys)[Size])
  {
    std::array<const char*, Size> result;
    for (auto i = 0; i < Size; i++)
    {
      result[i] = keys[i];
    }
    return result;
  }
#else
#define KEYS_CONSTEXPR constexpr
  template<std::size_t Size>
  constexpr std::array<std::string_view, Size> make_keys(const char* (&&keys)[Size])
  {
    std::array<std::string_view, Size> result;
    for (auto i = 0; i < Size; i++)
    {
      result[i] = keys[i];
    }
    return result;
  }
#endif

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

  template<std::size_t Count>
  constexpr std::array<std::byte, Count> to_tag(const char* values)
  {
    std::array<std::byte, Count> result{};

    for (auto i = 0u; i < Count; i++)
    {
      result[i] = std::byte(values[i]);
    }

    return result;
  }

  template<std::size_t Count>
  constexpr std::array<std::byte, Count> to_tag(const std::array<std::uint8_t, Count> values)
  {
    std::array<std::byte, Count> result{};

    for (auto i = 0u; i < values.size(); i++)
    {
      result[i] = std::byte{ values[i] };
    }

    return result;
  }

  template<typename CharType>
  inline std::basic_string<CharType> to_lower(std::basic_string_view<CharType> some_string, const std::locale& locale)
  {
    std::basic_string<CharType> result(some_string);
    std::transform(result.begin(), result.end(), result.begin(), [&](auto c) { return std::tolower(c, locale); });
    return result;
  }

  template<typename CharType>
  inline std::basic_string<CharType> to_lower(std::basic_string_view<CharType> some_string)
  {
    std::basic_string<CharType> result(some_string);
    std::transform(result.begin(), result.end(), result.begin(), [&](auto c) { return std::tolower(c, std::locale()); });
    return result;
  }

  inline std::wstring to_lower(std::wstring_view some_string)
  {
    return to_lower<wchar_t>(some_string);
  }

  inline std::u16string to_lower(std::u16string_view some_string)
  {
    return to_lower<char16_t>(some_string);
  }

  inline std::string to_lower(std::string_view some_string)
  {
    return to_lower<char>(some_string);
  }

  inline bool ends_with(std::string_view value, std::string_view ending)
  {
    if (ending.size() > value.size())
    {
      return false;
    }
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
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
}// namespace siege::platform

#endif// DARKSTARDTSCONVERTER_SHARED_HPP
