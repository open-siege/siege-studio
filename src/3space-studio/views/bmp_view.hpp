#ifndef DARKSTARDTSCONVERTER_BMP_VIEW_HPP
#define DARKSTARDTSCONVERTER_BMP_VIEW_HPP

#include "graphics_view.hpp"
#include "resource/resource_explorer.hpp"
#include "content/palette.hpp"

namespace studio::views
{
  class bmp_view : public graphics_view
  {
  public:
    bmp_view(const studio::resource::file_info& info, std::basic_istream<std::byte>& image_stream, const studio::resource::resource_explorer&);
    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks() override;
    void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) override;
    void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) override {}
    void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) override;

  private:
    enum class colour_strategy : int
    {
      do_nothing,
      remap,
      remap_unique
    };

    struct selection_state
    {
      std::string_view selected_palette_name;
      std::size_t selected_palette_index = std::string::npos;

      std::string_view default_palette_name;
      std::size_t default_palette_index = std::string::npos;
      int selected_bitmap_index = 0;
    };

    void refresh_image();

    std::size_t get_unique_colours(const std::vector<std::int32_t>& pixels);

    static std::filesystem::path export_path;
    const studio::resource::resource_explorer& archive;
    studio::resource::file_info info;
    std::list<std::string> sort_order;
    std::map<std::string_view, std::pair<studio::resource::file_info, std::vector<content::pal::palette>>> loaded_palettes;

    int strategy = static_cast<int>(colour_strategy::remap);
    bool opened_folder = false;
    bool scale_changed = false;
    bool is_phoenix_bitmap = false;
    int bit_depth = 8;
    std::size_t num_unique_colours = 0;

    selection_state selection_state;

    float image_scale = 1;

    std::vector<std::vector<std::int32_t>> original_pixels;

    sf::Image loaded_image;
    sf::Texture texture;
    sf::Sprite sprite;

    std::function<void(const sf::Event&)> zoom_in;
    std::function<void(const sf::Event&)> zoom_out;
  };

}
#endif//DARKSTARDTSCONVERTER_BMP_VIEW_HPP
