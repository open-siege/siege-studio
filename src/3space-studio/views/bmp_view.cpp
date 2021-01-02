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


bmp_view::bmp_view(std::basic_istream<std::byte>& image_stream, const studio::fs::file_system_archive& manager)
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
  if (darkstar::bmp::is_microsoft_bmp(image_stream))
  {
    auto windows_bmp = darkstar::bmp::get_bmp_data(image_stream);

    create_image(loaded_image,
      windows_bmp.info.width,
      windows_bmp.info.height,
      windows_bmp.pixels,
      windows_bmp.colours);

    rect.width = windows_bmp.info.width;
    rect.height = windows_bmp.info.height;

    original_pixels = std::move(windows_bmp.pixels);
  }
  else if (darkstar::bmp::is_phoenix_bmp(image_stream))
  {
    auto phoenix_bmp = darkstar::bmp::get_pbmp_data(image_stream);
    std::vector<std::uint8_t> pixels;

    pixels.reserve(phoenix_bmp.bmp_header.width * phoenix_bmp.bmp_header.height * sizeof(std::int32_t));

    auto palettes = manager.find_files({ ".ppl", ".PPL", ".ipl", ".IPL", ".pal", ".PAL" });

    for (auto& palette_info : palettes)
    {
      auto raw_palette = manager.load_file(palette_info);

      if (darkstar::pal::is_phoenix_pal(*raw_palette))
      {
        auto result = loaded_palettes.emplace(palette_info.filename, darkstar::pal::get_ppl_data(*raw_palette));

        if (selected_palette)
        {
          continue;
        }

        for (auto i = 0u; i < result.first->second.size(); ++i)
        {
          auto& child = result.first->second[i];
          if (child.index == phoenix_bmp.palette_index)
          {
            selected_index = default_index = i;
            selected_palette = &result.first->second;
            selected_palette_name = default_palette_name = result.first->first;
            break;
          }
        }
      }
    }

    create_image(loaded_image,
      phoenix_bmp.bmp_header.width,
      phoenix_bmp.bmp_header.height,
      phoenix_bmp.pixels,
      selected_palette ? selected_palette->at(selected_index).colours : default_colours);

    original_pixels = std::move(phoenix_bmp.pixels);
    rect.width = phoenix_bmp.bmp_header.width;
    rect.height = phoenix_bmp.bmp_header.height;
  }

  rect.top = 0;
  rect.left = 0;

  texture.loadFromImage(loaded_image, rect);
  sprite.setTexture(texture);
}

void bmp_view::refresh_image()
{
  auto [width, height] = loaded_image.getSize();
  strategy colour_strat = strategy(colour_strategy);

  if (colour_strat == strategy::do_nothing)
  {
    create_image(loaded_image,
                 width,
                 height,
                 original_pixels,
                 selected_palette->at(selected_index).colours);
  }
  else if (colour_strat == strategy::remap)
  {
    create_image(loaded_image,
                 width,
                 height,
                 darkstar::bmp::remap_bitmap(original_pixels,
                      loaded_palettes.at(default_palette_name).at(default_index).colours,
                      selected_palette->at(selected_index).colours),
                 selected_palette->at(selected_index).colours);
  }

  texture.update(loaded_image);
}

void bmp_view::render_ui(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext)
{
  window->clear();
  window->draw(sprite);

  if (!loaded_palettes.empty())
  {
    ImGui::Begin("Palettes");

    auto child_count = 0;
    for (auto& [key, value] : loaded_palettes)
    {
      if (ImGui::RadioButton(key.c_str(), key == selected_palette_name))
      {
        selected_palette_name = key;
        selected_palette = &value;

        if (loaded_palettes.at(key).size() - 1 < selected_index)
        {
          selected_index = 0;
        }

        refresh_image();
      }

      ImGui::Indent(8);
      for (auto i = 0u; i < value.size(); ++i)
      {
        std::stringstream label;
        label << "Palette " << i + 1;

        if (i == default_index)
        {
          label << " (default for this image)";
        }

        for (auto y = 0; y < child_count; ++y)
        {
          label << " ";
        }

        if (ImGui::RadioButton(label.str().c_str(), key == selected_palette_name && i == selected_index))
        {
          selected_palette_name = key;
          selected_palette = &value;
          selected_index = i;

          refresh_image();
        }
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
  }
}

void bmp_view::setup_gl(sf::RenderWindow* window, wxControl* parent, ImGuiContext* guiContext)
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