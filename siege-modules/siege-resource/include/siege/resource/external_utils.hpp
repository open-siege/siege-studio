#ifndef OPEN_SIEGE_EXTERNAL_UTILS_HPP
#define OPEN_SIEGE_EXTERNAL_UTILS_HPP

#include <vector>
#include <siege/platform/archive_plugin.hpp>

namespace siege::resource
{
  using content_info = std::variant<platform::folder_info, siege::platform::file_info>;
  std::vector<content_info> zip_get_content_listing(const platform::listing_query& query);
  std::vector<content_info> iso_get_content_listing(const platform::listing_query& query);
  std::vector<content_info> cab_get_content_listing(const platform::listing_query& query);

  [[maybe_unused]] bool seven_extract_file_contents(const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage);

  void iso_extract_file_contents(const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage);

  void cab_extract_file_contents(const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage);

  [[nodiscard]] inline std::string rtrim(std::string str)
  {
    auto it1 = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    str.erase(it1.base(), str.end());
    return str;
  }

  [[nodiscard]] inline std::vector<std::string> read_lines(std::istream& input)
  {
    std::vector<std::string> raw_contents;
    raw_contents.reserve(128);

    for (std::string temp; std::getline(input, temp);)
    {
      raw_contents.push_back(rtrim(std::move(temp)));
    }

    return raw_contents;
  }
}

#endif// OPEN_SIEGE_EXTERNAL_UTILS_HPP
