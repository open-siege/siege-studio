#include <fstream>
#include <functional>
//#include <siege/resource/darkstar_volume.hpp>
#include <siege/content/gui/interface.hpp>
#include <siege/platform/stream.hpp>

namespace siege::gui::darkstar
{
  using gui_item_reader_map = tagged_item_map<gui_item>::tagged_item_reader_map;
  using gui_item_reader = tagged_item_reader<gui_item>;

  constexpr auto sim_control_tag = platform::to_tag<4>({ 'S', 'G', 'c', 't' });
  constexpr auto sim_active_control_tag = platform::to_tag<4>({ 'S', 'G', 'a', 'c' });
  constexpr auto sim_bitmap_control_tag = platform::to_tag<4>({ 'S', 'G', 'b', 'm' });
  constexpr auto sim_palette_control_tag = platform::to_tag<4>({ 'S', 'G', 'p', 'l' });
  constexpr auto sim_text_wrap_control_tag = platform::to_tag<4>({ 'S', 'G', 't', 'w' });
  constexpr auto sim_alt_text_wrap_control_tag = platform::to_tag<4>({ 'G', 'f', 't', 'w' });
  constexpr auto es_palette_control_tag = platform::to_tag<4>({ 'E', 'S', 'p', 'c' });
  constexpr auto es_text_wrap_control_tag = platform::to_tag<4>({ 'E', 'S', 't', 'w' });
  constexpr auto es_text_edit_control_tag = platform::to_tag<4>({ 'E', 'S', 't', 'e' });
  constexpr auto es_alt_text_edit_control_tag = platform::to_tag<4>({ 'S', 'e', 't', 'e' });
  constexpr auto es_text_list_control_tag = platform::to_tag<4>({ 'E', 'S', 't', 'l' });
  constexpr auto es_smacker_movie_control_tag = platform::to_tag<4>({ 'E', 'S', 's', 'm' });
  constexpr auto es_button_control_tag = platform::to_tag<4>({ 'E', 'S', 'b', 't' });
  constexpr auto es_hidden_button_control_tag = platform::to_tag<4>({ 'E', 'g', 'h', 'b' });
  constexpr auto es_tab_control_tag = platform::to_tag<4>({ 'G', 'T', 't', 'l' });
  constexpr auto es_tab_child_control_tag = platform::to_tag<4>({ 'G', 'T', 'c', 'l' });
  constexpr auto es_slider_control_tag = platform::to_tag<4>({ 'S', 'H', 's', 'l' });
  constexpr auto es_toggle_control_tag = platform::to_tag<4>({ 'E', 'S', 'y', 'n' });
  constexpr auto es_combo_control_tag = platform::to_tag<4>({ 'E', 'S', 'c', 'x' });
  constexpr auto sim_timer_control_tag = platform::to_tag<4>({ 'S', 'G', 't', 'm' });

  constexpr auto es_matrix_control_tag = platform::to_tag<4>({ 'G', 'b', 'm', 'c' });

  constexpr auto es_scroll_control_tag = platform::to_tag<4>({ 'S', 'H', 's', 'c' });
  constexpr auto es_scroll_control_alt_tag = platform::to_tag<4>({ 'S', 'G', 's', 'c' });
  constexpr auto sim_scroll_content_control_tag = platform::to_tag<4>({ 'S', 'G', 's', 'C' });
  constexpr auto sim_text_control_tag = platform::to_tag<4>({ 'S', 'G', 's', 't' });

  constexpr auto gial_control_tag = platform::to_tag<4>({ 'G', 'i', 'a', 'l' });
  constexpr auto es_picture_pack_control_tag = platform::to_tag<4>({ 'G', 'p', 'p', 'k' });
  constexpr auto es_bitmap_animation_control_tag = platform::to_tag<4>({ 'G', 'B', 'm', 'A' });
  constexpr auto eshm_control_tag = platform::to_tag<4>({ 'E', 'S', 'h', 'm' });
  constexpr auto es_encyclopedia_control_tag = platform::to_tag<4>({ 'S', 'e', 'c', 'l' });
  constexpr auto es_scannex_control_tag = platform::to_tag<4>({ 'S', 's', 'x', 'l' });
  constexpr auto es_map_control_tag = platform::to_tag<4>({ 'H', 'U', 'm', 'v' });
  constexpr auto es_irc_channel_list_control_tag = platform::to_tag<4>({ 'S', 'i', 'c', 'l' });
  constexpr auto es_irc_people_list_control_tag = platform::to_tag<4>({ 'S', 'i', 'p', 'l' });
  constexpr auto es_irc_sgir_control_tag = platform::to_tag<4>({ 'S', 'G', 'i', 'r' });
  constexpr auto es_irc_sgit_control_tag = platform::to_tag<4>({ 'S', 'G', 'i', 't' });
  constexpr auto gftf_control_tag = platform::to_tag<4>({ 'G', 'f', 't', 'f' });
  constexpr auto gsas_control_tag = platform::to_tag<4>({ 'G', 's', 'a', 's' });
  constexpr auto gsdi_control_tag = platform::to_tag<4>({ 'G', 'S', 'd', 'i' });

