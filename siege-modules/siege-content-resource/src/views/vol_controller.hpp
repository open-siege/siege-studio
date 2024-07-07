#ifndef VOL_CONTROLLER_HPP
#define VOL_CONTROLLER_HPP

#include <istream>
#include <vector>
#include <variant>
#include <filesystem>
#include <sstream>
#include <span>
#include <siege/platform/resource.hpp>

namespace siege::views
{
  class vol_controller
  {
  public:
    constexpr static auto formats = std::array<siege::fs_string_view, 20>{ { FSL".vol",
      FSL".rmf",
      FSL".mis",
      FSL".map",
      FSL".rbx",
      FSL".tbv",
      FSL".zip",
      FSL".vl2",
      FSL".pk3",
      FSL".iso",
      FSL".mds",
      FSL".cue",
      FSL".nrg",
      FSL".7z",
      FSL".tgz",
      FSL".rar",
      FSL".cab",
      FSL".z",
      FSL".cln",
      FSL".atd" } };
    static bool is_vol(std::istream&) noexcept;
    std::size_t load_volume(std::istream&, std::optional<std::filesystem::path>);
    std::span<siege::platform::resource_reader::content_info> get_contents();
    std::vector<char> load_content_data(const siege::platform::resource_reader::content_info&);

  private:
    std::unique_ptr<siege::platform::resource_reader> resource;
    std::vector<siege::platform::resource_reader::content_info> contents;
    std::variant<std::monostate, std::filesystem::path, std::stringstream> storage;
  };
}// namespace siege::views

#endif// !VOL_CONTROLLER_HPP
