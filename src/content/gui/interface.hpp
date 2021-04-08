#ifndef INC_3SPACESTUDIO_INTERFACE_HPP
#define INC_3SPACESTUDIO_INTERFACE_HPP

#include <vector>
#include <map>
#include <map>
#include <optional>
#include <istream>
#include <variant>
#include <filesystem>
#include "content/tagged_data.hpp"
#include "endian_arithmetic.hpp"
#include "shared.hpp"

namespace studio::gui::darkstar
{
  namespace endian = boost::endian;
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
  struct gial_control;
  struct es_picture_pack_control;
  struct es_bitmap_animation_control;
  struct eshm_control;
  struct es_encyclopedia_control;
  struct es_scannex_control;
  struct es_map_control;
  struct es_irc_channel_list_control;
  struct es_irc_people_list_control;
  struct es_irc_sgit_control;
  struct gftf_control;

  using gui_item = std::variant<sim_control, sim_bitmap_control, es_palette_control, es_text_wrap_control, sim_text_control,
    es_button_control,es_smacker_movie_control, es_hidden_button_control, sim_timer_control, sim_active_control, es_text_list_control,
    es_encyclopedia_control, es_scannex_control, es_map_control, gftf_control, es_irc_channel_list_control, es_irc_people_list_control,
    es_irc_sgit_control,
    es_scroll_control, es_matrix_control, es_text_edit_control, gial_control, es_picture_pack_control, es_bitmap_animation_control, eshm_control, raw_item>;

  using gui_items = std::vector<gui_item>;

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
    gui_items children;
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

  struct gial_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 56> raw_data;
    sim_control control_data;
  };

  struct es_picture_pack_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 36> raw_data;
    sim_active_control control_data;
  };

  struct es_bitmap_animation_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 40> raw_data;
    endian::little_uint32_t flags;
    endian::little_int32_t pba_tag;
    sim_text_control control_data;
  };

  struct eshm_control
  {
    endian::little_uint32_t version;
    std::byte raw_data;
    sim_control control_data;
  };

  struct es_encyclopedia_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 36> raw_data;
    sim_active_control control_data;
  };

  struct es_scannex_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 36> raw_data;
    sim_active_control control_data;
  };

  struct es_map_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 40> raw_data;
    sim_control control_data;
  };

  struct es_irc_channel_list_control
  {
    endian::little_uint32_t version;
    std::byte unknown1;
    std::byte unknown2;
    sim_text_control control_data;
  };

  struct es_irc_people_list_control
  {
    endian::little_uint32_t version;
    std::byte unknown1;
    std::byte unknown2;
    sim_text_control control_data;
  };

  struct es_irc_sgit_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 17> raw_data;
    sim_text_control control_data;
  };

  struct gftf_control
  {
    endian::little_uint32_t version;
    std::array<std::byte, 48> raw_data;
    endian::little_int32_t unknown;
    endian::little_int32_t font_tag;
    endian::little_int32_t alt_font_tag;
    std::array<std::byte, 32> raw_data2;
    sim_active_control control_data;
  };

  bool is_interface_data(std::basic_istream<std::byte>& file);

  gui_items read_interface_data(std::basic_istream<std::byte>& file);
}// namespace studio::mis::darkstar


#endif//INC_3SPACESTUDIO_MISSION_HPP
