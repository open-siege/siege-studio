#include <fstream>
#include <functional>
#include "resources/darkstar_volume.hpp"
#include "interface.hpp"

namespace studio::gui::darkstar
{
  using gui_item_reader_map = tagged_item_map<gui_item>::tagged_item_reader_map;
  using gui_item_reader = tagged_item_reader<gui_item>;

  constexpr auto sim_control_tag = shared::to_tag<4>({ 'S', 'G', 'c', 't' });
  constexpr auto sim_active_control_tag = shared::to_tag<4>({ 'S', 'G', 'a', 'c' });
  constexpr auto sim_bitmap_control_tag = shared::to_tag<4>({ 'S', 'G', 'b', 'm' });
  constexpr auto sim_palette_control_tag = shared::to_tag<4>({ 'S', 'G', 'p', 'l' });
  constexpr auto es_palette_control_tag = shared::to_tag<4>({ 'E', 'S', 'p', 'c' });
  constexpr auto es_text_wrap_control_tag = shared::to_tag<4>({ 'E', 'S', 't', 'w' });
  constexpr auto es_text_edit_control_tag = shared::to_tag<4>({ 'E', 'S', 't', 'e' });
  constexpr auto es_text_list_control_tag = shared::to_tag<4>({ 'E', 'S', 't', 'l' });
  constexpr auto es_smacker_movie_control_tag = shared::to_tag<4>({ 'E', 'S', 's', 'm' });
  constexpr auto es_button_control_tag = shared::to_tag<4>({ 'E', 'S', 'b', 't' });
  constexpr auto es_hidden_button_control_tag = shared::to_tag<4>({ 'E', 'g', 'h', 'b' });
  constexpr auto es_tab_control_tag = shared::to_tag<4>({ 'G', 'T', 't', 'l' });
  constexpr auto es_tab_child_control_tag = shared::to_tag<4>({ 'G', 'T', 'c', 'l' });
  constexpr auto es_slider_control_tag = shared::to_tag<4>({ 'S', 'H', 's', 'l' });
  constexpr auto es_toggle_control_tag = shared::to_tag<4>({ 'E', 'S', 'y', 'n' });
  constexpr auto es_combo_control_tag = shared::to_tag<4>({ 'E', 'S', 'c', 'x' });
  constexpr auto sim_timer_control_tag = shared::to_tag<4>({ 'S', 'G', 't', 'm' });

  constexpr auto es_matrix_control_tag = shared::to_tag<4>({ 'G', 'b', 'm', 'c' });

  constexpr auto es_scroll_control_tag = shared::to_tag<4>({ 'S', 'H', 's', 'c' });
  constexpr auto sim_scroll_content_control_tag = shared::to_tag<4>({ 'S', 'G', 's', 'C' });
  constexpr auto sim_text_control_tag = shared::to_tag<4>({ 'S', 'G', 's', 't' });


  bool is_interface_data(std::basic_istream<std::byte>& file)
  {
    std::array<std::byte, 4> header{};

    file.read(header.data(), sizeof(header));
    file.seekg(-int(sizeof(header)), std::ios::cur);

    return header == sim_bitmap_control_tag;
  }

