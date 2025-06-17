#include <siege/platform/resource.hpp>

namespace siege::platform
{
  std::list<platform::file_info> resource_reader::get_all_files_for_query(std::any& cache, std::istream& stream, const platform::listing_query& query) const
  {
    std::list<platform::file_info> results;
    auto top_level_items = this->get_content_listing(cache, stream, query);

    std::function<void(decltype(top_level_items)&)> get_full_listing = [&](std::vector<resource_reader::content_info>& items) mutable {
      for (resource_reader::content_info& info : items)
      {
        if (auto parent_info = std::get_if<folder_info>(&info); parent_info)
        {
          std::vector<resource_reader::content_info> children;
          children = this->get_content_listing(cache, stream, listing_query{ .archive_path = query.archive_path, .folder_path = parent_info->full_path });
          get_full_listing(children);
        }

        if (auto leaf_info = std::get_if<file_info>(&info); leaf_info)
        {
          results.emplace_back(*leaf_info);
        }
      }
    };

    get_full_listing(top_level_items);
    
    return results;
  }
}// namespace siege::platform