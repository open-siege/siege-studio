#ifndef DARKSTARDTSCONVERTER_BMP_VIEW_HPP
#define DARKSTARDTSCONVERTER_BMP_VIEW_HPP

#include <future>
#include <set>
#include <variant>
#include <utility>
#include "graphics_view.hpp"
#include "resources/resource_explorer.hpp"
#include "content/pal/palette.hpp"
#include "content/bmp/bitmap.hpp"

namespace studio::views
{
  class bmp_view 
  {
  public:
    bmp_view(const studio::resources::file_info& info, std::basic_istream<std::byte>& image_stream, const studio::resources::resource_explorer&);
    std::map<sf::Keyboard::Key, std::reference_wrapper<std::function<void(const sf::Event&)>>> get_callbacks();
    void setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);
    void render_gl(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext) {}
    void render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext);

    struct image_data
    {
      int width;
      int height;
      int bit_depth;
      std::vector<std::int32_t> pixels;
    };

    enum class bitmap_type
    {
      unknown,
      microsoft,
      earthsiege,
      phoenix
    };

    using bmp_variant = std::variant<studio::content::bmp::windows_bmp_data, std::vector<studio::content::bmp::dbm_data>, std::vector<content::bmp::pbmp_data>>;

    using palette_map = std::map<std::string_view, std::pair<studio::resources::file_info, std::vector<content::pal::palette>>>;

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

    void bmp_view::load_palettes(const std::vector<studio::resources::file_info>& palettes, const studio::resources::resource_explorer& manager);
    void refresh_image(bool create_new_image = false);

    template<typename IndexType>
    std::size_t get_unique_colours(const std::vector<IndexType>& pixels)
    {
      return std::set<IndexType>(pixels.begin(), pixels.end()).size();
    }

    static std::filesystem::path export_path;
    const studio::resources::resource_explorer& archive;
    studio::resources::file_info info;
    std::list<std::string> sort_order;
    std::map<std::string_view, std::pair<studio::resources::file_info, std::vector<content::pal::palette>>> loaded_palettes;

    int strategy = static_cast<int>(colour_strategy::remap);
    bool opened_folder = false;
    bool scale_changed = false;

    bitmap_type image_type = bitmap_type::unknown;
    std::size_t num_unique_colours = 0;

    selection_state selection_state;

    float image_scale = 1;

    std::vector<image_data> original_pixels;

    sf::Image loaded_image;
    sf::Texture texture;
    sf::Sprite sprite;

    std::future<bool> pending_save;

    std::function<void(const sf::Event&)> zoom_in;
    std::function<void(const sf::Event&)> zoom_out;
  };

}
#endif//DARKSTARDTSCONVERTER_BMP_VIEW_HPP
