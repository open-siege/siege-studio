#include "bmp_view.hpp"
#include "content/bitmap.hpp"


void create_image(sf::Image& loaded_image,
  int width,
  int height,
  const std::vector<std::byte>& pixels,
  const std::vector<darkstar::pal::colour>& colours)
{
  std::vector<std::uint8_t> rendered_pixels;
  rendered_pixels.reserve(width * height * sizeof(std::int32_t));

  for (auto index : pixels)
  {
    auto& colour = colours[int(index)];
    rendered_pixels.emplace_back(int(colour.red));
    rendered_pixels.emplace_back(int(colour.green));
    rendered_pixels.emplace_back(int(colour.blue));
    rendered_pixels.emplace_back(int(colour.flags));
  }

  loaded_image.create(width, height, rendered_pixels.data());
}


bmp_view::bmp_view(const shared::archive::file_info& info, std::basic_istream<std::byte>& image_stream, const studio::fs::file_system_archive& manager)
{
  default_colours.reserve(256);
  for (auto i = 0; i < 256; ++i)
  {
    auto colour1 = std::byte(i);
    auto colour2 = std::byte(256 - i);
    auto colour3 = std::byte(std::rand() % 256);
    default_colours.emplace_back(darkstar::pal::colour{ colour1, colour2, colour3, std::byte(0xFF) });
  }

  sf::IntRect rect;

  std::vector<shared::archive::file_info> palettes;


  palettes = manager.find_files(studio::fs::file_system_archive::get_archive_path(info.folder_path).parent_path(), { ".ppl", ".PPL", ".ipl", ".IPL", ".pal", ".PAL" });

  auto all_palettes = manager.find_files({ ".ppl", ".ipl", ".pal" });

  studio::fs::file_system_archive::merge_results(palettes, all_palettes);


  for (auto& palette_info : palettes)
  {
    auto raw_palette = manager.load_file(palette_info);

    if (darkstar::pal::is_phoenix_pal(*raw_palette.second))
    {
      loaded_palettes.emplace(sort_order.emplace_back(palette_info.filename.string()), darkstar::pal::get_ppl_data(*raw_palette.second));
    }
    else if (darkstar::pal::is_microsoft_pal(*raw_palette.second))
    {
      std::vector<darkstar::pal::palette> temp;
      temp.emplace_back().colours = darkstar::pal::get_pal_data(*raw_palette.second);

      loaded_palettes.emplace(sort_order.emplace_back(palette_info.filename.string()), std::move(temp));
    }
  }

  if (darkstar::bmp::is_microsoft_bmp(image_stream))
  {
    auto windows_bmp = darkstar::bmp::get_bmp_data(image_stream);

    std::vector<darkstar::pal::palette> temp;
    temp.emplace_back().colours = windows_bmp.colours;

    loaded_palettes.emplace(sort_order.emplace_back("Internal"), std::move(temp));

    selected_palette_index = default_palette_index = 0;
    selected_palette_name = default_palette_name = "Internal";

    create_image(loaded_image,
      windows_bmp.info.width,
      windows_bmp.info.height,
      windows_bmp.pixels,
      windows_bmp.colours);

    rect.width = windows_bmp.info.width;
    rect.height = windows_bmp.info.height;

    original_pixels.emplace_back(std::move(windows_bmp.pixels));
  }
  else
  {
    std::vector<darkstar::bmp::pbmp_data> frames;

    if (darkstar::bmp::is_phoenix_bmp(image_stream))
    {
      frames.emplace_back(darkstar::bmp::get_pbmp_data(image_stream));
    }
    else if (darkstar::bmp::is_phoenix_bmp_array(image_stream))
    {
      frames = darkstar::bmp::get_pba_data(image_stream);
    }

    for (auto& phoenix_bmp : frames)
    {
      if (original_pixels.empty())
      {
        if (!selected_palette_name.empty())
        {
          continue;
        }


        std::vector<decltype(palettes)::pointer> results;
        results.reserve(palettes.size() / 2);

        for (auto& palette : palettes)
        {
          if (palette.folder_path == info.folder_path)
          {
            results.emplace_back(&palette);
          }
        }

        auto get_defaults = [&](auto& value) {
          for (auto i = 0u; i < value.second.size(); ++i)
          {
            auto& child = value.second[i];
            if (child.index == phoenix_bmp.palette_index)
            {
              selected_palette_index = default_palette_index = i;
              selected_palette_name = default_palette_name = value.first;
              break;
            }
          }
        };

        if (results.empty())
        {
          for (auto& entry : loaded_palettes)
          {
            if (entry.second.size() > phoenix_bmp.palette_index)
            {
              get_defaults(entry);
            }

            if (selected_palette_index != std::string::npos)
            {
              break;
            }
          }
        }
        else
        {
          for (auto result : results)
          {
            if (auto entry = loaded_palettes.find(result->filename.string()); entry != loaded_palettes.end())
            {
              get_defaults(*entry);
            }

            if (selected_palette_index != std::string::npos)
            {
              break;
            }
          }
        }

        create_image(loaded_image,
          phoenix_bmp.bmp_header.width,
          phoenix_bmp.bmp_header.height,
          phoenix_bmp.pixels,
          selected_palette_name.empty() ? default_colours : loaded_palettes.at(selected_palette_name).at(selected_palette_index).colours);

        rect.width = phoenix_bmp.bmp_header.width;
        rect.height = phoenix_bmp.bmp_header.height;
      }

      original_pixels.emplace_back(std::move(phoenix_bmp.pixels));
    }
  }

  sort_order.sort([&](const auto& a, const auto& b) {
    return (a == "Internal") || loaded_palettes.at(a).size() < loaded_palettes.at(b).size();
  });

  if (!original_pixels.empty())
  {
    rect.top = 0;
    rect.left = 0;

    texture.loadFromImage(loaded_image, rect);
    sprite.setTexture(texture);
  }
}

