#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <iostream>
#include "resources/darkstar_volume.hpp"
#include "mission.hpp"

namespace studio::mis::darkstar
{
  constexpr auto sim_group_tag = shared::to_tag<4>({ 'S', 'I', 'M', 'G' });
  constexpr auto sim_set_tag = shared::to_tag<4>({ 'S', 'I', 'M', 'S' });
  constexpr auto sim_vol_tag = shared::to_tag<4>({ 'S', 'V', 'o', 'l' });
  constexpr auto sim_terrain_tag = shared::to_tag<4>({ 'S', 'T', 'E', 'R' });
  constexpr auto es_palette_tag = shared::to_tag<4>({ 'E', 'S', 'p', 't' });
  constexpr auto interior_shape_tag = shared::to_tag<4>({ 0x0c, 0x01, 0x00, 0x00 });
  constexpr auto sim_structure_tag = shared::to_tag<4>({ 0x00, 0x01, 0x00, 0x00 });
  constexpr auto drop_point_tag = shared::to_tag<4>({ 'D', 'P', 'N', 'T' });
  constexpr auto nav_marker_tag = shared::to_tag<4>({ 'E', 'S', 'N', 'M' });
  constexpr auto sim_marker_tag = shared::to_tag<4>({ 'm', 'a', 'r', 'k' });
  constexpr auto herc_tag = shared::to_tag<4>({ 'H', 'E', 'R', 'C' });
  constexpr auto tank_tag = shared::to_tag<4>({ 'T', 'A', 'N', 'K' });
  constexpr auto flyer_tag = shared::to_tag<4>({ 'F', 'L', 'Y', 'R' });

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


  bool is_mission_data(std::basic_istream<std::byte>& file)
  {
    std::array<std::byte, 4> header{};

    file.read(header.data(), sizeof(header));
    file.seekg(-int(sizeof(header)), std::ios::cur);

    return header == sim_group_tag;
  }

  template<std::size_t Length>
  std::string hex_str(std::array<std::byte, Length>& data)
  {
    std::stringstream ss;
    ss << std::hex;

    for (auto item : data)
    {
      ss << std::setw(2) << std::setfill('0') << static_cast<std::uint16_t>(item);
    }

    return ss.str();
  }


  void skip_alignment_bytes(std::basic_istream<std::byte>& file, std::uint32_t base)
  {
    int offset = 0;
    while ((base + offset) % 2 != 0)
    {
      offset++;
    }

    if (offset > 0)
    {
      file.seekg(offset, std::ios_base::cur);
    }
  }

  darkstar::sim_items read_children(std::basic_istream<std::byte>& file, std::uint32_t children_count, darkstar::sim_item_reader_map& readers)
  {
    using volume_header = object_header;
    namespace endian = boost::endian;
    darkstar::sim_items children;

    children.reserve(children_count);

    for (auto i = 0u; i < children_count; ++i)
    {
      volume_header child_header;

      file.read(reinterpret_cast<std::byte*>(&child_header), sizeof(child_header));

      auto reader = readers.find(child_header.object_tag);

      if (reader != readers.end())
      {
        children.emplace_back(reader->second.read(file, child_header, readers));
        continue;
      }

      darkstar::raw_item item;

      std::cout << "Reading raw item at " << file.tellg() << '\n';

      item.header = child_header;
      item.raw_bytes = std::vector<std::byte>(item.header.object_size);
      file.read(item.raw_bytes.data(), item.header.object_size);

      children.emplace_back(std::move(item));

      skip_alignment_bytes(file, item.header.object_size);
    }

    return children;
  }

  std::string read_string(std::basic_istream<std::byte>& file, std::size_t size)
  {
    std::string name(size, '\0');

    file.read(reinterpret_cast<std::byte*>(name.data()), size);

    return name;
  }


  std::string read_string(std::basic_istream<std::byte>& file)
  {
    std::uint8_t size;
    file.read(reinterpret_cast<std::byte*>(&size), sizeof(std::uint8_t));

    std::string name(size, '\0');

    file.read(reinterpret_cast<std::byte*>(name.data()), size);

    return name;
  }

