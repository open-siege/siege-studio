#include <iomanip>
#include <execution>
#include <wx/quantize.h>
#include "bmp_view.hpp"
#include "content/bmp/bitmap.hpp"
#include "sfml_keys.hpp"
#include "utility.hpp"
#include "content/json_boost.hpp"

namespace studio::content::pal
{
  using studio::content::to_json;
  using studio::content::from_json;
}// namespace studio::content::pal

std::vector<std::int32_t> widen(const std::vector<std::byte>& pixels)
{
  std::vector<std::int32_t> results;
  results.reserve(pixels.size());

  std::transform(pixels.begin(), pixels.end(), std::back_inserter(results), [](auto value) {
    return std::int32_t(value);
  });

  return results;
}

std::vector<std::byte> narrow(const std::vector<std::int32_t>& pixels)
{
  std::vector<std::byte> results;
  results.reserve(pixels.size());

  std::transform(pixels.begin(), pixels.end(), std::back_inserter(results), [](auto value) {
    return std::byte(value);
  });

  return results;
}

namespace studio::views
{
  constexpr static auto auto_generated_name = std::string_view("Auto-generated");
  const static std::vector<std::string_view> bmp_extensions = { ".bmp", ".dib", ".pba", ".dbm", ".dba", ".db0", ".db1", ".db2", ".hba", ".hb0", ".hb1", ".hb2" };