void bmp_view::refresh_image()
{
  if (!original_pixels.empty())
  {
    auto [width, height] = loaded_image.getSize();
    auto colour_strat = strategy(colour_strategy);

    if (colour_strat == strategy::do_nothing)
    {
      create_image(loaded_image,
        width,
        height,
        original_pixels.at(selected_bitmap_index),
        loaded_palettes.at(selected_palette_name).at(selected_palette_index).colours);
    }
    else if (colour_strat == strategy::remap)
    {
      create_image(loaded_image,
        width,
        height,
        darkstar::bmp::remap_bitmap(original_pixels.at(selected_bitmap_index),
          loaded_palettes.at(default_palette_name).at(default_palette_index).colours,
          loaded_palettes.at(selected_palette_name).at(selected_palette_index).colours),
        loaded_palettes.at(selected_palette_name).at(selected_palette_index).colours);
    }

    texture.update(loaded_image);
  }
}

void bmp_view::render_ui(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext)
{
  window->clear();
  window->draw(sprite);

  if (!loaded_palettes.empty())
  {
    ImGui::Begin("Palettes");

    auto child_count = 0;
    for (auto& key : sort_order)
    {
      auto& value = loaded_palettes.at(key);

      if (ImGui::RadioButton(key.c_str(), key == selected_palette_name))
      {
        selected_palette_name = key;

        if (loaded_palettes.at(key).size() - 1 < selected_palette_index)
        {
          selected_palette_index = 0;
        }

        refresh_image();
      }

      ImGui::Indent(8);

      std::string placeholder;
      placeholder.reserve(std::strlen("Palette ") + std::strlen(" (default for this image)") + child_count);

      std::stringstream label(placeholder);

      for (auto i = 0u; i < value.size(); ++i)
      {
        label << "Palette " << i + 1;

        if (key == default_palette_name && i == default_palette_index)
        {
          label << " (default for this image)";
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

      ImGui::Unindent(8);
    }

    ImGui::End();

    ImGui::Begin("Colour Strategy");
    if (ImGui::RadioButton("Do Nothing", &colour_strategy, 0))
    {
      refresh_image();
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Remap", &colour_strategy, 1))
    {
      refresh_image();
    }
    ImGui::SameLine();

    if (ImGui::RadioButton("Remap (only unique colours)", &colour_strategy, 2))
    {
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
          refresh_image();
        }
        label.str("");
      }
      ImGui::End();
    }
  }
}

void bmp_view::setup_view(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext)
{
  auto [width, height] = parent->GetClientSize();

  auto image_width = sprite.getTexture()->getSize().x;
  auto image_height = sprite.getTexture()->getSize().y;

  auto width_ratio = float(image_width) / width;
  auto height_ratio = float(image_height) / height;

  sf::FloatRect visibleArea(0, 0, image_width, image_height);
  sf::View view(visibleArea);
  view.setViewport(sf::FloatRect(0.5 - width_ratio / 2, 0.5 - height_ratio / 2, width_ratio, height_ratio));
  window->setView(view);
}