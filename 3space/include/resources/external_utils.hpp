#ifndef OPEN_SIEGE_EXTERNAL_UTILS_HPP
#define OPEN_SIEGE_EXTERNAL_UTILS_HPP

#include <vector>
#include "archive_plugin.hpp"

namespace studio::resources
{
  using content_info = std::variant<folder_info, studio::resources::file_info>;
  std::vector<content_info> zip_get_content_listing(const listing_query& query);
  std::vector<content_info> iso_get_content_listing(const listing_query& query);
  std::vector<content_info> cab_get_content_listing(const listing_query& query);

  [[maybe_unused]] bool seven_extract_file_contents(const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage);

  void iso_extract_file_contents(const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage);

  void cab_extract_file_contents(const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage);
}

#endif// OPEN_SIEGE_EXTERNAL_UTILS_HPP
