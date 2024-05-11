#include <fstream>
#include <sstream>
#include <functional>
//#include <siege/resource/darkstar_volume.hpp>
#include <siege/content/mis/mission.hpp>
#include <siege/platform/stream.hpp>

namespace siege::mis::darkstar
{
  using sim_item_reader_map = tagged_item_map<sim_item>::tagged_item_reader_map;

  constexpr auto sim_group_tag = platform::to_tag<4>({ 'S', 'I', 'M', 'G' });
  constexpr auto sim_set_tag = platform::to_tag<4>({ 'S', 'I', 'M', 'S' });
  constexpr auto sim_vol_tag = platform::to_tag<4>({ 'S', 'V', 'o', 'l' });
  constexpr auto sim_terrain_tag = platform::to_tag<4>({ 'S', 'T', 'E', 'R' });
  constexpr auto es_palette_tag = platform::to_tag<4>({ 'E', 'S', 'p', 't' });
  constexpr auto interior_shape_tag = platform::to_tag<4>({ 0x0c, 0x01, 0x00, 0x00 });
  constexpr auto sim_structure_tag = platform::to_tag<4>({ 0x00, 0x01, 0x00, 0x00 });
  constexpr auto drop_point_tag = platform::to_tag<4>({ 'D', 'P', 'N', 'T' });
  constexpr auto nav_marker_tag = platform::to_tag<4>({ 'E', 'S', 'N', 'M' });
  constexpr auto sim_marker_tag = platform::to_tag<4>({ 'm', 'a', 'r', 'k' });
  constexpr auto herc_tag = platform::to_tag<4>({ 'H', 'E', 'R', 'C' });
  constexpr auto tank_tag = platform::to_tag<4>({ 'T', 'A', 'N', 'K' });
  constexpr auto flyer_tag = platform::to_tag<4>({ 'F', 'L', 'Y', 'R' });

  bool is_mission_data(std::istream& file)
  {
    std::array<std::byte, 4> header{};

    platform::read(file, header.data(), sizeof(header));
    file.seekg(-int(sizeof(header)), std::ios::cur);

    return header == sim_group_tag;
  }

  darkstar::sim_network_object read_sim_network_object(std::istream& file)
  {
    darkstar::sim_network_object object{};

    platform::read(file, reinterpret_cast<char*>(&object), sizeof(object));

    return object;
  }

  darkstar::sim_volume read_sim_volume(std::istream& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::sim_volume volume;
    volume.header = header;
    volume.base = read_sim_network_object(file);
    volume.filename = read_string(file);

    skip_alignment_bytes(file, header.object_size);

    return volume;
  }

  darkstar::sim_terrain read_sim_terrain(std::istream& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::sim_terrain volume;
    volume.header = header;
    volume.base = read_sim_network_object(file);

    platform::read(file, volume.data.data(), volume.data.size());
    volume.dtf_file = read_string(file);
    platform::read(file, volume.footer.data(), volume.footer.size());

    skip_alignment_bytes(file, header.object_size);

    return volume;
  }

  darkstar::sim_palette read_sim_palette(std::istream& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::sim_palette palette;
    palette.header = header;
    palette.base = read_sim_network_object(file);

    palette.palette_file = read_string(file);

    skip_alignment_bytes(file, header.object_size);

    return palette;
  }

  darkstar::interior_shape read_interior_shape(std::istream& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::interior_shape shape;

    shape.header = header;
    platform::read(file, reinterpret_cast<char*>(&shape.version), sizeof(shape.version));
    platform::read(file, shape.data.data(), shape.data.size());
    platform::read(file, reinterpret_cast<char*>(&shape.string_length), sizeof(shape.string_length));

    shape.filename = std::string(shape.string_length, '\0');

    platform::read(file, reinterpret_cast<char*>(shape.filename.data()), shape.filename.size());

    platform::read(file, shape.footer.data(), shape.footer.size());

    skip_alignment_bytes(file, header.object_size);

    return shape;
  }

  darkstar::sim_marker read_sim_marker(std::istream& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::sim_marker marker{};
    marker.header = header;
    marker.base = read_sim_network_object(file);

    platform::read(file, reinterpret_cast<char*>(&marker.transformation), sizeof(marker.transformation));

    skip_alignment_bytes(file, header.object_size);

    return marker;
  }

  darkstar::nav_marker read_nav_marker(std::istream& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::nav_marker marker{};
    marker.header = header;
    marker.base = read_sim_network_object(file);

    platform::read(file, reinterpret_cast<char*>(&marker.transformation), sizeof(marker.transformation));
    platform::read(file, marker.footer.data(), marker.footer.size());

    skip_alignment_bytes(file, header.object_size);

    return marker;
  }