  std::vector<std::string> read_strings(std::basic_istream<std::byte>& file, std::uint32_t children_count)
  {
    std::vector<std::string> results;
    results.reserve(children_count);

    for (auto i = 0u; i < children_count; ++i)
    {
      std::uint8_t size;
      file.read(reinterpret_cast<std::byte*>(&size), sizeof(std::uint8_t));

      std::string& name = results.emplace_back(size, '\0');

      file.read(reinterpret_cast<std::byte*>(name.data()), size);
    }

    return results;
  }

  darkstar::sim_network_object read_sim_network_object(std::basic_istream<std::byte>& file)
  {
    darkstar::sim_network_object object{};

    file.read(reinterpret_cast<std::byte*>(&object), sizeof(object));

    return object;
  }

  darkstar::sim_volume read_sim_volume(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::sim_volume volume;
    volume.header = header;
    volume.base = read_sim_network_object(file);
    volume.filename = read_string(file);

    skip_alignment_bytes(file, header.object_size);

    return volume;
  }

  darkstar::sim_terrain read_sim_terrain(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::sim_terrain volume;
    volume.header = header;
    volume.base = read_sim_network_object(file);

    file.read(volume.data.data(), volume.data.size());
    volume.dtf_file = read_string(file);
    file.read(volume.footer.data(), volume.footer.size());

    skip_alignment_bytes(file, header.object_size);

    return volume;
  }

  darkstar::sim_palette read_sim_palette(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::sim_palette palette;
    palette.header = header;
    palette.base = read_sim_network_object(file);

    palette.palette_file = read_string(file);

    skip_alignment_bytes(file, header.object_size);

    return palette;
  }

  darkstar::interior_shape read_interior_shape(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::interior_shape shape;

    shape.header = header;
    file.read(reinterpret_cast<std::byte*>(&shape.version), sizeof(shape.version));
    file.read(shape.data.data(), shape.data.size());
    file.read(reinterpret_cast<std::byte*>(&shape.string_length), sizeof(shape.string_length));

    shape.filename = std::string(shape.string_length, '\0');

    file.read(reinterpret_cast<std::byte*>(shape.filename.data()), shape.filename.size());

    file.read(shape.footer.data(), shape.footer.size());

    skip_alignment_bytes(file, header.object_size);

    return shape;
  }

  darkstar::sim_marker read_sim_marker(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::sim_marker marker{};
    marker.header = header;
    marker.base = read_sim_network_object(file);

    file.read(reinterpret_cast<std::byte*>(&marker.transformation), sizeof(marker.transformation));

    skip_alignment_bytes(file, header.object_size);

    return marker;
  }

  darkstar::nav_marker read_nav_marker(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::nav_marker marker{};
    marker.header = header;
    marker.base = read_sim_network_object(file);

    file.read(reinterpret_cast<std::byte*>(&marker.transformation), sizeof(marker.transformation));
    file.read(marker.footer.data(), marker.footer.size());

    skip_alignment_bytes(file, header.object_size);

    return marker;
  }

  darkstar::vehicle read_vehicle(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::vehicle vehicle;
    vehicle.header = header;

    const auto footer_length = header.object_size - sizeof(vehicle.version) - vehicle.data.size() - darkstar::vehicle::type_fields_size;

    file.read(reinterpret_cast<std::byte*>(&vehicle.version), sizeof(vehicle.version));
    file.read(vehicle.data.data(), vehicle.data.size());
    file.read(reinterpret_cast<std::byte*>(&vehicle.vehicle_type), darkstar::vehicle::type_fields_size);

    vehicle.footer = std::vector<std::byte>(footer_length);
    file.read(vehicle.footer.data(), vehicle.footer.size());

    skip_alignment_bytes(file, header.object_size);

    return vehicle;
  }

