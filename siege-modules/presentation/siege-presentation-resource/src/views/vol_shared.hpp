#ifndef VOL_SHARED_HPP
#define VOL_SHARED_HPP


#include <span>
#include <siege/platform/shared.hpp>
#include <siege/platform/resource.hpp>


namespace siege::views
{
  std::span<const siege::fs_string_view> get_volume_formats() noexcept;
  bool is_vol(std::istream&) noexcept;
  
  std::size_t load_volume(std::any&, std::istream&, std::optional<std::filesystem::path>, std::function<void(siege::platform::resource_reader::content_info&)> on_new_item);
  std::vector<std::reference_wrapper<siege::platform::resource_reader::content_info>> get_contents(std::any&);
  std::vector<char> load_content_data(std::any&, const siege::platform::resource_reader::content_info&);
  std::vector<char> get_raw_resource_data(std::any&);

  void stop_loading(std::any&);
  std::optional<std::filesystem::path> get_original_path(std::any&);
}// namespace siege::views

#endif// !VOL_SHARED_HPP