  darkstar::vehicle read_vehicle(std::istream& file, object_header& header, darkstar::sim_item_reader_map&)
  {
    darkstar::vehicle vehicle;
    vehicle.header = header;

    const auto footer_length = header.object_size - sizeof(vehicle.version) - vehicle.data.size() - darkstar::vehicle::type_fields_size;

    platform::read(file, reinterpret_cast<char*>(&vehicle.version), sizeof(vehicle.version));
    platform::read(file, vehicle.data.data(), vehicle.data.size());
    platform::read(file, reinterpret_cast<char*>(&vehicle.vehicle_type), darkstar::vehicle::type_fields_size);

    vehicle.footer = std::vector<std::byte>(footer_length);
    platform::read(file, vehicle.footer.data(), vehicle.footer.size());

    skip_alignment_bytes(file, header.object_size);

    return vehicle;
  }

  void write_vehicle(const darkstar::vehicle& vehicle, std::ostream& output)
  {
    output.write(reinterpret_cast<const char*>(&vehicle.header), sizeof(vehicle.header));
    output.write(reinterpret_cast<const char*>(&vehicle.version), sizeof(vehicle.version));
    output.write(reinterpret_cast<const char*>(vehicle.data.data()), vehicle.data.size());
    output.write(reinterpret_cast<const char*>(&vehicle.vehicle_type), darkstar::vehicle::type_fields_size);
    output.write(reinterpret_cast<const char*>(vehicle.footer.data()), vehicle.footer.size());
  }

  darkstar::sim_group read_sim_group(std::istream& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::sim_group group;
    group.header = header;

    platform::read(file, reinterpret_cast<char*>(&group.version), sizeof(group.version));
    platform::read(file, reinterpret_cast<char*>(&group.children_count), sizeof(group.children_count));

    group.children = read_children<sim_item>(file, group.children_count, readers);
    group.names = read_strings(file, group.children_count);
    skip_alignment_bytes(file, header.object_size);

    return group;
  }

  darkstar::sim_set read_sim_set(std::istream& file, object_header& header, darkstar::sim_item_reader_map& readers)
  {
    darkstar::sim_set set;
    set.header = header;
    platform::read(file, reinterpret_cast<char*>(&set.item_id), sizeof(set.item_id));
    platform::read(file, reinterpret_cast<char*>(&set.children_count), sizeof(set.children_count));
    set.children = read_children<sim_item>(file, set.children_count, readers);
    skip_alignment_bytes(file, header.object_size);

    return set;
  }

  darkstar::sim_items read_mission_data(std::istream& file)
  {
    static sim_item_reader_map readers = {
      { sim_group_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_group(file, header, readers); } } },
      { sim_set_tag, { [](auto& file, auto& header, auto& readers) -> sim_item { return read_sim_set(file, header, readers); } } },
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

    return read_children<sim_item>(file, 1, readers);
  }
}// namespace siege::mis::darkstar

namespace siege::resource::mis::darkstar
{
  using namespace siege::mis::darkstar;

  bool mis_resource_reader::is_supported(std::istream& stream)
  {
    return is_mission_data(stream);
  }

  bool mis_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_mission_data(stream);
  }

  std::filesystem::path get_archive_path(const std::filesystem::path& archive_or_folder_path)
  {
    auto archive_path = archive_or_folder_path;

    constexpr auto depth = 20;

    for (auto i = 0; i < depth; ++i)
    {
      if (platform::to_lower(archive_path.extension().string()) == ".mis")
      {
        break;
      }
      archive_path = archive_path.parent_path();
    }

    return archive_path;
  }

  decltype(mis_resource_reader::content_list_info)::iterator mis_resource_reader::cache_data(std::istream& stream,
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

            mis_resource_reader::folder_info folder{};
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

            mis_resource_reader::folder_info folder{};
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
              mis_resource_reader::file_info file{};
              file.folder_path = child.first.parent_path();
              file.filename = child.first.filename().replace_extension(".veh");
              file.size = sizeof(object_header) + child_as_vehicle.header.object_size;
              file.offset = position;
              file.compression_type = platform::compression_type::none;

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

  std::vector<mis_resource_reader::content_info> mis_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    auto existing_info = cache_data(stream, query.folder_path);
    std::vector<mis_resource_reader::content_info> final_results;
    final_results.reserve(existing_info->second.size());

    for (auto& ref : existing_info->second)
    {
      std::visit([&](auto& info) {
        using info_type = std::decay_t<decltype(info)>;

        if constexpr (std::is_same_v<info_type, siege::platform::file_info>)
        {
          if (info.folder_path == query.folder_path)
          {
            final_results.emplace_back(ref.second);
          }
        }

        if constexpr (std::is_same_v<info_type, siege::platform::folder_info>)
        {
          if (info.full_path.parent_path() == query.folder_path)
          {
            final_results.emplace_back(ref.second);
          }
        }
      },
        ref.second);
    }

    return final_results;
  }

  void mis_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    stream.seekg(info.offset, std::ios::beg);
  }

  void mis_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
    set_stream_position(stream, info);
    auto existing_info = cache_data(stream, info.folder_path);

    for (auto& ref : existing_info->second)
    {
      if (!std::visit([&](auto& child_info) -> bool {
            if constexpr (std::is_same_v<std::decay_t<decltype(child_info)>, siege::platform::file_info>)
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
}// namespace siege::resource::mis::darkstar
