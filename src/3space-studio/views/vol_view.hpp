#ifndef DARKSTARDTSCONVERTER_VOL_VIEW_HPP
#define DARKSTARDTSCONVERTER_VOL_VIEW_HPP

#include <future>
#include "graphics_view.hpp"
#include "resource/resource_explorer.hpp"

namespace studio::views
{
  class vol_view : public normal_view
  {
  public:
    vol_view(const studio::resource::file_info& info, const studio::resource::resource_explorer& archive);
    void setup_view(wxWindow& parent) override;

  private:
    const studio::resource::resource_explorer& archive;
    std::filesystem::path archive_path;
    std::vector<studio::resource::file_info> files;
    std::future<bool> pending_save;
    bool should_cancel;
    bool opened_folder = false;
  };
}// namespace studio::views

#endif//DARKSTARDTSCONVERTER_VOL_VIEW_HPP
