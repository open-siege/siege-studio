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
#include "endian_arithmetic.hpp"
#include "shared.hpp"

namespace studio::mis::darkstar
{
  namespace endian = boost::endian;
  struct sim_set;
  struct sim_group;
  struct raw_item;
  struct sim_volume;
  struct sim_terrain;
  struct sim_palette;
  struct interior_shape;
  struct sim_structure;
  struct sim_marker;
  struct nav_marker;
  struct vehicle;
  struct sim_control;
  struct sim_active_control;
  struct sim_bitmap_control;
  struct es_palette_control;
  struct es_text_wrap_control;
  struct es_smacker_movie_control;
  struct es_button_control;
  struct es_hidden_button_control;
  struct sim_text_control;
  struct sim_timer_control;
  struct es_text_list_control;
  struct es_scroll_control;
  struct es_matrix_control;
  struct es_text_edit_control;

  struct object_header
  {
    std::array<std::byte, 4> object_tag;
    endian::little_uint32_t object_size;
  };

  using sim_item = std::variant<sim_set, sim_group, sim_volume, sim_control, sim_bitmap_control, es_palette_control, es_text_wrap_control, sim_text_control,
    es_button_control, es_smacker_movie_control, es_hidden_button_control, sim_timer_control, sim_active_control, es_text_list_control,
    es_scroll_control, es_matrix_control, es_text_edit_control,
    sim_terrain, sim_palette, interior_shape, sim_structure, sim_marker, nav_marker, vehicle, raw_item>;

  using sim_items = std::vector<sim_item>;

  struct sim_item_reader;

  using sim_item_reader_map = std::map<std::array<std::byte, 4>, darkstar::sim_item_reader>;

  struct sim_item_reader
  {
    darkstar::sim_item (*read)(std::basic_istream<std::byte>&, object_header&, sim_item_reader_map&);
  };

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

  struct sim_control
  {
    constexpr static auto inspect_size = 80 + 1;
    object_header header;
    endian::little_uint32_t version;
    std::byte control_version;

    std::byte opaque;
    std::byte fill_colour;
    std::byte selected_fill_colour;
    std::byte ghost_fill_colour;
    std::byte border;
    std::byte border_color;
    std::byte selected_border_color;
    std::byte ghost_border_color;

    std::string console_command;
    std::string alt_console_command;

    std::array<endian::little_int32_t, 2> position;
    std::array<endian::little_int32_t, 2> size;
    endian::little_uint32_t flags;
    endian::little_uint32_t tag;
    endian::little_int32_t horizontal_sizing;
    endian::little_int32_t vertical_sizing;
    endian::little_int32_t help_tag;
    std::string console_variable;

    endian::little_uint32_t children_count;
    sim_items children;
    std::vector<std::string> names;
  };

  struct sim_active_control
  {
    endian::little_uint32_t version;
    std::byte is_active;
    endian::little_uint32_t message;
    sim_control control_data;
  };

  struct sim_timer_control
  {
    endian::little_uint32_t version;
    endian::little_int32_t message;
    float initial_timeout;
    float interval_timeout;

    sim_control control_data;
  };

  struct sim_bitmap_control
  {
    endian::little_uint32_t version;
    endian::little_int32_t bitmap_tag;
    std::string inspection_data;
    std::byte is_transparent;
    sim_active_control control_data;
  };

  struct sim_text_control
  {
    endian::little_uint32_t version;
    std::array<endian::little_int32_t, 2> unused;
    endian::little_int32_t disabled_font_tag;
    endian::little_int32_t font_tag;
    endian::little_int32_t alt_font_tag;
    endian::little_int32_t text_tag;
    std::string default_text;
    endian::little_int32_t alignment;
    endian::little_int32_t vertical_position_delta;
    sim_active_control control_data;
  };

  struct es_button_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 20> button_data;
    sim_text_control control_data;
  };

  struct es_text_wrap_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 24> button_data;
    sim_text_control control_data;
  };

  struct es_text_list_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 2> list_data;
    sim_text_control control_data;
  };

  struct es_text_edit_control
  {
    endian::little_uint32_t version;
    endian::little_uint32_t numbers_only;
    std::array<endian::little_uint32_t, 2> unused;
    endian::little_int32_t max_length;
    sim_text_control control_data;
  };

  struct es_hidden_button_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 13> button_data;
    sim_active_control control_data;
  };

  struct es_smacker_movie_control
  {
    endian::little_uint32_t version;
    endian::little_int32_t video_tag;
    std::byte unknown1;
    endian::little_int32_t category_tag;
    std::byte unknown2;
    std::array<std::byte, 9> unknown3;
    sim_active_control control_data;
  };

  struct es_palette_control
  {
    endian::little_uint32_t version;
    endian::little_int32_t palette_tag;
    std::string inspection_data;
    sim_control control_data;
  };

  struct es_scroll_control
  {
    endian::little_uint32_t version;
    endian::little_int32_t scroll_pba_tag;
    std::byte use_arrow_keys;
    endian::little_int32_t force_horizontal_scroll_bar;
    endian::little_int32_t force_vertical_scroll_bar;
    std::byte use_constant_thumb_height;
    sim_control control_data;
  };

  struct es_matrix_control
  {
    endian::little_uint32_t version;
    std::array<endian::little_int32_t, 2> header_size;
    std::array<std::byte, 91> raw_data;
    sim_control control_data;
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

  struct raw_item
  {
    object_header header;
    std::vector<std::byte> raw_bytes;
  };

  bool is_mission_data(std::basic_istream<std::byte>& file);

  sim_items read_mission_data(std::basic_istream<std::byte>& file);

  sim_item get_sim_item(const std::filesystem::path&);

  template<typename ItemType>
  bool matches_type(const sim_item& item)
  {
    return false;
  }

  template<typename ItemType>
  std::optional<ItemType> get_real_item(const sim_item& item)
  {
    return std::nullopt;
  }

}// namespace studio::mis::darkstar

namespace studio::resources::mis::darkstar
{
  struct mis_file_archive : studio::resources::archive_plugin
  {
    inline static std::array<std::string_view, 1> supported_extensions = std::array<std::string_view, 1>{ std::string_view{".veh"} };
    using ref_vector = std::vector<std::pair<std::reference_wrapper<::studio::mis::darkstar::sim_item>, content_info>>;
    mutable std::map<std::filesystem::path, ::studio::mis::darkstar::sim_items> contents;
    mutable std::map<std::filesystem::path, ref_vector> content_list_info;

    decltype(content_list_info)::iterator mis_file_archive::cache_data(std::basic_istream<std::byte>& stream, const std::filesystem::path& archive_or_folder_path) const;

    static bool is_supported(std::basic_istream<std::byte>& stream);

    bool stream_is_supported(std::basic_istream<std::byte>& stream) const override;
    std::vector<content_info> get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const override;
    void set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const override;
    void extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const override;
  };
}// namespace studio::resources::mis::darkstar

#endif//INC_3SPACESTUDIO_MISSION_HPP
