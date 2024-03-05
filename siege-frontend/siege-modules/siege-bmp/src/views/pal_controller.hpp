#ifndef DARKSTARDTSCONVERTER_PAL_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_VIEW_HPP


namespace studio::views
{
  class pal_controller
  {
  public:
    void load_content(std::istream& image_stream);

  private:

    struct rect
    {
      int x;
      int y;
      int width;
      int height;
    };

    struct surface
    {
      rect position;
      studio::content::pal::colour colour;
    };

    std::vector<surface>* rectangles = nullptr;

    std::vector<surface> all_rectangles;
  };
}// namespace studio::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
