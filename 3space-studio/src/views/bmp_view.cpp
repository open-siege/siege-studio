#include <iomanip>
#include <iostream>
#include <execution>
#include <wx/quantize.h>
#include "bmp_view.hpp"
#include "content/bmp/bitmap.hpp"
#include "sfml_keys.hpp"
#include "utility.hpp"
#include "bmp_shared.hpp"
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
  std::filesystem::path bmp_view::export_path = std::filesystem::path();
  std::vector<bmp_view::image_data> get_texture_data(const bmp_variant& bmp_data)
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

  auto remap_image(bmp_view::colour_strategy colour_strat,
    bmp_view::image_data existing_pixels,
    const std::vector<content::pal::colour>& default_colours,
    const std::vector<content::pal::colour>& selected_colours)
  {
    if (colour_strat == bmp_view::colour_strategy::do_nothing || default_colours == selected_colours)
    {
      return existing_pixels;
    }

    if (existing_pixels.pixels.empty())
    {
      std::vector<std::array<std::byte, 3>> narrowed_colours;
      narrowed_colours.reserve(default_colours.size());

      std::transform(default_colours.begin(), default_colours.end(), std::back_inserter(narrowed_colours), [](auto& colour) {
        return std::array<std::byte, 3>{ colour.red, colour.green, colour.blue };
      });

      std::vector<std::byte> new_indexes(default_colours.size(), std::byte('\0'));

      std::vector<std::array<std::byte, 3>> new_palette(selected_colours.size(), std::array<std::byte, 3>{});

      auto width = existing_pixels.width;
      auto height = existing_pixels.height;

      wxQuantize::DoQuantize(width, height, temp_rows(narrowed_colours, width, height).data(), temp_rows(new_indexes, width, height).data(), reinterpret_cast<std::uint8_t*>(new_palette.data()), new_palette.size());

      std::vector<content::pal::colour> new_new_palette;
      new_new_palette.reserve(selected_colours.size());

      std::transform(new_palette.begin(), new_palette.end(), std::back_inserter(new_new_palette), [](auto& colour) {
        return content::pal::colour{ colour[0], colour[1], colour[2], std::byte{ 0xFF } };
      });

      existing_pixels.bit_depth = selected_colours.size() <= 256 ? 8 : existing_pixels.bit_depth;

      existing_pixels.pixels = widen(content::bmp::remap_bitmap(new_indexes,
        new_new_palette,
        selected_colours,
        colour_strat == bmp_view::colour_strategy::remap_unique));
    }
    else
    {
      existing_pixels.pixels = content::bmp::remap_bitmap(existing_pixels.pixels,
        default_colours,
        selected_colours);
    }

    return existing_pixels;
  }


  bmp_view::bmp_view(view_context context, std::basic_istream<std::byte>& image_stream)
    : context(std::move(context))
  {
    const auto& info = this->context.file_info;
    const auto& explorer = this->context.explorer;

    if (export_path == std::filesystem::path())
    {
      export_path = explorer.get_search_path() / "exported";
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

    const auto pal_extensions = this->context.actions.get_extensions_by_category("all_palettes");
    const auto palettes = explorer.find_files(pal_extensions);

    palette_data.load_palettes(explorer, palettes);

    auto& [sort_order, loaded_palettes] = palette_data;

    auto bmp_data = load_image_data(info, image_stream);
    image_type = bmp_data.first;

    auto [default_palette_name, default_palette_index] = detect_default_palette(bmp_data.second, info, explorer, loaded_palettes);

    auto selected_palette = selected_palette_from_settings(info, explorer, loaded_palettes)
                              .value_or(std::make_pair(default_palette_name, default_palette_index));

    selection_state.default_palette_name = default_palette_name;
    selection_state.default_palette_index = default_palette_index;
    selection_state.selected_palette_name = selected_palette.first;
    selection_state.selected_palette_index = selected_palette.second;


    if (default_palette_name == "Internal")
    {
      std::visit([&](const auto& data) {
        using T = std::decay_t<decltype(data)>;

        if constexpr (std::is_same_v<T, studio::content::bmp::windows_bmp_data>)
        {
          std::vector<content::pal::palette> temp;
          auto& pal = temp.emplace_back();
          pal.colours = std::move(data.colours);

          palette_data.loaded_palettes.emplace(palette_data.sort_order.emplace_back("Internal"), std::make_pair(info, std::move(temp)));
        }
      },
        bmp_data.second);
    }

    std::vector<content::pal::palette> temp;
    temp.emplace_back().colours = get_default_colours();
    loaded_palettes.emplace(sort_order.emplace_back(auto_generated_name), std::make_pair(info, std::move(temp)));

    if (default_palette_name == auto_generated_name)
    {
      strategy = static_cast<int>(colour_strategy::do_nothing);
    }

    original_pixels = get_texture_data(bmp_data.second);

    sort_order.sort([&](const auto& a, const auto& b) {
      if (a == "Internal" && b == auto_generated_name)
      {
        return true;
      }
      else if (b == "Internal" && a == auto_generated_name)
      {
        return false;
      }
      return (a == "Internal") || (a == auto_generated_name) || palette_data.loaded_palettes.at(a).second.size() < palette_data.loaded_palettes.at(b).second.size();
    });

    if (!original_pixels.empty())
    {
      create_image(loaded_image,
        original_pixels.at(selection_state.selected_bitmap_index),
        loaded_palettes.at(selection_state.selected_palette_name).second.at(selection_state.selected_palette_index).colours);
      texture.loadFromImage(loaded_image);
    }
  }

  void bmp_view::refresh_image(bool create_new_image)
  {
    const auto& [selected_palette_name, selected_palette_index, default_palette_name, default_palette_index, selected_bitmap_index] = selection_state;

    if (selected_palette_name == "Internal" && original_pixels.at(selected_bitmap_index).bit_depth > 8)
    {
      create_image(loaded_image,
        original_pixels.at(selected_bitmap_index),
        palette_data.loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours);

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
      const auto& selected_colours = palette_data.loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours;

      auto fresh_image = remap_image(
        colour_strategy(strategy),
        original_pixels.at(selected_bitmap_index),
        palette_data.loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
        selected_colours);

      num_unique_colours = get_unique_colours(fresh_image.pixels);

      create_image(loaded_image,
        fresh_image,
        selected_colours);

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

    if (!palette_data.loaded_palettes.empty())
    {
      auto& [selected_palette_name, selected_palette_index, default_palette_name, default_palette_index, selected_bitmap_index] = selection_state;
      ImGui::Begin("Palettes");

      auto child_count = 0;
      for (auto& key : palette_data.sort_order)
      {
        auto& [file_info, value] = palette_data.loaded_palettes.at(key);

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
            context.actions.open_new_tab(file_info);
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
              context.actions.open_new_tab(file_info);
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

                set_selected_palette(context.explorer, context.file_info, selected_palette_name, selected_palette_index);

                refresh_image();
              }

              if (colour_strategy(strategy) == colour_strategy::do_nothing &&
                  key == selected_palette_name && i == selected_palette_index && !(key == default_palette_name && i == default_palette_index))
              {
                ImGui::SameLine();
                if (ImGui::Button("Set as Default"))
                {
                  default_palette_name = key;
                  default_palette_index = i;
                  set_default_palette(context.explorer, context.file_info, default_palette_name, default_palette_index);
                }
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

      if (original_pixels.at(selected_bitmap_index).bit_depth == 8)
      {
        ImGui::LabelText("", "Original number of unique colours: %llu", unique_colours);
      }

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

        if (ImGui::Button("Export to Regular BMP"))
        {
          auto new_file_name = context.file_info.filename.replace_extension(".bmp");
          std::filesystem::create_directories(export_path);
          std::basic_ofstream<std::byte> output(export_path / new_file_name, std::ios::binary);

          const auto& colours = palette_data.loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours;

          auto fresh_image = remap_image(
            colour_strategy(strategy),
            original_pixels.at(selected_bitmap_index),
            palette_data.loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
            colours);

          content::bmp::write_bmp_data(output, colours, narrow(fresh_image.pixels), width, height, fresh_image.bit_depth);
          if (!opened_folder)
          {
            wxLaunchDefaultApplication(export_path.string());
            opened_folder = true;
          }
        }

        if (!(selected_palette_name == "Internal" &&
            original_pixels.at(selected_bitmap_index).bit_depth > 8) &&
            ImGui::Button("Export to Phoenix BMP"))
        {
          auto new_file_name = context.file_info.filename.replace_extension(".bmp");
          std::filesystem::create_directories(export_path);
          std::basic_ofstream<std::byte> output(export_path / new_file_name, std::ios::binary);

          const auto& selected_palette = palette_data.loaded_palettes.at(selected_palette_name).second.at(selected_palette_index);

          auto fresh_image = remap_image(
            colour_strategy(strategy),
            original_pixels.at(selected_bitmap_index),
            palette_data.loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
            selected_palette.colours);

          content::bmp::write_pbmp_data(output, width, height, selected_palette.colours, narrow(fresh_image.pixels), selected_palette.index);
          if (!opened_folder)
          {
            wxLaunchDefaultApplication(export_path.string());
            opened_folder = true;
          }
        }

        if (ImGui::Button("Export All to Regular BMPs"))
        {
          auto extensions = context.actions.get_extensions_by_category("all_images");
          auto files = context.explorer.find_files(extensions);

          if (!opened_folder && !files.empty())
          {
            wxLaunchDefaultApplication(export_path.string());
            opened_folder = true;
          }

          std::for_each(std::execution::par_unseq, files.begin(), files.end(), [=](const auto& shape_info) {
            const auto new_path = export_path / std::filesystem::relative(shape_info.folder_path, context.explorer.get_search_path());

            auto new_file_name = std::filesystem::path(shape_info.filename).replace_extension(".bmp");
            try
            {
              std::filesystem::create_directories(new_path);
              std::basic_ofstream<std::byte> output(new_path / new_file_name, std::ios::binary);

              auto image_stream = context.explorer.load_file(shape_info);
              const auto [bmp_type, bmp_data] = load_image_data(image_stream.first,*image_stream.second);

              const auto default_palette = detect_default_palette(bmp_data, shape_info, context.explorer, palette_data.loaded_palettes);

              const auto selected_palette =
                selected_palette_from_settings(shape_info, context.explorer, palette_data.loaded_palettes)
                  .value_or(default_palette);


              // Used because the colours of the image have to be unwrapped at a later stage
              thread_local auto placeholder_colours = get_default_colours();

              auto& default_colours = default_palette.first == "Internal" ?
                                                                                placeholder_colours :
                                                                                 palette_data.loaded_palettes.at(default_palette.first).second.at(default_palette.second).colours;
              auto& selected_colours = selected_palette.first == "Internal" ?
                                                                                  placeholder_colours :
                                                                                  palette_data.loaded_palettes.at(selected_palette.first).second.at(selected_palette.second).colours;

              const auto fresh_image = std::visit([&](const auto& frames) -> image_data {
                using T = std::decay_t<decltype(frames)>;

                if constexpr (std::is_same_v<T, studio::content::bmp::windows_bmp_data>)
                {
                  image_data temp;
                  temp.pixels = frames.indexes;
                  temp.bit_depth = frames.info.bit_depth;
                  temp.width = frames.info.width;
                  temp.height = frames.info.height;

                  if (default_palette.first == "Internal")
                  {
                    default_colours = frames.colours;
                  }

                  if (selected_palette.first == "Internal")
                  {
                    selected_colours = frames.colours;
                  }

                  return remap_image(
                    colour_strategy::remap,
                    temp,
                    default_colours,
                    selected_colours);
                }

                if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::dbm_data>>)
                {
                  if (frames.empty())
                  {
                    return {};
                  }

                  const auto& frame = frames.front();

                  image_data temp;
                  temp.pixels = widen(frame.pixels);
                  temp.bit_depth = int(frame.header.bit_depth);
                  temp.width = frame.header.width;
                  temp.height = frame.header.height;

                  return remap_image(
                    colour_strategy::remap,
                    temp,
                    default_colours,
                    selected_colours);
                }

                if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::pbmp_data>>)
                {
                  if (frames.empty())
                  {
                    return {};
                  }

                  const auto& frame = frames.front();

                  image_data temp;
                  temp.pixels = widen(frame.pixels);
                  temp.bit_depth = int(frame.bmp_header.bit_depth);
                  temp.width = frame.bmp_header.width;
                  temp.height = frame.bmp_header.height;

                  return remap_image(
                    colour_strategy::remap,
                    temp,
                    default_colours,
                    selected_colours);
                }

              }, bmp_data);

              content::bmp::write_bmp_data(output, selected_colours, narrow(fresh_image.pixels), fresh_image.width, fresh_image.height, fresh_image.bit_depth);
            }
            catch (const std::exception& ex)
            {
              std::cerr << ((new_path / new_file_name).string() + ": " + ex.what() + '\n');
            }
          });
          auto new_file_name = context.file_info.filename.replace_extension(".bmp");
          std::filesystem::create_directories(export_path);
          std::basic_ofstream<std::byte> output(export_path / new_file_name, std::ios::binary);

          const auto& colours = palette_data.loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours;
          auto pixels = content::bmp::remap_bitmap(original_pixels.at(selected_bitmap_index).pixels,
            palette_data.loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
            colours);

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
            auto new_file_name = context.file_info.filename.string() + ".json";
            std::filesystem::create_directories(export_path);
            std::ofstream output(export_path / new_file_name, std::ios::trunc);

            const auto& colours = palette_data.loaded_palettes.at(selected_palette_name).second.at(selected_palette_index).colours;
            auto indexes = content::bmp::remap_bitmap(original_pixels.at(selected_bitmap_index).pixels,
              palette_data.loaded_palettes.at(default_palette_name).second.at(default_palette_index).colours,
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

      ImGui::Text("%s", "Zoom: ");
      ImGui::SameLine();

      if (ImGui::SliderFloat("   ", &image_scale, 1.0f, 4.0f))
      {
        setup_view(parent, window, gui_context);
      }


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

      int name_distance = std::distance(palette_data.loaded_palettes.begin(), palette_data.loaded_palettes.find(default_palette_name));

      if (ImGui::Combo(
            "", (&name_distance), [](void*data, int idx, const char**out_text) -> bool {
              auto* real_data = reinterpret_cast<decltype(palette_data.loaded_palettes)*>(data);
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
            reinterpret_cast<void*>(&palette_data.loaded_palettes),
            palette_data.loaded_palettes.size()))
      {
        auto new_palette = palette_data.loaded_palettes.begin();

        std::advance(new_palette, name_distance);

        if (new_palette != palette_data.loaded_palettes.end())
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

          set_default_palette(context.explorer, context.file_info, default_palette_name, default_palette_index);

          refresh_image();
        }
      }

      ImGui::Text("%s", "Default Palette: ");
      ImGui::SameLine();


      auto old_index = default_palette_index;
      if (ImGui::Combo(
            "  ", reinterpret_cast<int*>(&default_palette_index), [](void*data, int idx, const char**out_text) -> bool {
              auto* real_data = reinterpret_cast<decltype(palette_data.loaded_palettes)::value_type::second_type*>(data);
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
            reinterpret_cast<void*>(&palette_data.loaded_palettes.at(default_palette_name)),
            palette_data.loaded_palettes.at(default_palette_name).second.size()))
      {
        if (selected_palette_index == old_index)
        {
          selected_palette_index = default_palette_index;
        }

        set_default_palette(context.explorer, context.file_info, default_palette_name, default_palette_index);

        refresh_image();
      }

      if (image_type == bitmap_type::earthsiege && ImGui::Button("Set as Shared Default"))
      {
        set_default_palette(context.explorer, "default", default_palette_name);
      }

      ImGui::SameLine();

      if (image_type == bitmap_type::phoenix && ImGui::Button("Reset"))
      {
        auto image_stream = context.explorer.load_file(context.file_info);
        const auto [bmp_type, bmp_data] = load_image_data(image_stream.first, *image_stream.second);

        const auto [name, index]  = detect_default_palette(bmp_data, context.file_info, context.explorer, palette_data.loaded_palettes, true);
        selected_palette_name = default_palette_name = name;
        selected_palette_index = default_palette_index = index;
        set_default_palette(context.explorer, context.file_info, "");
        refresh_image();
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
