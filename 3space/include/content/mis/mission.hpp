#ifndef INC_3SPACESTUDIO_MISSION_HPP
#define INC_3SPACESTUDIO_MISSION_HPP

#include <vector>
#include <map>
#include <map>
#include <optional>
#include <istream>
#include <variant>
#include <filesystem>
#include "resources/archive_plugin.hpp"
#include "content/tagged_data.hpp"
#include "endian_arithmetic.hpp"
#include "shared.hpp"

namespace studio::mis::darkstar
{
  namespace endian = boost::endian;
  struct sim_set;
  struct sim_group;
  struct sim_volume;
  struct sim_terrain;
  struct sim_palette;
  struct interior_shape;
  struct sim_structure;
  struct sim_marker;
  struct nav_marker;
  struct vehicle;

  using sim_item = std::variant<sim_set, sim_group, sim_volume,
    sim_terrain, sim_palette, interior_shape, sim_structure, sim_marker, nav_marker, vehicle, raw_item>;

  using sim_items = std::vector<sim_item>;

  struct sim_set
  {
    object_header header;
    endian::little_uint32_t item_id;
    endian::little_uint32_t children_count;
    sim_items children;
  };

  struct sim_group
  {
    object_header header;
    endian::little_uint32_t version;
    endian::little_uint32_t children_count;
    sim_items children;
    std::vector<std::string> names;
  };

  struct address
  {
    endian::little_uint32_t reserved;
    endian::little_uint32_t object_id;
    endian::little_uint32_t manager_id;
  };

  struct sim_network_object
  {
    endian::little_uint32_t version;
    endian::little_uint32_t id;
    endian::little_uint32_t flags;
    address object_address;
  };

  struct sim_volume
  {
    object_header header;
    sim_network_object base;
    std::string filename;
  };

  struct sim_terrain
  {
    object_header header;
    sim_network_object base;
    std::array<std::byte, 48> data;
    std::string dtf_file;
    std::array<std::byte, 20> footer;
  };

  struct sim_palette
  {
    object_header header;
    sim_network_object base;
    std::string palette_file;
  };


  struct interior_shape
  {
    object_header header;
    endian::little_uint32_t version;
    std::array<std::byte, 124> data;
    endian::little_uint32_t string_length;
    std::string filename;
    std::array<std::byte, 22> footer;
  };

  struct sim_structure
  {
    object_header header;
    endian::little_uint32_t version;
    std::array<std::byte, 117> data;
    endian::little_uint32_t string_length;
    std::string filename;
  };

  struct transform_matrix
  {
    endian::little_int32_t flags;
    std::array<std::array<float, 3>, 3> rotation;
    std::array<float, 3> position;
  };

  struct sim_marker
  {
    object_header header;
    sim_network_object base;
    transform_matrix transformation;
  };

  struct nav_marker
  {
    object_header header;
    sim_network_object base;
    transform_matrix transformation;
    std::array<std::byte, 12> footer;
  };

  struct vehicle
  {
    constexpr static auto type_fields_size = sizeof(std::array<endian::little_uint16_t, 9>);

    object_header header;
    endian::little_uint32_t version;
    std::array<std::byte, 14> data;

    endian::little_uint16_t vehicle_type;
    endian::little_uint16_t engine_type;
    endian::little_uint16_t reactor_type;
    endian::little_uint16_t computer_type;
    endian::little_uint16_t shield_type;
    endian::little_uint16_t armor_type;
    endian::little_uint16_t sensor_type;
    endian::little_uint16_t special_1_type;
    endian::little_uint16_t special_2_type;

    std::vector<std::byte> footer;
  };

  bool is_mission_data(std::basic_istream<std::byte>& file);

  sim_items read_mission_data(std::basic_istream<std::byte>& file);

}// namespace studio::mis::darkstar

namespace studio::resources::mis::darkstar
{
  struct mis_file_archive : studio::resources::archive_plugin
  {
    inline static std::array<std::string_view, 1> supported_extensions = std::array<std::string_view, 1>{ std::string_view{".veh"} };
    using ref_vector = std::vector<std::pair<std::reference_wrapper<::studio::mis::darkstar::sim_item>, content_info>>;
    mutable std::map<std::filesystem::path, ::studio::mis::darkstar::sim_items> contents;
    mutable std::map<std::filesystem::path, ref_vector> content_list_info;

    decltype(content_list_info)::iterator cache_data(std::basic_istream<std::byte>& stream, const std::filesystem::path& archive_or_folder_path) const;

    static bool is_supported(std::basic_istream<std::byte>& stream);

    bool stream_is_supported(std::basic_istream<std::byte>& stream) const override;
    std::vector<content_info> get_content_listing(std::basic_istream<std::byte>& stream, const listing_query& query) const override;
    void set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const override;
    void extract_file_contents(std::basic_istream<std::byte>& stream,
      const studio::resources::file_info& info,
      std::basic_ostream<std::byte>& output,
      std::optional<std::reference_wrapper<batch_storage>> = std::nullopt) const override;
  };
}// namespace studio::resources::mis::darkstar

#endif//INC_3SPACESTUDIO_MISSION_HPP
