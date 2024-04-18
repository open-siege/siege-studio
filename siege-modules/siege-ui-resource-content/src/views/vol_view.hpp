#ifndef DARKSTARDTSCONVERTER_VOL_VIEW_HPP
#define DARKSTARDTSCONVERTER_VOL_VIEW_HPP

#include <future>
#include <string_view>
#include <optional>
#include <wx/treelist.h>
#include "graphics_view.hpp"
#include "view_context.hpp"
#include "siege/resource/resource_explorer.hpp"

namespace siege::views
{
  class vol_view
  {
  public:
    vol_view(view_context context);
    void setup_view(wxWindow& parent);

  private:
    void filter_files(const std::set<std::filesystem::path>& folders,
      std::shared_ptr<wxTreeListCtrl> table,
      std::optional<std::string_view> search_text
    );

    view_context context;
    std::filesystem::path archive_path;
    std::vector<siege::platform::file_info> files;
    std::future<bool> pending_save;
    bool should_cancel;
    bool opened_folder = false;
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_VOL_VIEW_HPP