  void write_vehicle(const darkstar::vehicle& vehicle, std::basic_ostream<std::byte>& output)
  {
    output.write(reinterpret_cast<const std::byte*>(&vehicle.header), sizeof(vehicle.header));
    output.write(reinterpret_cast<const std::byte*>(&vehicle.version), sizeof(vehicle.version));
    output.write(vehicle.data.data(), vehicle.data.size());
    output.write(reinterpret_cast<const std::byte*>(&vehicle.vehicle_type), darkstar::vehicle::type_fields_size);
    output.write(vehicle.footer.data(), vehicle.footer.size());
  }

  darkstar::sim_control read_sim_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers, bool read_version = false)
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

    group.children = read_children(file, group.children_count, readers);
    group.names = read_strings(file, group.children_count);
    skip_alignment_bytes(file, header.object_size);

    return group;
  }

  darkstar::sim_active_control read_sim_active_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers, bool read_version = false)
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

  darkstar::sim_bitmap_control read_sim_bitmap_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::sim_bitmap_control control;
    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.bitmap_tag), sizeof(control.bitmap_tag));
    control.inspection_data = read_string(file, sim_control::inspect_size);
    file.read(&control.is_transparent, sizeof(control.is_transparent));
    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }

  darkstar::es_palette_control read_es_palette_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::es_palette_control control;
    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.palette_tag), sizeof(control.palette_tag));
    control.inspection_data = read_string(file, sim_control::inspect_size);
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::sim_timer_control read_sim_timer_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::sim_timer_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.message), sizeof(control.message));
    file.read(reinterpret_cast<std::byte*>(&control.initial_timeout), sizeof(control.initial_timeout));
    file.read(reinterpret_cast<std::byte*>(&control.interval_timeout), sizeof(control.interval_timeout));

    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_hidden_button_control read_es_hidden_button_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::es_hidden_button_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(control.button_data.data(), sizeof(control.button_data));

    control.control_data = read_sim_active_control(file, header, readers);

    return control;
  }


  darkstar::sim_text_control read_sim_text_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers, bool read_version = false)
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

  darkstar::es_button_control read_es_button_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::es_button_control control;
    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(control.button_data.data()), sizeof(control.button_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_wrap_control read_es_text_wrap_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::es_text_wrap_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(control.button_data.data()), sizeof(control.button_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_list_control read_es_text_list_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::es_text_list_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(control.list_data.data()), sizeof(control.list_data));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_text_edit_control read_es_text_edit_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::es_text_edit_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(&control.numbers_only), sizeof(control.numbers_only));
    file.read(reinterpret_cast<std::byte*>(control.unused.data()), sizeof(control.unused));
    file.read(reinterpret_cast<std::byte*>(&control.max_length), sizeof(control.max_length));

    control.control_data = read_sim_text_control(file, header, readers);

    return control;
  }

  darkstar::es_scroll_control read_es_scroll_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
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

  darkstar::es_matrix_control read_es_matrix_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::es_matrix_control control;

    file.read(reinterpret_cast<std::byte*>(&control.version), sizeof(control.version));
    file.read(reinterpret_cast<std::byte*>(control.header_size.data()), sizeof(control.header_size));
    file.read(control.raw_data.data(), sizeof(control.raw_data));
    control.control_data = read_sim_control(file, header, readers);

    return control;
  }

  darkstar::es_smacker_movie_control read_es_smacker_movie_control(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
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

  darkstar::sim_group read_sim_group(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::sim_group group;
    group.header = header;

    file.read(reinterpret_cast<std::byte*>(&group.version), sizeof(group.version));
    file.read(reinterpret_cast<std::byte*>(&group.children_count), sizeof(group.children_count));

    group.children = read_children(file, group.children_count, readers);
    group.names = read_strings(file, group.children_count);
    skip_alignment_bytes(file, header.object_size);

    return group;
  }

  darkstar::sim_set read_sim_set(std::basic_istream<std::byte>& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::sim_set set;
    set.header = header;
    file.read(reinterpret_cast<std::byte*>(&set.item_id), sizeof(set.item_id));
    file.read(reinterpret_cast<std::byte*>(&set.children_count), sizeof(set.children_count));
    set.children = read_children(file, set.children_count, readers);
    skip_alignment_bytes(file, header.object_size);

    return set;
  }

  darkstar::sim_items read_mission_data(std::basic_istream<std::byte>& file)
  {
    static sim_item_reader_map readers = {
      { sim_group_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_group(file, header, readers); } } },
      { sim_set_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_set(file, header, readers); } } },
      { sim_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_control(file, header, readers, true); } } },
      { sim_scroll_content_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_control(file, header, readers, true); } } },
      { sim_active_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_active_control(file, header, readers, true); } } },
      { sim_text_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_text_control(file, header, readers, true); } } },
      { es_combo_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_text_control(file, header, readers, true); } } },
      { es_text_edit_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_text_edit_control(file, header, readers); } } },
      { sim_bitmap_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_bitmap_control(file, header, readers); } } },
      { es_palette_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_palette_control(file, header, readers); } } },
      { sim_palette_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_palette_control(file, header, readers); } } },
      { es_text_wrap_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_text_wrap_control(file, header, readers); } } },
      { es_button_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_button_control(file, header, readers); } } },
      { es_hidden_button_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_hidden_button_control(file, header, readers); } } },
      { es_text_list_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_text_list_control(file, header, readers); } } },
      { es_scroll_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_scroll_control(file, header, readers); } } },
      { es_matrix_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_matrix_control(file, header, readers); } } },
      { es_smacker_movie_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_es_smacker_movie_control(file, header, readers); } } },
      { sim_timer_control_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_timer_control(file, header, readers); } } },
      //TODO come back to these at another time.
      // Some of the tags (interior shape being the most common) have issues when parsing
