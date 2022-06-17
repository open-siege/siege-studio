#ifndef DARKSTARDTSCONVERTER_VOL_VIEW_HPP
#define DARKSTARDTSCONVERTER_VOL_VIEW_HPP

#include <future>
#include <string_view>
#include <optional>
#include <wx/treelist.h>
#include "graphics_view.hpp"
#include "resources/resource_explorer.hpp"

namespace studio::views
{
  class vol_view
  {
  public:
    vol_view(const studio::resources::file_info& info, const studio::resources::resource_explorer& archive);
    void setup_view(wxWindow& parent);

  private:
    void filter_files(const std::set<std::filesystem::path>& folders,
      std::shared_ptr<wxTreeListCtrl> table,
      std::optional<std::string_view> search_text
    );

    const studio::resources::resource_explorer& archive;
    std::filesystem::path archive_path;
    std::vector<studio::resources::file_info> files;
    std::future<bool> pending_save;
    bool should_cancel;
    bool opened_folder = false;
  };
}// namespace studio::views

#endif//DARKSTARDTSCONVERTER_VOL_VIEW_HPP