  darkstar::sim_control read_sim_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers, bool read_version = false)
  {
    darkstar::sim_control group;
    group.header = header;
    group.version = 0;

    if (read_version)
    {
      file.read(reinterpret_cast<std::byte*>(&group.version), sizeof(group.version));
    }

    file.read(&group.control_version, sizeof(group.control_version));

    if (int(group.control_version) < 4)
    {
      throw std::invalid_argument("Control being parsed has wrong version.");
    }

    endian::little_uint32_t temp;
    file.read(reinterpret_cast<std::byte*>(&temp), sizeof(temp));
    file.read(&group.opaque, sizeof(group.opaque));
    file.read(&group.fill_colour, sizeof(group.fill_colour));
    file.read(&group.selected_fill_colour, sizeof(group.selected_fill_colour));
    file.read(&group.ghost_fill_colour, sizeof(group.ghost_fill_colour));
    file.read(&group.border, sizeof(group.border));
    file.read(&group.border_color, sizeof(group.border_color));
    file.read(&group.selected_border_color, sizeof(group.selected_border_color));
    file.read(&group.ghost_border_color, sizeof(group.ghost_border_color));

    group.console_command = read_string(file);
    group.alt_console_command = read_string(file);

    file.read(reinterpret_cast<std::byte*>(group.position.data()), sizeof(group.position));
    file.read(reinterpret_cast<std::byte*>(group.size.data()), sizeof(group.size));

    file.read(reinterpret_cast<std::byte*>(&group.flags), sizeof(group.flags));
    file.read(reinterpret_cast<std::byte*>(&group.tag), sizeof(group.tag));
    file.read(reinterpret_cast<std::byte*>(&group.horizontal_sizing), sizeof(group.horizontal_sizing));
    file.read(reinterpret_cast<std::byte*>(&group.vertical_sizing), sizeof(group.vertical_sizing));
    file.read(reinterpret_cast<std::byte*>(&group.help_tag), sizeof(group.help_tag));

    group.console_variable = read_string(file, sim_control::inspect_size);

    file.read(reinterpret_cast<std::byte*>(&group.children_count), sizeof(group.children_count));

    group.children = read_children<gui_item>(file, group.children_count, readers);
    group.names = read_strings(file, group.children_count);
    skip_alignment_bytes(file, header.object_size);

    return group;
  }

  darkstar::sim_active_control read_sim_active_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers, bool read_version = false)
  {
    darkstar::sim_active_control control;
    control.version = 0;

    if (read_version)
    {
      file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    }

    file.read(reinterpret_cast<std::byte*>(&control.is_active), sizeof(control.is_active));
    file.read(reinterpret_cast<std::byte*>(&control.message), sizeof(control.message));

    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::sim_bitmap_control read_sim_bitmap_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::sim_bitmap_control control;
    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.bitmap_tag), sizeof(control.bitmap_tag));
    control.inspection_data = read_string(file, sim_control::inspect_size);
    file.read(&control.is_transparent, sizeof(control.is_transparent));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::es_palette_control read_es_palette_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_palette_control control;
    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.palette_tag), sizeof(control.palette_tag));
    control.inspection_data = read_string(file, sim_control::inspect_size);
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::sim_timer_control read_sim_timer_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::sim_timer_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.message), sizeof(control.message));
    file.read(reinterpret_cast<std::byte*>(&control.initial_timeout), sizeof(control.initial_timeout));
    file.read(reinterpret_cast<std::byte*>(&control.interval_timeout), sizeof(control.interval_timeout));

    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_hidden_button_control read_es_hidden_button_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_hidden_button_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(control.button_data.data(), sizeof(control.button_data));

    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::sim_text_control read_sim_text_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers, bool read_version = false)
  {
    darkstar::sim_text_control control;

    control.version = 0;

    if (read_version)
    {
      file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    }

    file.read(reinterpret_cast<std::byte*>(control.unused.data()), sizeof(control.unused));
    file.read(reinterpret_cast<std::byte*>(&control.disabled_font_tag), sizeof(control.disabled_font_tag));
    file.read(reinterpret_cast<std::byte*>(&control.font_tag), sizeof(control.font_tag));
    file.read(reinterpret_cast<std::byte*>(&control.alt_font_tag), sizeof(control.alt_font_tag));
    file.read(reinterpret_cast<std::byte*>(&control.text_tag), sizeof(control.text_tag));
    control.default_text = read_string(file, sim_control::inspect_size);
    file.read(reinterpret_cast<std::byte*>(&control.alignment), sizeof(control.alignment));
    file.read(reinterpret_cast<std::byte*>(&control.vertical_position_delta), sizeof(control.vertical_position_delta));

    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::es_button_control read_es_button_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_button_control control;
    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(control.button_data.data()), sizeof(control.button_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_wrap_control read_es_text_wrap_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_text_wrap_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(control.button_data.data()), sizeof(control.button_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_list_control read_es_text_list_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_text_list_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(control.list_data.data()), sizeof(control.list_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_edit_control read_es_text_edit_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_text_edit_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.numbers_only), sizeof(control.numbers_only));
    file.read(reinterpret_cast<std::byte*>(control.unused.data()), sizeof(control.unused));
    file.read(reinterpret_cast<std::byte*>(&control.max_length), sizeof(control.max_length));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_scroll_control read_es_scroll_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_scroll_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.scroll_pba_tag), sizeof(control.scroll_pba_tag));
    file.read(&control.use_arrow_keys, sizeof(control.use_arrow_keys));
    file.read(reinterpret_cast<std::byte*>(&control.force_horizontal_scroll_bar), sizeof(control.force_horizontal_scroll_bar));
    file.read(reinterpret_cast<std::byte*>(&control.force_vertical_scroll_bar), sizeof(control.force_vertical_scroll_bar));
    file.read(&control.use_constant_thumb_height, sizeof(control.use_constant_thumb_height));

    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_matrix_control read_es_matrix_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_matrix_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(control.header_size.data()), sizeof(control.header_size));
    file.read(control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_smacker_movie_control read_es_smacker_movie_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::es_smacker_movie_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.video_tag), sizeof(control.video_tag));

    file.read(&control.unknown1, sizeof(control.unknown1));
    file.read(reinterpret_cast<std::byte*>(&control.category_tag), sizeof(control.category_tag));
    file.read(&control.unknown2, sizeof(control.unknown2));
    file.read(control.unknown3.data(), sizeof(control.unknown3));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::sim_group read_sim_group(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::sim_group group;
    group.header = header;

    file.read(reinterpret_cast<std::byte*>(&group.version), sizeof(group.version));
    file.read(reinterpret_cast<std::byte*>(&group.children_count), sizeof(group.children_count));

    group.children = read_children<gui_item>(file, group.children_count, readers);
    group.names = read_strings(file, group.children_count);
    skip_alignment_bytes(file, header.object_size);

    return group;
  }

  darkstar::sim_set read_sim_set(std::basic_istream<std::byte>& file, object_header& header, darkstar::gui_item_reader_map& readers)
  {
    darkstar::sim_set set;
    set.header = header;
    file.read(reinterpret_cast<std::byte*>(&set.item_id), sizeof(set.item_id));
    file.read(reinterpret_cast<std::byte*>(&set.children_count), sizeof(set.children_count));
    set.children = read_children<gui_item>(file, set.children_count, readers);
    skip_alignment_bytes(file, header.object_size);

    return set;
  }

  darkstar::gui_items read_interface_data(std::basic_istream<std::byte>& file)
  {
    static gui_item_reader_map readers = {
      { sim_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_control(file, header, readers, true); } } },
      { sim_scroll_content_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_control(file, header, readers, true); } } },
      { sim_active_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_active_control(file, header, readers, true); } } },
      { sim_text_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_text_control(file, header, readers, true); } } },
      { es_combo_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_text_control(file, header, readers, true); } } },
      { es_text_edit_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_text_edit_control(file, header, readers); } } },
      { sim_bitmap_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_bitmap_control(file, header, readers); } } },
      { es_palette_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_palette_control(file, header, readers); } } },
      { sim_palette_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_palette_control(file, header, readers); } } },
      { es_text_wrap_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_text_wrap_control(file, header, readers); } } },
      { es_button_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_button_control(file, header, readers); } } },
      { es_hidden_button_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_hidden_button_control(file, header, readers); } } },
      { es_text_list_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_text_list_control(file, header, readers); } } },
      { es_scroll_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_scroll_control(file, header, readers); } } },
      { es_matrix_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_matrix_control(file, header, readers); } } },
      { es_smacker_movie_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_es_smacker_movie_control(file, header, readers); } } },
      { sim_timer_control_tag, { [](auto& file, auto& header, auto& readers) -> gui_item { return read_sim_timer_control(file, header, readers); } } }
    };

    return read_children<gui_item>(file, 1, readers);
  }
}// namespace studio::gui::darkstar
