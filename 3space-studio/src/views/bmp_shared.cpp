#include "bmp_shared.hpp"
#include "content/pal/palette.hpp"
#include "content/json_boost.hpp"
#include "content/image/image.hpp"

namespace studio::views
{
  void palette_context::load_palettes(const studio::resources::resource_explorer& manager, const std::vector<studio::resources::file_info>& palettes)
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

  std::pair<bitmap_type, bmp_variant> load_image_data_for_pal_detection(const studio::resources::file_info& info, std::basic_istream<std::byte>& image_stream)
  {
    if (content::bmp::is_microsoft_bmp(image_stream) ||
        content::bmp::is_jpg(image_stream) ||
        content::bmp::is_png(image_stream) ||
        content::bmp::is_tga(image_stream))
    {
      return std::make_pair(bitmap_type::microsoft, content::bmp::windows_bmp_data{});
    }
    if (content::bmp::is_earthsiege_bmp(image_stream) || content::bmp::is_earthsiege_bmp_array(image_stream))
    {
      return std::make_pair(bitmap_type::earthsiege, std::vector<studio::content::bmp::dbm_data>{});
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

      return std::make_pair(bitmap_type::phoenix, std::move(frames));
    }
  }

  std::pair<bitmap_type, bmp_variant> load_image_data(const studio::resources::file_info& info, std::basic_istream<std::byte>& image_stream)
  {
    if (content::bmp::is_microsoft_bmp(image_stream))
    {
      return std::make_pair(bitmap_type::microsoft, content::bmp::get_bmp_data(image_stream));
    }
    else if (content::bmp::is_jpg(image_stream))
    {
      return std::make_pair(bitmap_type::microsoft, content::bmp::get_jpg_data(info, image_stream));
    }
    else if (content::bmp::is_png(image_stream))
    {
      return std::make_pair(bitmap_type::microsoft, content::bmp::get_png_data(info, image_stream));
    }
    else if (content::bmp::is_tga(image_stream))
    {
      return std::make_pair(bitmap_type::microsoft, content::bmp::get_tga_data(info, image_stream));
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

      return std::make_pair(bitmap_type::earthsiege, std::move(frames));
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

      return std::make_pair(bitmap_type::phoenix, std::move(frames));
    }
  }

  std::string get_palette_key(const studio::resources::resource_explorer& explorer,
    const studio::resources::file_info& file)
  {
    return (std::filesystem::relative(file.folder_path, explorer.get_search_path()) / file.filename).string();
  }

  std::pair<std::string_view, std::size_t> detect_default_palette(
    const bmp_variant& data,
    const studio::resources::file_info& file,
    const studio::resources::resource_explorer& manager,
    const palette_map& loaded_palettes,
    bool ignore_settings)
  {
    return std::visit([&](const auto& frames) {
      using T = std::decay_t<decltype(frames)>;
      if constexpr (std::is_same_v<T, studio::content::bmp::windows_bmp_data>)
      {
        return std::make_pair(std::string_view("Internal"), std::size_t(0u));
      }

      if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::dbm_data>>)
      {
        if (!ignore_settings)
        {
          const auto settings = default_palette_from_settings(file, manager, loaded_palettes);

          if (settings.has_value())
          {
            return settings.value();
          }
        }

        return std::make_pair(auto_generated_name, std::size_t(0u));
      }

      if constexpr (std::is_same_v<T, std::vector<studio::content::bmp::pbmp_data>>)
      {
        std::size_t index = std::string::npos;
        std::string_view name;

        if (!ignore_settings)
        {
          const auto settings = default_palette_from_settings(file, manager, loaded_palettes);

          if (settings.has_value())
          {
            return settings.value();
          }
        }

        for (auto& phoenix_bmp : frames)
        {
          if (!name.empty())
          {
            break;
          }

          std::vector<const studio::resources::file_info*> results;
          results.reserve(loaded_palettes.size() / 2);

          for (auto& palette : loaded_palettes)
          {
            if (palette.first != auto_generated_name && palette.second.first.folder_path == file.folder_path)
            {
              results.emplace_back(&palette.second.first);
            }
          }

          auto get_defaults = [&](palette_map::const_reference value) {
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
              auto key = get_palette_key(manager, *(*possible_palette));

              if (auto entry = loaded_palettes.find(key); entry != loaded_palettes.end())
              {
                get_defaults(*entry);
              }
            }
            else
            {
              for (auto result : results)
              {
                auto pal_key = get_palette_key(manager, *result);

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
        }

        if (name.empty())
        {
          index = 0;
          name = auto_generated_name;
        }
        return std::make_pair(name, index);
      }
    },
      data);
  }

  std::optional<std::pair<std::string_view, std::size_t>> default_palette_from_settings(
    const studio::resources::file_info& file,
    const studio::resources::resource_explorer& manager,
    const palette_map& loaded_palettes)
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
}
