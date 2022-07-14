#ifndef DARKSTARDTSCONVERTER_VIEW_FACTORY_HPP
#define DARKSTARDTSCONVERTER_VIEW_FACTORY_HPP

#include <istream>
#include <memory>
#include <variant>
#include <set>
#include <map>
#include <string>
#include <string_view>
#include "graphics_view.hpp"
#include "default_view.hpp"
#include "resources/resource_explorer.hpp"
#include "utility.hpp"

namespace studio::views
{
  using stream_validator = bool(std::basic_istream<std::byte>&);

  using studio_view = std::variant<std::monostate, normal_view, graphics_view>;
  using view_creator = studio_view(const studio::resources::file_info&, std::basic_istream<std::byte>&, const studio::resources::resource_explorer&);

  class view_factory
  {
  public:
    void add_file_type(stream_validator* checker, view_creator* creator, bool no_fallback = false);

    void add_extension(std::string_view extension, stream_validator* checker);

    void add_extension_category(std::string_view category, std::vector<std::string_view> extensions, bool is_interface_visible = false);

    [[nodiscard]] std::vector<std::string_view> get_extensions() const;

    [[nodiscard]] std::vector<std::string_view> get_extensions_by_category(std::string_view category) const;

    [[nodiscard]] std::vector<std::string_view> get_extension_categories(bool only_interface_visible = true) const;

    studio_view create_view(const studio::resources::file_info& file_info, std::basic_istream<std::byte>& stream, const studio::resources::resource_explorer& manager) const;

    studio_view create_default_view(const studio::resources::file_info& file_info, std::basic_istream<std::byte>& stream, const studio::resources::resource_explorer& manager) const;

  private:
    std::set<std::string> extensions;
    std::map<std::string, std::set<std::string_view>> extension_categories;
    std::set<std::string_view> visible_categories;
    std::set<stream_validator*> no_fallback_allowed;
    std::map<stream_validator*, view_creator*> creators;
    std::multimap<std::string_view, stream_validator*> validators;
  };
}// namespace studio::views


#endif//DARKSTARDTSCONVERTER_VIEW_FACTORY_HPP