  // GC - maybe graphics canvas?
  // GCvc is inside VehLab.gui and is perhaps the area where the vehicle is drawn on screen.
  constexpr auto gcmb_control_tag = platform::to_tag<4>({ 'G', 'C', 'm', 'b' });
  constexpr auto gcvc_control_tag = platform::to_tag<4>({ 'G', 'C', 'v', 'c' });
  constexpr auto spau_control_tag = platform::to_tag<4>({ 'S', 'p', 'a', 'u' });
  constexpr auto ghan_control_tag = platform::to_tag<4>({ 'G', 'h', 'a', 'n' });
  constexpr auto gpsc_control_tag = platform::to_tag<4>({ 'G', 'p', 's', 'c' });

  bool is_interface_data(std::istream& file)
  {
    std::array<std::byte, 4> header{};

    platform::read(file, header.data(), sizeof(header));
    file.seekg(-int(sizeof(header)), std::ios::cur);

    return header == sim_bitmap_control_tag;
  }

  darkstar::sim_control read_sim_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers, bool read_version = false)
  {
    darkstar::sim_control group;
    group.header = header;
    group.version = 0;

    if (read_version)
    {
      platform::read(file, reinterpret_cast<char*>(&group.version), sizeof(group.version));
    }

    platform::read(file, &group.control_version, sizeof(group.control_version));

    const auto control_version = int(group.control_version);

    endian::little_uint32_t temp;
    platform::read(file, reinterpret_cast<char*>(&temp), sizeof(temp));
    platform::read(file, &group.opaque, sizeof(group.opaque));
    platform::read(file, &group.fill_colour, sizeof(group.fill_colour));

    if (control_version > 1)
    {
      platform::read(file, &group.selected_fill_colour, sizeof(group.selected_fill_colour));
      platform::read(file, &group.ghost_fill_colour, sizeof(group.ghost_fill_colour));
    }

    platform::read(file, &group.border, sizeof(group.border));
    platform::read(file, &group.border_color, sizeof(group.border_color));

    if (control_version > 1)
    {
      platform::read(file, &group.selected_border_color, sizeof(group.selected_border_color));
      platform::read(file, &group.ghost_border_color, sizeof(group.ghost_border_color));
    }

    if (control_version == 0)
    {
      endian::little_uint16_t temp2;
      platform::read(file, reinterpret_cast<char*>(&temp2), sizeof(temp2));
    }

    group.console_command = read_string(file);

    if (control_version > 2)
    {
      group.alt_console_command = read_string(file);
    }

    platform::read(file, reinterpret_cast<char*>(group.position.data()), sizeof(group.position));
    platform::read(file, reinterpret_cast<char*>(group.size.data()), sizeof(group.size));

    platform::read(file, reinterpret_cast<char*>(&group.flags), sizeof(group.flags));
    platform::read(file, reinterpret_cast<char*>(&group.tag), sizeof(group.tag));
    platform::read(file, reinterpret_cast<char*>(&group.horizontal_sizing), sizeof(group.horizontal_sizing));
    platform::read(file, reinterpret_cast<char*>(&group.vertical_sizing), sizeof(group.vertical_sizing));

    if (control_version > 3)
    {
      platform::read(file, reinterpret_cast<char*>(&group.help_tag), sizeof(group.help_tag));
    }

    group.console_variable = read_string(file, sim_control::inspect_size);

    platform::read(file, reinterpret_cast<char*>(&group.children_count), sizeof(group.children_count));

    group.children = read_children<gui_item>(file, group.children_count, readers);
    group.names = read_strings(file, group.children_count);
    skip_alignment_bytes(file, header.object_size);

    return group;
  }

  darkstar::sim_active_control read_sim_active_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers, bool read_version = false)
  {
    darkstar::sim_active_control control;
    control.version = 0;

    if (read_version)
    {
      platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    }

    platform::read(file, reinterpret_cast<char*>(&control.is_active), sizeof(control.is_active));
    platform::read(file, reinterpret_cast<char*>(&control.message), sizeof(control.message));

    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::sim_text_wrap_control read_sim_text_wrap_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers, bool read_version = false)
  {
    darkstar::sim_text_wrap_control control;
    control.version = 0;

    if (read_version)
    {
      platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    }

    platform::read(file, reinterpret_cast<char*>(&control.ignore_click), sizeof(control.ignore_click));
    platform::read(file, reinterpret_cast<char*>(&control.disabled_font_tag), sizeof(control.disabled_font_tag));
    platform::read(file, reinterpret_cast<char*>(&control.alignment), sizeof(control.alignment));
    platform::read(file, reinterpret_cast<char*>(&control.font_tag), sizeof(control.font_tag));
    platform::read(file, reinterpret_cast<char*>(&control.text_tag), sizeof(control.text_tag));
    platform::read(file, reinterpret_cast<char*>(&control.line_spacing), sizeof(control.line_spacing));

    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::sim_alt_text_wrap_control read_alt_sim_text_wrap_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::sim_alt_text_wrap_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));

    control.control_data = read_sim_text_wrap_control(file, header, readers);

    return control;
  }

  darkstar::sim_bitmap_control read_sim_bitmap_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::sim_bitmap_control control;
    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(&control.bitmap_tag), sizeof(control.bitmap_tag));
    control.inspection_data = read_string(file, sim_control::inspect_size);
    platform::read(file, &control.is_transparent, sizeof(control.is_transparent));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::es_palette_control read_es_palette_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_palette_control control;
    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(&control.palette_tag), sizeof(control.palette_tag));
    control.inspection_data = read_string(file, sim_control::inspect_size);
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::sim_timer_control read_sim_timer_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::sim_timer_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(&control.message), sizeof(control.message));
    platform::read(file, reinterpret_cast<char*>(&control.initial_timeout), sizeof(control.initial_timeout));
    platform::read(file, reinterpret_cast<char*>(&control.interval_timeout), sizeof(control.interval_timeout));

    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_hidden_button_control read_es_hidden_button_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_hidden_button_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.button_data.data(), sizeof(control.button_data));

    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::sim_text_control read_sim_text_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers, bool read_version = false)
  {
    darkstar::sim_text_control control;

    control.version = 0;

    if (read_version)
    {
      platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    }

    platform::read(file, reinterpret_cast<char*>(control.unused.data()), sizeof(control.unused));
    platform::read(file, reinterpret_cast<char*>(&control.disabled_font_tag), sizeof(control.disabled_font_tag));
    platform::read(file, reinterpret_cast<char*>(&control.font_tag), sizeof(control.font_tag));
    platform::read(file, reinterpret_cast<char*>(&control.alt_font_tag), sizeof(control.alt_font_tag));
    platform::read(file, reinterpret_cast<char*>(&control.text_tag), sizeof(control.text_tag));
    control.default_text = read_string(file, sim_control::inspect_size);
    platform::read(file, reinterpret_cast<char*>(&control.alignment), sizeof(control.alignment));
    platform::read(file, reinterpret_cast<char*>(&control.vertical_position_delta), sizeof(control.vertical_position_delta));

    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::es_button_control read_es_button_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_button_control control;
    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(control.button_data.data()), sizeof(control.button_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_wrap_control read_es_text_wrap_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_text_wrap_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(control.button_data.data()), sizeof(control.button_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_list_control read_es_text_list_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_text_list_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(control.list_data.data()), sizeof(control.list_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_edit_control read_es_text_edit_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_text_edit_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(&control.numbers_only), sizeof(control.numbers_only));
    platform::read(file, reinterpret_cast<char*>(control.unused.data()), sizeof(control.unused));
    platform::read(file, reinterpret_cast<char*>(&control.max_length), sizeof(control.max_length));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_scroll_control read_es_scroll_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_scroll_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(&control.scroll_pba_tag), sizeof(control.scroll_pba_tag));
    platform::read(file, &control.use_arrow_keys, sizeof(control.use_arrow_keys));
    platform::read(file, reinterpret_cast<char*>(&control.force_horizontal_scroll_bar), sizeof(control.force_horizontal_scroll_bar));
    platform::read(file, reinterpret_cast<char*>(&control.force_vertical_scroll_bar), sizeof(control.force_vertical_scroll_bar));
    platform::read(file, &control.use_constant_thumb_height, sizeof(control.use_constant_thumb_height));

    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_matrix_control read_es_matrix_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_matrix_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(control.header_size.data()), sizeof(control.header_size));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_smacker_movie_control read_es_smacker_movie_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_smacker_movie_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, reinterpret_cast<char*>(&control.video_tag), sizeof(control.video_tag));

    platform::read(file, &control.unknown1, sizeof(control.unknown1));
    platform::read(file, reinterpret_cast<char*>(&control.category_tag), sizeof(control.category_tag));
    platform::read(file, &control.unknown2, sizeof(control.unknown2));
    platform::read(file, control.unknown3.data(), sizeof(control.unknown3));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::gial_control read_gial_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::gial_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_picture_pack_control read_es_picture_pack_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_picture_pack_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::eshm_control read_eshm_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::eshm_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, &control.raw_data, sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_encyclopedia_control read_es_encyclopedia_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_encyclopedia_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::es_scannex_control read_es_scannex_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_scannex_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::es_map_control read_es_map_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_map_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_irc_channel_list_control read_es_irc_channel_list_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_irc_channel_list_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, &control.unknown1, sizeof(control.unknown1));
    platform::read(file, &control.unknown2, sizeof(control.unknown2));
    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_irc_people_list_control read_es_irc_people_list_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_irc_people_list_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, &control.unknown1, sizeof(control.unknown1));
    platform::read(file, &control.unknown2, sizeof(control.unknown2));
    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_irc_sgit_control read_es_irc_sgit_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_irc_sgit_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::gsas_control read_gsas_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::gsas_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::gsdi_control read_gsdi_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::gsdi_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::gcmb_control read_gcmb_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::gcmb_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::gcvc_control read_gcvc_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::gcvc_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::ghan_control read_ghan_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::ghan_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::gftf_control read_gftf_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::gftf_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    platform::read(file, reinterpret_cast<char*>(&control.unknown), sizeof(control.unknown));
    platform::read(file, reinterpret_cast<char*>(&control.font_tag), sizeof(control.font_tag));
    platform::read(file, reinterpret_cast<char*>(&control.alt_font_tag), sizeof(control.alt_font_tag));
    platform::read(file, control.raw_data2.data(), sizeof(control.raw_data2));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::es_bitmap_animation_control read_es_bitmap_animation_control(std::istream& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_bitmap_animation_control control;

    platform::read(file, reinterpret_cast<char*>(&control.version), sizeof(control.version));
    platform::read(file, control.raw_data.data(), sizeof(control.raw_data));
    platform::read(file, reinterpret_cast<char*>(&control.flags), sizeof(control.flags));
    platform::read(file, reinterpret_cast<char*>(&control.pba_tag), sizeof(control.pba_tag));
    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::gui_items read_interface_data(std::istream& file)
  {
    static gui_item_reader_map readers = {
      { sim_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_control(file, header, readers, true); } } },
      { spau_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_control(file, header, readers, true); } } },
      { gpsc_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_control(file, header, readers, true); } } },
      { sim_scroll_content_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_control(file, header, readers, true); } } },
      { sim_active_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_active_control(file, header, readers, true); } } },
      { sim_text_wrap_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_text_wrap_control(file, header, readers, true); } } },
      { sim_alt_text_wrap_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_alt_sim_text_wrap_control(file, header, readers); } } },
      { es_irc_sgir_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_active_control(file, header, readers, true); } } },
      { sim_text_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_text_control(file, header, readers, true); } } },
      { es_combo_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_text_control(file, header, readers, true); } } },
      { es_text_edit_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_text_edit_control(file, header, readers); } } },
      { es_alt_text_edit_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_text_edit_control(file, header, readers); } } },
      { sim_bitmap_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_bitmap_control(file, header, readers); } } },
      { es_palette_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_palette_control(file, header, readers); } } },
      { sim_palette_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_palette_control(file, header, readers); } } },
      { es_text_wrap_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_text_wrap_control(file, header, readers); } } },
      { es_button_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_button_control(file, header, readers); } } },
      { es_hidden_button_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_hidden_button_control(file, header, readers); } } },
      { es_text_list_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_text_list_control(file, header, readers); } } },
      { es_scroll_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_scroll_control(file, header, readers); } } },
      { es_scroll_control_alt_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_scroll_control(file, header, readers); } } },
      { es_matrix_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_matrix_control(file, header, readers); } } },
      { es_smacker_movie_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_smacker_movie_control(file, header, readers); } } },
      { sim_timer_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_timer_control(file, header, readers); } } },
      { es_picture_pack_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_picture_pack_control(file, header, readers); } } },
      { es_bitmap_animation_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_bitmap_animation_control(file, header, readers); } } },
      { es_encyclopedia_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_encyclopedia_control(file, header, readers); } } },
      { es_scannex_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_scannex_control(file, header, readers); } } },
      { es_map_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_map_control(file, header, readers); } } },
      { es_irc_channel_list_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_irc_channel_list_control(file, header, readers); } } },
      { es_irc_people_list_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_irc_people_list_control(file, header, readers); } } },
      { es_irc_sgit_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_irc_sgit_control(file, header, readers); } } },
      { gftf_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_gftf_control(file, header, readers); } } },
      { gsas_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_gsas_control(file, header, readers); } } },
      { gcmb_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_gcmb_control(file, header, readers); } } },
      { gcvc_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_gcvc_control(file, header, readers); } } },
      { gsdi_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_gsdi_control(file, header, readers); } } },
      { ghan_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_ghan_control(file, header, readers); } } },
      { eshm_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_eshm_control(file, header, readers); } } },
      { gial_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_gial_control(file, header, readers); } } }
    };

    return read_children<gui_item>(file, 1, readers);
  }
}// namespace siege::gui::darkstar