//      { sim_vol_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_volume(file, header, readers); } } },
//      { sim_terrain_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_terrain(file, header, readers); } } },
//      { es_palette_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_palette(file, header, readers); } } },
//      { interior_shape_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_interior_shape(file, header, readers); } } },
//      { drop_point_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_marker(file, header, readers); } } },
//      { sim_marker_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_marker(file, header, readers); } } },
//      { nav_marker_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_nav_marker(file, header, readers); } } },
      { herc_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_vehicle(file, header, readers); } } },
      { tank_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_vehicle(file, header, readers); } } },
      { flyer_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_vehicle(file, header, readers); } } }
    };

    return read_children(file, 1, readers);
  }
}// namespace studio::mis::darkstar

namespace studio::resources::mis::darkstar
{
  using namespace studio::mis::darkstar;


  bool mis_file_archive::is_supported(std::basic_istream<std::byte>& stream)
  {
    return is_mission_data(stream);
  }

  bool mis_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream) const
  {
    return is_mission_data(stream);
  }

  std::filesystem::path get_archive_path(const std::filesystem::path& archive_or_folder_path)
  {
    auto archive_path = archive_or_folder_path;

    constexpr auto depth = 20;

    for (auto i = 0; i < depth; ++i)
    {
      if (shared::to_lower(archive_path.extension().string()) == ".mis")
      {
        break;
      }
      archive_path = archive_path.parent_path();
    }

    return archive_path;
  }

  decltype(mis_file_archive::content_list_info)::iterator mis_file_archive::cache_data(std::basic_istream<std::byte>& stream,
    const std::filesystem::path& archive_or_folder_path) const
  {
    auto archive_path = get_archive_path(archive_or_folder_path);
    auto existing_items = contents.find(archive_path);

    std::size_t position = 0;

    if (existing_items == contents.end())
    {
      position = stream.tellg();
      existing_items = contents.emplace(std::make_pair(archive_path, read_mission_data(stream))).first;
    }

    auto existing_info = content_list_info.find(archive_path);

    if (existing_info == content_list_info.end())
    {
      existing_info = content_list_info.emplace(std::make_pair(archive_path, ref_vector())).first;
      existing_info->second.reserve(20);

      using item_and_name = std::pair<std::filesystem::path, std::reference_wrapper<sim_item>>;
      using optional_item = std::optional<item_and_name>;

      std::function<void(optional_item, item_and_name)> visit_item = [&](optional_item parent, item_and_name child) {
        std::visit([&](auto& real_child) {
          using item_type = std::decay_t<decltype(real_child)>;
          if constexpr (std::is_same_v<item_type, sim_group>)
          {
            auto& child_as_group = static_cast<sim_group&>(real_child);

            mis_file_archive::folder_info folder{};
            folder.full_path = child.first;
            folder.name = child.first.filename().string();
            folder.file_count = child_as_group.children.size();

            existing_info->second.emplace_back(std::make_pair(child.second, folder));

            if (child_as_group.children.size() == child_as_group.names.size())
            {
              for (auto i = 0u; i < child_as_group.children.size(); ++i)
              {
                visit_item(child, std::make_pair(child.first / child_as_group.names[i], std::ref(child_as_group.children[i])));
              }
            }
            return;
          }

          if constexpr (std::is_same_v<item_type, sim_set>)
          {
            auto& child_as_set = static_cast<sim_set&>(real_child);

            mis_file_archive::folder_info folder{};
            folder.full_path = child.first;
            folder.name = child.first.filename().string();
            folder.file_count = child_as_set.children.size();

            existing_info->second.emplace_back(std::make_pair(child.second, folder));

            for (auto i = 0u; i < child_as_set.children.size(); ++i)
            {
              visit_item(child, std::make_pair(child.first / std::to_string(i), std::ref(child_as_set.children[i])));
            }
            return;
          }

          if constexpr (std::is_same_v<item_type, vehicle>)
          {
            if (parent.has_value())
            {
              auto& child_as_vehicle = static_cast<vehicle&>(real_child);
              mis_file_archive::file_info file{};
              file.folder_path = child.first.parent_path();
              file.filename = child.first.filename().replace_extension(".veh");
              file.size = sizeof(object_header) + child_as_vehicle.header.object_size;
              file.offset = position;
              file.compression_type = compression_type::none;

              existing_info->second.emplace_back(std::make_pair(child.second, file));
            }
          }
        },
          child.second.get());
      };

      for (auto& result : existing_items->second)
      {
        visit_item(std::nullopt, std::make_pair(archive_path, std::ref(result)));
      }
    }

    return existing_info;
  }

  std::vector<mis_file_archive::content_info> mis_file_archive::get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const
  {
    auto existing_info = cache_data(stream, archive_or_folder_path);
    std::vector<mis_file_archive::content_info> final_results;
    final_results.reserve(existing_info->second.size());

    for (auto& ref : existing_info->second)
    {
      std::visit([&](auto& info) {
        using info_type = std::decay_t<decltype(info)>;

        if constexpr (std::is_same_v<info_type, studio::resources::file_info>)
        {
          if (info.folder_path == archive_or_folder_path)
          {
            final_results.emplace_back(ref.second);
          }
        }

        if constexpr (std::is_same_v<info_type, studio::resources::folder_info>)
        {
          if (info.full_path.parent_path() == archive_or_folder_path)
          {
            final_results.emplace_back(ref.second);
          }
        }
      },
        ref.second);
    }

    return final_results;
  }

  void mis_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const
  {
    stream.seekg(info.offset, std::ios::beg);
  }

  void mis_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const
  {
    set_stream_position(stream, info);
    auto existing_info = cache_data(stream, info.folder_path);

    for (auto& ref : existing_info->second)
    {
      if (!std::visit([&](auto& child_info) -> bool {
            if constexpr (std::is_same_v<std::decay_t<decltype(child_info)>, studio::resources::file_info>)
            {
              if (child_info.folder_path == info.folder_path && child_info.filename == info.filename)
              {
                return std::visit([&](auto& data) -> bool {
                  if constexpr (std::is_same_v<std::decay_t<decltype(data)>, vehicle>)
                  {
                    write_vehicle(data, output);
                    return false;
                  }
                  return true;
                },
                  ref.first.get());
              }
            }

            return true;
          },
            ref.second))
      {
        break;
      }
    }
  }
}// namespace studio::resources::mis::darkstar