  std::filesystem::path bmp_view::export_path = std::filesystem::path();
  std::vector<bmp_view::image_data> get_texture_data(const bmp_view::bmp_variant& bmp_data)
  {
    return std::visit([&](const auto& frames) {
      using T = std::decay_t<decltype(frames)>;

      if constexpr (std::is_same_v<T, studio::content::bmp::windows_bmp_data>)
      {
        std::vector<bmp_view::image_data> results;
        bmp_view::image_data temp_image{ frames.info.width, frames.info.height, frames.info.bit_depth, std::move(frames.indexes) };

        results.emplace_back(temp_image);

        return results;
      }

      if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::dbm_data>>)
      {
        std::vector<bmp_view::image_data> results;
        results.reserve(frames.size());
        for (const auto& es_bmp : frames)
        {
          bmp_view::image_data temp_image{
            es_bmp.header.width,
            es_bmp.header.height,
            int(es_bmp.header.bit_depth),
            widen(es_bmp.pixels)
          };
          results.emplace_back(temp_image);
        }
        return results;
      }

      if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::pbmp_data>>)
      {
        std::vector<bmp_view::image_data> results;
        results.reserve(frames.size());

        for (auto& phoenix_bmp : frames)
        {
          bmp_view::image_data temp_image{
            phoenix_bmp.bmp_header.width,
            phoenix_bmp.bmp_header.height,
            int(phoenix_bmp.bmp_header.bit_depth),
            widen(phoenix_bmp.pixels)
          };

          results.emplace_back(temp_image);
        }
        return results;
      }
    },
      bmp_data);
  }

  void set_default_palette(const studio::resources::resource_explorer& manager,
    const std::string& key,
    std::string_view name,
    nlohmann::json& settings,
    std::optional<std::size_t> index = std::nullopt)
  {
    try
    {
      const auto settings_path = manager.get_search_path() / "palettes.settings.json";

      if (index.has_value())
      {
        settings[key] = { {"name", name}, {"index", index.value()} };
      }
      else
      {
        settings[key] = name;
      }

      std::ofstream new_file(settings_path, std::ios::trunc);
      new_file << std::setw(4) << settings;
    }
    catch (...)
    {
      return;
    }
  }

  auto load_settings(const studio::resources::resource_explorer& manager)
  {
    const auto settings_path = manager.get_search_path() / "palettes.settings.json";
    auto settings = std::filesystem::exists(settings_path) ? nlohmann::json::parse(std::ifstream(settings_path)) : nlohmann::json{};
    return settings;
  }

  auto generate_settings_key(const studio::resources::resource_explorer& manager, const studio::resources::file_info& file)
  {
    return (std::filesystem::relative(file.folder_path, manager.get_search_path()) / file.filename).string();
  }

  void set_default_palette(const studio::resources::resource_explorer& manager,
    const std::string& key,
    std::string_view name,
    std::optional<std::size_t> index = std::nullopt)
  {
    auto settings =load_settings(manager);
    set_default_palette(manager, key, name, settings, index);
  }

  void set_default_palette(const studio::resources::resource_explorer& manager,
    const studio::resources::file_info& file,
    std::string_view name,
    std::optional<std::size_t> index = std::nullopt)
  {
      set_default_palette(manager, generate_settings_key(manager, file), name, index);
  }

  std::optional<std::pair<std::string_view, std::size_t>> default_palette_from_settings(
    const studio::resources::file_info& file,
    const studio::resources::resource_explorer& manager,
    const bmp_view::palette_map& loaded_palettes)
  {
    std::size_t index = 0;
    std::string_view name;

    const auto settings_path = manager.get_search_path() / "palettes.settings.json";

    if (!std::filesystem::exists(settings_path))
    {
      return std::nullopt;
    }

    try
    {
      auto settings = nlohmann::json::parse(std::ifstream(settings_path));

      auto get_value = [&](std::string key) -> bool {
        if (settings.contains(key))
        {
          auto value = settings[key];

          if (value.type() == nlohmann::json::value_t::object && value.contains("name"))
          {
            if (value.contains("index"))
            {
              index = value["index"];
            }

            value = value["name"];
          }
          else if (value.type() != nlohmann::json::value_t::string)
          {
            return false;
          }

          auto loaded_palette = loaded_palettes.find(std::string_view(value));

          if (loaded_palette != loaded_palettes.end())
          {
            name = loaded_palette->first;
            return true;
          }

        }

        return false;
      };

      if (get_value((std::filesystem::relative(file.folder_path, manager.get_search_path()) / file.filename).string()) ||
          get_value("default"))
      {
        return std::make_pair(name, index);
      }

      return std::nullopt;
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  std::pair<std::string_view, std::size_t> detect_default_palette(
    const bmp_view::bmp_variant& data,
    const studio::resources::file_info& file,
    const studio::resources::resource_explorer& manager,
    const bmp_view::palette_map& loaded_palettes)
  {
    return std::visit([&](const auto& frames) {
      using T = std::decay_t<decltype(frames)>;
      if constexpr (std::is_same_v<T, studio::content::bmp::windows_bmp_data>)
      {
        return std::make_pair(std::string_view("Internal"), std::size_t(0u));
      }

      if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::dbm_data>>)
      {
        const auto settings = default_palette_from_settings(file, manager, loaded_palettes);

        if (settings.has_value())
        {
          return settings.value();
        }

        return std::make_pair(auto_generated_name, std::size_t(0u));
      }

      if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::pbmp_data>>)
      {
        std::size_t index = std::string::npos;
        std::string_view name;

        const auto settings = default_palette_from_settings(file, manager, loaded_palettes);

        if (settings.has_value())
        {
          return settings.value();
        }

        for (auto& phoenix_bmp : frames)
        {
          if (!name.empty())
          {
            continue;
          }

          std::vector<const studio::resources::file_info*> results;
          results.reserve(loaded_palettes.size() / 2);

          for (auto& palette : loaded_palettes)
          {
            if (palette.second.first.folder_path == file.folder_path)
            {
              results.emplace_back(&palette.second.first);
            }
          }

          auto get_defaults = [&](bmp_view::palette_map::const_reference value) {
            for (auto i = 0u; i < value.second.second.size(); ++i)
            {
              auto& child = value.second.second[i];
              if (child.index == phoenix_bmp.palette_index)
              {
                index = i;
                name = value.first;
                break;
              }
            }
          };

          if (results.empty())
          {
            for (auto& entry : loaded_palettes)
            {
              if (entry.second.second.size() > phoenix_bmp.palette_index)
              {
                get_defaults(entry);
              }

              if (index != std::string::npos)
              {
                break;
              }
            }
          }
          else
          {
            auto bmp_file_name = shared::to_lower(file.filename.stem().string());

            auto possible_palette = std::find_if(results.begin(), results.end(), [&](auto* item) {
              auto palette_file_name = shared::to_lower(item->filename.stem().string());
              return bmp_file_name.rfind(palette_file_name, 0) == 0;
            });

            if (possible_palette != results.end())
            {
              auto key = (std::filesystem::relative((*possible_palette)->folder_path, manager.get_search_path()) / (*possible_palette)->filename).string();

              if (auto entry = loaded_palettes.find(key); entry != loaded_palettes.end())
              {
                get_defaults(*entry);
              }
            }
            else
            {
              for (auto result : results)
              {
                auto pal_key = (std::filesystem::relative(result->folder_path, manager.get_search_path()) / result->filename).string();

                if (auto entry = loaded_palettes.find(pal_key); entry != loaded_palettes.end())
                {
                  get_defaults(*entry);
                }

                if (index != std::string::npos)
                {
                  break;
                }
              }
            }
          }

          if (name.empty())
          {
            index = 0;
            name = auto_generated_name;
          }
        }
        return std::make_pair(name, index);
      }
    },
      data);
  }

  std::pair<bmp_view::bitmap_type, bmp_view::bmp_variant> load_image_data(std::basic_istream<std::byte>& image_stream)
  {
    if (content::bmp::is_microsoft_bmp(image_stream))
    {
      return std::make_pair(bmp_view::bitmap_type::microsoft, content::bmp::get_bmp_data(image_stream));
    }
    else if (content::bmp::is_earthsiege_bmp(image_stream) || content::bmp::is_earthsiege_bmp_array(image_stream))
    {
      std::vector<studio::content::bmp::dbm_data> frames;

      if (content::bmp::is_earthsiege_bmp(image_stream))
      {
        frames.emplace_back(content::bmp::read_earthsiege_bmp(image_stream));
      }
      else if (content::bmp::is_earthsiege_bmp_array(image_stream))
      {
        frames = content::bmp::read_earthsiege_bmp_array(image_stream);
      }

      return std::make_pair(bmp_view::bitmap_type::earthsiege, std::move(frames));
    }
    else
    {
      std::vector<content::bmp::pbmp_data> frames;

      if (content::bmp::is_phoenix_bmp(image_stream))
      {
        frames.emplace_back(content::bmp::get_pbmp_data(image_stream));
      }
      else if (content::bmp::is_phoenix_bmp_array(image_stream))
      {
        frames = content::bmp::get_pba_data(image_stream);
      }

      return std::make_pair(bmp_view::bitmap_type::phoenix, std::move(frames));
    }
  }


  std::vector<content::pal::colour> get_default_colours()
  {
    std::vector<content::pal::colour> default_colours;
    default_colours.reserve(256);
    for (auto i = 0; i < 256; ++i)
    {
      auto colour1 = std::byte(i);
      auto colour2 = std::byte(256 - i);
      auto colour3 = std::byte(std::rand() % 256);
      default_colours.emplace_back(content::pal::colour{ colour1, colour2, colour3, std::byte(0xFF) });
    }

    return default_colours;
  }

  void create_image(sf::Image& loaded_image,
    const bmp_view::image_data& data,
    const std::vector<std::array<std::byte, 3>>& colours)
  {
    std::vector<std::uint8_t> rendered_pixels;
    rendered_pixels.reserve(data.width * data.height * sizeof(std::int32_t));

    if (data.pixels.empty())
    {
      for (auto colour : colours)
      {
        rendered_pixels.emplace_back(int(colour[0]));
        rendered_pixels.emplace_back(int(colour[1]));
        rendered_pixels.emplace_back(int(colour[2]));
        rendered_pixels.emplace_back(int(0xFF));
      }
    }
    else
    {
      for (auto index : data.pixels)
      {
        auto& colour = colours[static_cast<std::int32_t>(index)];
        rendered_pixels.emplace_back(int(colour[0]));
        rendered_pixels.emplace_back(int(colour[1]));
        rendered_pixels.emplace_back(int(colour[2]));
        rendered_pixels.emplace_back(int(0xFF));
      }
    }

    loaded_image.create(data.width, data.height, rendered_pixels.data());
    loaded_image.flipVertically();
  }

  void create_image(sf::Image& loaded_image,
    const bmp_view::image_data& data,
    const std::vector<content::pal::colour>& colours)
  {
    std::vector<std::uint8_t> rendered_pixels;
    rendered_pixels.reserve(data.width * data.height * sizeof(std::int32_t));

    if (data.pixels.empty())
    {
      for (auto colour : colours)
      {
        rendered_pixels.emplace_back(int(colour.red));
        rendered_pixels.emplace_back(int(colour.green));
        rendered_pixels.emplace_back(int(colour.blue));
        rendered_pixels.emplace_back(int(colour.flags));
      }
    }
    else
    {
      for (auto index : data.pixels)
      {
        auto& colour = colours[static_cast<std::int32_t>(index)];
        rendered_pixels.emplace_back(int(colour.red));
        rendered_pixels.emplace_back(int(colour.green));
        rendered_pixels.emplace_back(int(colour.blue));
        rendered_pixels.emplace_back(int(colour.flags));
      }
    }

    loaded_image.create(data.width, data.height, rendered_pixels.data());
    loaded_image.flipVertically();
  }

  void bmp_view::load_palettes(const std::vector<studio::resources::file_info>& palettes, const studio::resources::resource_explorer& manager)
  {
    for (auto& palette_info : palettes)
    {
      auto raw_palette = manager.load_file(palette_info);
      auto result = (std::filesystem::relative(palette_info.folder_path, manager.get_search_path()) / palette_info.filename).string();

      if (content::pal::is_earthsiege_pal(*raw_palette.second))
      {
        std::vector<content::pal::palette> temp;
        temp.emplace_back().colours = content::pal::get_earthsiege_pal(*raw_palette.second);
        loaded_palettes.emplace(sort_order.emplace_back(std::move(result)), std::make_pair(palette_info, std::move(temp)));
      }
      else if (content::pal::is_phoenix_pal(*raw_palette.second))
      {
        loaded_palettes.emplace(sort_order.emplace_back(std::move(result)), std::make_pair(palette_info, content::pal::get_ppl_data(*raw_palette.second)));
      }
      else if (content::pal::is_microsoft_pal(*raw_palette.second))
      {
        std::vector<content::pal::palette> temp;
        temp.emplace_back().colours = content::pal::get_pal_data(*raw_palette.second);

        loaded_palettes.emplace(sort_order.emplace_back(std::move(result)), std::make_pair(palette_info, std::move(temp)));
      }
    }
  }



  bmp_view::bmp_view(const studio::resources::file_info& info, std::basic_istream<std::byte>& image_stream, const studio::resources::resource_explorer& manager)
    : archive(manager), info(info)
  {
    if (export_path == std::filesystem::path())
    {
      export_path = archive.get_search_path() / "exported";
    }

    zoom_in = [&](const sf::Event&) {
      if (image_scale < 4.0f)
      {
        image_scale += 0.1f;
        scale_changed = true;
      }
    };

    zoom_out = [&](const sf::Event&) {
      if (image_scale > 1.0f)
      {
        image_scale -= 0.1f;
        scale_changed = true;
      }
    };

    std::vector<studio::resources::file_info> palettes;

    palettes = manager.find_files(studio::resources::resource_explorer::get_archive_path(info.folder_path).parent_path(), { ".ppl", ".ipl", ".pal", ".dpl" });

    auto all_palettes = manager.find_files({ ".ppl", ".ipl", ".pal", ".dpl" });

    studio::resources::resource_explorer::merge_results(palettes, all_palettes);

    load_palettes(palettes, manager);

    auto bmp_data = load_image_data(image_stream);
    image_type = bmp_data.first;

    auto [palette_name, palette_index] = detect_default_palette(bmp_data.second, info, manager, loaded_palettes);

    selection_state.selected_palette_index = selection_state.default_palette_index = palette_index;
    selection_state.selected_palette_name = selection_state.default_palette_name = palette_name;

    if (palette_name == "Internal")
    {
      std::visit([&](const auto& data) {
        using T = std::decay_t<decltype(data)>;

        if constexpr (std::is_same_v<T, studio::content::bmp::windows_bmp_data>)
        {
          std::vector<content::pal::palette> temp;
          auto& pal = temp.emplace_back();
          pal.colours = std::move(data.colours);

          loaded_palettes.emplace(sort_order.emplace_back("Internal"), std::make_pair(info, std::move(temp)));
        }
      },
        bmp_data.second);
    }
    else
    {
      std::vector<content::pal::palette> temp;
      temp.emplace_back().colours = get_default_colours();
      loaded_palettes.emplace(sort_order.emplace_back(auto_generated_name), std::make_pair(info, std::move(temp)));
      strategy = static_cast<int>(colour_strategy::do_nothing);
    }

    original_pixels = get_texture_data(bmp_data.second);

    sort_order.sort([&](const auto& a, const auto& b) {
      return (a == "Internal") || (a == auto_generated_name) || loaded_palettes.at(a).second.size() < loaded_palettes.at(b).second.size();
    });

    if (!original_pixels.empty())
    {
      create_image(loaded_image,
        original_pixels.at(selection_state.selected_bitmap_index),
        loaded_palettes.at(selection_state.selected_palette_name).second.at(selection_state.selected_palette_index).colours);
      texture.loadFromImage(loaded_image);
    }
  }

  template<typename RowType>
  std::vector<std::uint8_t*> temp_rows(std::vector<RowType>& data, int width, int height)
  {
    std::vector<std::uint8_t*> rows;
    rows.reserve(height);

    auto stride = width * sizeof(RowType);

    for (auto i = 0; i < height * stride; i += stride)
    {
      auto raw = reinterpret_cast<std::uint8_t*>(data.data()) + i;
      rows.emplace_back(raw);
    }

    return rows;
  }

  void bmp_view::refresh_image(bool create_new_image)
  {
    const auto& [selected_palette_name, selected_palette_index, default_palette_name, default_palette_index, selected_bitmap_index] = selection_state;

    if (selected_palette_name == "Internal" && original_pixels.at(selected_bitmap_index).bit_depth > 8)
    {
      create_image(loaded_image,
        original_pixels.at(selected_bitmap_index),
        loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours);

      if (create_new_image)
      {
        texture.loadFromImage(loaded_image);
      }
      else
      {
        texture.update(loaded_image);
      }

      return;
    }

    if (!original_pixels.empty())
    {
      auto colour_strat = colour_strategy(strategy);

      if (colour_strat == colour_strategy::do_nothing)
      {
        num_unique_colours = get_unique_colours(original_pixels.at(selected_bitmap_index).pixels);

        create_image(loaded_image,
          original_pixels.at(selected_bitmap_index),
          loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours);
      }
      else if (colour_strat == colour_strategy::remap)
      {
        auto existing_pixels = original_pixels.at(selected_bitmap_index);

        if (existing_pixels.pixels.empty())
        {
          auto& colours = loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours;

          std::vector<std::array<std::byte, 3>> narrowed_colours;
          narrowed_colours.reserve(colours.size());

          std::transform(colours.begin(), colours.end(), std::back_inserter(narrowed_colours), [](auto& colour) {
            return std::array<std::byte, 3>{ colour.red, colour.green, colour.blue };
          });

          std::vector<std::byte> new_indexes(colours.size(), std::byte('\0'));

          auto& palette = loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours;

          std::vector<std::array<std::byte, 3>> new_palette(palette.size(), std::array<std::byte, 3>{});

          auto width = existing_pixels.width;
          auto height = existing_pixels.height;

          wxQuantize::DoQuantize(width, height, temp_rows(narrowed_colours, width, height).data(), temp_rows(new_indexes, width, height).data(), reinterpret_cast<std::uint8_t*>(new_palette.data()), new_palette.size());

          std::vector<content::pal::colour> new_new_palette;
          new_new_palette.reserve(palette.size());

          std::transform(new_palette.begin(), new_palette.end(), std::back_inserter(new_new_palette), [](auto& colour) {
            return content::pal::colour{ colour[0], colour[1], colour[2], std::byte{ 0xFF } };
          });

          existing_pixels.pixels = widen(content::bmp::remap_bitmap(new_indexes,
            new_new_palette,
            loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours));

          num_unique_colours = get_unique_colours(new_indexes);

          create_image(loaded_image,
            existing_pixels,
            new_palette);
        }
        else
        {
          existing_pixels.pixels = content::bmp::remap_bitmap(existing_pixels.pixels,
            loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
            loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours);

          num_unique_colours = get_unique_colours(existing_pixels.pixels);

          create_image(loaded_image,
            existing_pixels,
            loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours);
        }
      }
      else if (colour_strat == colour_strategy::remap_unique)
      {
        auto image_data = original_pixels.at(selected_bitmap_index);

        image_data.pixels = content::bmp::remap_bitmap(image_data.pixels,
          loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
          loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours,
          true);

        num_unique_colours = get_unique_colours(image_data.pixels);

        create_image(loaded_image,
          image_data,
          loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours);
      }

      if (create_new_image)
      {
        texture.loadFromImage(loaded_image);
      }
      else
      {
        texture.update(loaded_image);
      }
    }
  }

  std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> bmp_view::get_callbacks()
  {
    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> callbacks;

    // TODO read this from config file
    callbacks.emplace(get_key_for_name("Add"), std::ref(zoom_in));
    callbacks.emplace(get_key_for_name("Subtract"), std::ref(zoom_out));
    return callbacks;
  }

  void bmp_view::render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& gui_context)
  {
    if (scale_changed)
    {
      setup_view(parent, window, gui_context);
      scale_changed = false;
    }

    window.clear();
    window.draw(sprite);

    if (!loaded_palettes.empty())
    {
      auto& [selected_palette_name, selected_palette_index, default_palette_name, default_palette_index, selected_bitmap_index] = selection_state;
      ImGui::Begin("Palettes");

      auto child_count = 0;
      for (auto& key : sort_order)
      {
        auto& [file_info, value] = loaded_palettes.at(key);

        if (value.size() == 1)
        {
          std::stringstream label;
          label << key;

          if (default_palette_name == key && default_palette_index == 0)
          {
            label << " (default for this image)";
          }

          if (value[0].index > 0)
          {
            label << " (ID: " << value[0].index << ")";
          }

          for (auto y = 0; y < child_count; ++y)
          {
            label << ' ';
          }

          if (ImGui::RadioButton(label.str().c_str(), selected_palette_name == key && selected_palette_index == 0))
          {
            selected_palette_name = key;
            selected_palette_index = 0;

            refresh_image();
          }

          ImGui::SameLine();

          ImGui::PushID(&file_info);
          if (ImGui::Button("Open in New Tab"))
          {
            archive.execute_action("open_new_tab", file_info);
          }
          ImGui::PopID();
        }
        else
        {
          if (ImGui::CollapsingHeader(
                key == default_palette_name ? (key + " (contains default for image)").c_str() : key.c_str(),

                key == selected_palette_name ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen : 0))
          {
            ImGui::PushID(&file_info);
            if (ImGui::Button("Open in New Tab"))
            {
              archive.execute_action("open_new_tab", file_info);
            }
            ImGui::PopID();

            std::stringstream label;

            for (auto i = 0u; i < value.size(); ++i)
            {
              label << "Palette " << i + 1;

              if (key == default_palette_name && i == default_palette_index)
              {
                label << " (default for this image)";
              }

              if (value[i].index > 0)
              {
                label << " (ID: " << value[i].index << ")";
              }

              for (auto y = 0; y < child_count; ++y)
              {
                label << ' ';
              }

              if (ImGui::RadioButton(label.str().c_str(), key == selected_palette_name && i == selected_palette_index))
              {
                selected_palette_name = key;
                selected_palette_index = i;

                refresh_image();
              }

              label.str("");
            }
            child_count++;
          }
        }
      }
      ImGui::End();

      ImGui::Begin("File Info");
      auto [width, height] = loaded_image.getSize();

      if (image_type == bitmap_type::microsoft)
      {
        ImGui::LabelText("", "File type: %s", "Microsoft Bitmap");
      }

      if (image_type == bitmap_type::earthsiege)
      {
        ImGui::LabelText("", "File type: %s", "Earthsiege Bitmap");
      }

      if (image_type == bitmap_type::phoenix)
      {
        ImGui::LabelText("", "File type: %s", "Phoenix Bitmap");
      }

      ImGui::LabelText("", "Resolution: %ix%i", width, height);
      ImGui::LabelText("", "Bits per pixel: %i", original_pixels.at(selected_bitmap_index).bit_depth);

      auto unique_colours = get_unique_colours(original_pixels.at(selected_bitmap_index).pixels);
      ImGui::LabelText("", "Original number of unique colours: %llu", unique_colours);

      if (num_unique_colours == 0)
      {
        num_unique_colours = unique_colours;
      }

      if (auto colour_strat = colour_strategy(strategy);
          (colour_strat == colour_strategy::remap || colour_strat == colour_strategy::remap_unique) && num_unique_colours != unique_colours)
      {
        ImGui::LabelText("", "Remapped number of unique colours: %llu", num_unique_colours);
      }

      ImGui::End();

      {
        ImGui::Begin("Export Options");

        if (ImGui::Button("Set Export Directory"))
        {
          auto dialog = std::make_unique<wxDirDialog>(nullptr, "Open a folder to export files to");

          if (dialog->ShowModal() == wxID_OK)
          {
            export_path = dialog->GetPath().c_str().AsChar();
          }
        }

        ImGui::SameLine();
        ImGui::Text("%s", export_path.string().c_str());

        if (image_type != bitmap_type::microsoft && ImGui::Button("Export to Regular BMP"))
        {
          auto new_file_name = info.filename.replace_extension(".bmp");
          std::filesystem::create_directories(export_path);
          std::basic_ofstream<std::byte> output(export_path / new_file_name, std::ios::binary);

          const auto& colours = loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours;
          auto pixels = content::bmp::remap_bitmap(original_pixels.at(selected_bitmap_index).pixels,
            loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
            colours);

          content::bmp::vertical_flip(pixels, width);
          content::bmp::write_bmp_data(output, colours, narrow(pixels), width, height, 8);
          if (!opened_folder)
          {
            wxLaunchDefaultApplication(export_path.string());
            opened_folder = true;
          }
        }

        if (ImGui::Button("Export All to Regular BMPs"))
        {
          auto files = archive.find_files(bmp_extensions);

          if (!opened_folder && !files.empty())
          {
            wxLaunchDefaultApplication(export_path.string());
            opened_folder = true;
          }

          std::for_each(std::execution::par_unseq, files.begin(), files.end(), [=](const auto& shape_info) {
            const auto new_path = export_path / std::filesystem::relative(shape_info.folder_path, archive.get_search_path());

            auto new_file_name = std::filesystem::path(shape_info.filename).replace_extension(".bmp");
            std::filesystem::create_directories(new_path);
            std::basic_ofstream<std::byte> output(new_path / new_file_name, std::ios::binary);

            auto image_stream = archive.load_file(shape_info);
            const auto [bmp_type, bmp_data] = load_image_data(*image_stream.second);

            const auto default_palette = detect_default_palette(bmp_data, shape_info, archive, loaded_palettes);

            std::visit([&](const auto& frames) {
              using T = std::decay_t<decltype(frames)>;

              if constexpr (std::is_same_v<T, studio::content::bmp::windows_bmp_data>)
              {
                content::bmp::write_bmp_data(output, frames.colours, narrow(frames.indexes), frames.info.width, frames.info.height, 8);
              }

              if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::dbm_data>>)
              {
                if (frames.empty())
                {
                  return;
                }

                const auto& [palette_name, palette_index] = default_palette;

                const auto& frame = frames.front();

                const auto& colours = loaded_palettes.at(palette_name).second.at(palette_index).colours;
                auto pixels = frame.pixels;

                content::bmp::vertical_flip(pixels, frame.header.width);
                content::bmp::write_bmp_data(output, colours, pixels, frame.header.width, frame.header.height, 8);
              }

              if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::pbmp_data>>)
              {
                if (frames.empty())
                {
                  return;
                }

                const auto& [palette_name, palette_index] = default_palette;
                const auto& frame = frames.front();

                const auto& colours = loaded_palettes.at(palette_name).second.at(palette_index).colours;
                auto pixels = frame.pixels;

                content::bmp::vertical_flip(pixels, frame.bmp_header.width);
                content::bmp::write_bmp_data(output, colours, pixels, frame.bmp_header.width, frame.bmp_header.height, 8);
              }


            }, bmp_data);
          });
          auto new_file_name = info.filename.replace_extension(".bmp");
          std::filesystem::create_directories(export_path);
          std::basic_ofstream<std::byte> output(export_path / new_file_name, std::ios::binary);

          const auto& colours = loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours;
          auto pixels = content::bmp::remap_bitmap(original_pixels.at(selected_bitmap_index).pixels,
            loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
            colours);

          content::bmp::vertical_flip(pixels, width);
          content::bmp::write_bmp_data(output, colours, narrow(pixels), width, height, 8);
          if (!opened_folder)
          {
            wxLaunchDefaultApplication(export_path.string());
            opened_folder = true;
          }
        }

        if (ImGui::Button("Export to JSON"))
        {
          pending_save = std::async(std::launch::async, [this]() {
            auto [width, height] = loaded_image.getSize();
            auto& [selected_palette_name, selected_palette_index, default_palette_name, default_palette_index, selected_bitmap_index] = selection_state;
            auto new_file_name = info.filename.string() + ".json";
            std::filesystem::create_directories(export_path);
            std::ofstream output(export_path / new_file_name, std::ios::trunc);

            const auto& colours = loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours;
            auto indexes = content::bmp::remap_bitmap(original_pixels.at(selected_bitmap_index).pixels,
              loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
              colours);

            nlohmann::ordered_json item_as_json;

            if (indexes.empty())
            {
              item_as_json["width"] = width;
              item_as_json["height"] = height;
              item_as_json["bitDepth"] = original_pixels.at(selected_bitmap_index).bit_depth;
              item_as_json["pixels"] = colours;
            }
            else
            {
              item_as_json["width"] = width;
              item_as_json["height"] = height;
              item_as_json["bitDepth"] = original_pixels.at(selected_bitmap_index).bit_depth;
              item_as_json["colours"] = colours;
              item_as_json["indexes"] = indexes;
            }

            output << std::setw(4) << item_as_json;

            if (!opened_folder)
            {
              wxLaunchDefaultApplication(export_path.string());
              opened_folder = true;
            }

            return true;
          });
        }
        ImGui::End();
      }

      ImGui::Begin("Settings");

      ImGui::Text("%s", "Colour Strategy: ");
      ImGui::SameLine();

      if (ImGui::RadioButton("Do Nothing", &strategy, 0))
      {
        refresh_image();
      }
      ImGui::SameLine();

      if (ImGui::RadioButton("Remap", &strategy, 1))
      {
        refresh_image();
      }
      ImGui::SameLine();

      if (ImGui::RadioButton("Remap (only unique colours)", &strategy, 2))
      {
        refresh_image();
      }

      ImGui::Text("%s", "Default Palette File: ");
      ImGui::SameLine();

      int name_distance = std::distance(loaded_palettes.begin(), loaded_palettes.find(default_palette_name));

      if (ImGui::Combo(
            "", (&name_distance), [](void*data, int idx, const char**out_text) -> bool {
              auto* real_data = reinterpret_cast<decltype(loaded_palettes)*>(data);
              auto it = real_data->begin();
              std::advance(it, idx);
              if (it != real_data->end())
              {
                static std::string temp;
                temp = it->first;
                *out_text = temp.c_str();
                return true;
              }
              else
              {
                *out_text = "N/A";
                return false;
              }
            },
            reinterpret_cast<void*>(&loaded_palettes),
            loaded_palettes.size()))
      {
        auto new_palette = loaded_palettes.begin();

        std::advance(new_palette, name_distance);

        if (new_palette != loaded_palettes.end())
        {
          auto old_name = default_palette_name;
          default_palette_name = new_palette->first;
          if (new_palette->second.second.size() < default_palette_index)
          {
            default_palette_index = 0;
          }

          if (selected_palette_name == old_name)
          {
            selected_palette_index = default_palette_index;
            selected_palette_name = default_palette_name;
          }

          if (image_type == bitmap_type::earthsiege)
          {
            set_default_palette(archive, info, default_palette_name);
          }
          else
          {
            set_default_palette(archive, info, default_palette_name, default_palette_index);
          }

          refresh_image();
        }
      }

      ImGui::Text("%s", "Default Palette: ");
      ImGui::SameLine();


      auto old_index = default_palette_index;
      if (ImGui::Combo(
            "  ", reinterpret_cast<int*>(&default_palette_index), [](void*data, int idx, const char**out_text) -> bool {
              auto* real_data = reinterpret_cast<decltype(loaded_palettes)::value_type::second_type*>(data);
              auto it = real_data->second.begin();
              std::advance(it, idx);
              if (it != real_data->second.end())
              {
                static std::string temp;
                temp = "Palette " + std::to_string(idx + 1);
                *out_text = temp.c_str();
                return true;
              }
              else
              {
                *out_text = "N/A";
                return false;
              }
            },
            reinterpret_cast<void*>(&loaded_palettes.at(default_palette_name)),
            loaded_palettes.at(default_palette_name).second.size()))
      {
        if (selected_palette_index == old_index)
        {
          selected_palette_index = default_palette_index;
        }

        if (image_type == bitmap_type::earthsiege)
        {
          set_default_palette(archive, info, default_palette_name);
        }
        else
        {
          set_default_palette(archive, info, default_palette_name, default_palette_index);
        }

        refresh_image();
      }


      if (image_type == bitmap_type::earthsiege && ImGui::Button("Set as Shared Default"))
      {
        set_default_palette(archive, "default", default_palette_name);
      }

      if (ImGui::Button("Set as Default for All Files in Folder/Volume"))
      {
        auto archive_path = studio::resources::resource_explorer::get_archive_path(info.folder_path);
        auto all_bitmaps = archive.find_files(archive_path, bmp_extensions);

        auto settings = load_settings(archive);

        for (const auto& bmp_info : all_bitmaps)
        {
          set_default_palette(archive, generate_settings_key(archive, bmp_info), default_palette_name, settings, default_palette_index);
        }
      }


      ImGui::Text("%s", "Zoom: ");
      ImGui::SameLine();

      if (ImGui::SliderFloat("   ", &image_scale, 1.0f, 4.0f))
      {
        setup_view(parent, window, gui_context);
      }

      ImGui::End();

      if (original_pixels.size() > 1)
      {
        ImGui::Begin("Frames");
        std::stringstream label;
        for (auto i = 0; i < original_pixels.size(); ++i)
        {
          label << "Frame " << i + 1;

          if (ImGui::RadioButton(label.str().c_str(), &selected_bitmap_index, i))
          {
            refresh_image(true);
          }
          label.str("");
        }
        ImGui::End();
      }
    }
  }

  void bmp_view::setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext&)
  {
    if (!sprite.getTexture())
    {
      sprite.setTexture(texture);
    }

    auto [width, height] = parent.GetClientSize();

    auto image_width = sprite.getTexture()->getSize().x;
    auto image_height = sprite.getTexture()->getSize().y;

    auto width_ratio = float(image_width) / float(width) * image_scale;
    auto height_ratio = float(image_height) / float(height) * image_scale;

    sf::FloatRect visibleArea(0, 0, image_width, image_height);
    sf::View view(visibleArea);
    view.setViewport(sf::FloatRect(0.5 - width_ratio / 2, 0.5 - height_ratio / 2, width_ratio, height_ratio));
    window.setView(view);
  }
}// namespace studio::views
