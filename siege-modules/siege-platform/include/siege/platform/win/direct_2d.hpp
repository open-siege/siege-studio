#ifndef WIN_DESKTOP_DIRECT_2D_HPP
#define WIN_DESKTOP_DIRECT_2D_HPP

#include <vector>
#include <cstdint>
#include <span>
#include <siege/platform/win/com.hpp>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/direct_write.hpp>
#include <siege/platform/win/wic.hpp>
#include <d2d1.h>
#include <d2d1_3.h>

namespace win32::direct2d
{
  using namespace win32::com;

  inline auto& get_factory()
  {
    thread_local com_ptr factory = [] {
      com_ptr<ID2D1Factory> temp;

      if (::D2D1CreateFactory<ID2D1Factory>(D2D1_FACTORY_TYPE_SINGLE_THREADED, temp.put()) != S_OK)
      {
        throw std::exception("Could not create imaging factory");
      }

      return temp;
    }();

    return *factory;
  }

  class dc_render_target;

  class geometry
  {
  public:
    friend class dc_render_target;

    ID2D1Geometry& object()
    {
      return *instance;
    }

  protected:
    template<typename T>
    geometry(com_ptr<T> other) : instance(other.release())
    {
    }
    com_ptr<ID2D1Geometry> instance;
  };

  class rounded_rectangle_geometry : public geometry
  {
  public:
    rounded_rectangle_geometry(D2D1_ROUNDED_RECT rect) : geometry([rect] {
                                                           com_ptr<ID2D1RoundedRectangleGeometry> instance;
                                                           hresult_throw_on_error(get_factory().CreateRoundedRectangleGeometry(rect, instance.put()));
                                                           return instance;
                                                         }())
    {
    }
  };

  class rectangle_geometry : public geometry
  {
  public:
    rectangle_geometry(D2D1_RECT_F rect) : geometry([rect] {
                                             com_ptr<ID2D1RectangleGeometry> instance;
                                             hresult_throw_on_error(get_factory().CreateRectangleGeometry(rect, instance.put()));
                                             return instance;
                                           }())
    {
    }
  };

  class ellipse_geometry : public geometry
  {
  public:
    ellipse_geometry(D2D1_ELLIPSE ellipse) : geometry([ellipse] {
                                               com_ptr<ID2D1EllipseGeometry> instance;
                                               hresult_throw_on_error(get_factory().CreateEllipseGeometry(ellipse, instance.put()));
                                               return instance;
                                             }())
    {
    }
  };

  class path_geometry : public geometry
  {
  public:
    path_geometry() : geometry([] {
                        com_ptr<ID2D1PathGeometry> instance;
                        hresult_throw_on_error(get_factory().CreatePathGeometry(instance.put()));
                        return instance;
                      }())
    {
    }
  };

  class geometry_group : public geometry
  {
  public:
    geometry_group(D2D1_FILL_MODE mode, std::span<geometry> geometries) : geometry([=] {
                                                                            com_ptr<ID2D1GeometryGroup> instance;

                                                                            std::vector<ID2D1Geometry*> raw;
                                                                            raw.resize(geometries.size());

                                                                            std::transform(geometries.begin(), geometries.end(), raw.begin(), [](auto& value) {
                                                                              return &value.object();
                                                                            });

                                                                            hresult_throw_on_error(get_factory().CreateGeometryGroup(mode, raw.data(), (UINT32)raw.size(), instance.put()));
                                                                            return instance;
                                                                          }())
    {
    }
  };

  class transformed_geometry : public geometry
  {
    transformed_geometry(geometry& other, D2D1_MATRIX_3X2_F transform) : geometry([&] {
                                                                           com_ptr<ID2D1TransformedGeometry> instance;
                                                                           hresult_throw_on_error(get_factory().CreateTransformedGeometry(&other.object(), transform, instance.put()));
                                                                           return instance;
                                                                         }())
    {
    }
  };

  class brush
  {
  public:
    friend class render_target;

    ID2D1Brush& object()
    {
      return *instance;
    }

  protected:
    template<typename T>
    brush(com_ptr<T> other) : instance(other.release())
    {
    }
    com_ptr<ID2D1Brush> instance;
  };

  class solid_color_brush : public brush
  {

  public:
    solid_color_brush(com_ptr<ID2D1RenderTarget>& target, D2D1_COLOR_F color, D2D1_BRUSH_PROPERTIES props) : brush([&] {
                                                                                                               com_ptr<ID2D1SolidColorBrush> instance;
                                                                                                               hresult_throw_on_error(target->CreateSolidColorBrush(color, props, instance.put()));
                                                                                                               return instance;
                                                                                                             }())
    {
    }

    solid_color_brush(com_ptr<ID2D1RenderTarget>& target, D2D1_COLOR_F color) : brush([&] {
                                                                                  com_ptr<ID2D1SolidColorBrush> instance;
                                                                                  hresult_throw_on_error(target->CreateSolidColorBrush(color, instance.put()));
                                                                                  return instance;
                                                                                }())
    {
    }
  };

  class gdi_interop_render_target
  {
  public:
    friend class render_target;

    win32::gdi::drawing_context_ref get_dc(D2D1_DC_INITIALIZE_MODE mode)
    {
      HDC result = nullptr;

      hresult_throw_on_error(instance->GetDC(mode, &result));

      return win32::gdi::drawing_context_ref(result);
    }

    void release_dc(std::optional<RECT> rect = std::nullopt)
    {
      if (rect)
      {
        hresult_throw_on_error(instance->ReleaseDC(&*rect));
      }
      else
      {
        hresult_throw_on_error(instance->ReleaseDC(nullptr));
      }
    }

  private:
    gdi_interop_render_target(com_ptr<ID2D1RenderTarget>& target)
    {
      target->QueryInterface<ID2D1GdiInteropRenderTarget>(instance.put());

      if (!instance)
      {
        throw new std::invalid_argument("Render target could not be cast to interop target");
      }
    }

    com_ptr<ID2D1GdiInteropRenderTarget> instance;
  };

  class device_context
  {
  public:
    bool supports_svg_rendering()
    {
      return std::holds_alternative<com_ptr<ID2D1DeviceContext5>>(instance);
    }

  private:
    friend class render_target;

    device_context(com_ptr<ID2D1DeviceContext> context) : instance(std::move(context))
    {
    }

    device_context(com_ptr<ID2D1DeviceContext2> context) : instance(std::move(context))
    {
    }

    device_context(com_ptr<ID2D1DeviceContext5> context) : instance(std::move(context))
    {
    }

    std::variant<com_ptr<ID2D1DeviceContext>, com_ptr<ID2D1DeviceContext2>, com_ptr<ID2D1DeviceContext5>> instance;
  };

  class layer
  {
    friend class render_target;

  private:
    layer(com_ptr<ID2D1RenderTarget>& target)
    {
      hresult_throw_on_error(target->CreateLayer(instance.put()));
    }

    com_ptr<ID2D1Layer> instance;
  };


  class render_target
  {
  public:
    void begin_draw()
    {
      instance->BeginDraw();
    }

    virtual bool end_draw() = 0;

    gdi_interop_render_target get_interop_render_target()
    {
      return gdi_interop_render_target(instance);
    }

    std::optional<device_context> as_device_context()
    {
      ID2D1DeviceContext5* context_5 = nullptr;

      if (instance->QueryInterface<ID2D1DeviceContext5>(&context_5) == S_OK)
      {
        return device_context(com_ptr<ID2D1DeviceContext5>(context_5));
      }

      ID2D1DeviceContext2* context_2 = nullptr;

      if (instance->QueryInterface<ID2D1DeviceContext2>(&context_2) == S_OK)
      {
        return device_context(com_ptr<ID2D1DeviceContext2>(context_2));
      }

      ID2D1DeviceContext* context = nullptr;

      if (instance->QueryInterface<ID2D1DeviceContext>(&context) == S_OK)
      {
        return device_context(com_ptr<ID2D1DeviceContext>(context));
      }

      return std::nullopt;
    }

    solid_color_brush create_solid_color_brush(D2D1_COLOR_F color, D2D1_BRUSH_PROPERTIES props)
    {
      return solid_color_brush(instance, color, props);
    }

    solid_color_brush create_solid_color_brush(D2D1_COLOR_F color)
    {
      return solid_color_brush(instance, color);
    }

    void draw_rectangle(D2D1_RECT_F rect, brush& brush)
    {
      instance->DrawRectangle(rect, brush.instance.get());
    }

    void fill_rectangle(D2D1_RECT_F rect, brush& brush)
    {
      instance->FillRectangle(rect, brush.instance.get());
    }

    void draw_text(std::wstring_view text, directwrite::text_format& format, D2D1_RECT_F rect, brush& fill_brush, D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE mode = DWRITE_MEASURING_MODE_NATURAL)
    {
      instance->DrawTextW(text.data(), (UINT32)text.size(), &format.object(), rect, fill_brush.instance.get(), options, mode);
    }

    layer create_layer()
    {
      return layer(instance);
    }

    void push_layer(D2D1_LAYER_PARAMETERS params, layer& layer)
    {
      instance->PushLayer(params, layer.instance.get());
    }

    void pop_layer()
    {
      instance->PopLayer();
    }

    void clear(D2D1_COLOR_F color)
    {
      instance->Clear(color);
    }

  protected:
    template<typename TRenderTarget>
    render_target(com_ptr<TRenderTarget> instance) : instance(instance.release())
    {
    }

    com_ptr<ID2D1RenderTarget> instance;
  };

  class wic_bitmap_render_target : public render_target
  {
  public:
    wic_bitmap_render_target(win32::wic::bitmap& bitmap, D2D1_RENDER_TARGET_PROPERTIES props) : render_target([&] {
                                                                                                  com_ptr<ID2D1RenderTarget> temp;
                                                                                                  auto& factory = get_factory();
                                                                                                  auto bitmap_handle = bitmap.handle<IWICBitmap>();
                                                                                                  hresult_throw_on_error(factory.CreateWicBitmapRenderTarget(bitmap_handle.get(), &props, temp.put()));
                                                                                                  return temp;
                                                                                                }())
    {
    }

    bool end_draw() override
    {
      auto result = instance->EndDraw();
      return false;
    }
  };

  class dc_render_target : public render_target
  {
  public:
    dc_render_target(D2D1_RENDER_TARGET_PROPERTIES props) : render_target([&] {
                                                              com_ptr<ID2D1DCRenderTarget> temp;
                                                              auto& factory = get_factory();
                                                              hresult_throw_on_error(factory.CreateDCRenderTarget(&props, temp.put()));
                                                              return temp;
                                                            }())
    {
    }

    inline static dc_render_target& for_thread(D2D1_RENDER_TARGET_PROPERTIES props)
    {
      thread_local std::map<std::string, dc_render_target> cache;

      std::string key;
      key.resize(sizeof(props));
      std::memcpy(key.data(), &props, sizeof(props));

      auto result = cache.find(key);

      if (result == cache.end())
      {
        result = cache.emplace(std::move(key), props).first;
      }

      return result->second;
    }

    bool end_draw() override
    {
      auto result = instance->EndDraw();

      if (result == D2DERR_RECREATE_TARGET)
      {
        D2D1_RENDER_TARGET_PROPERTIES props{};
        if (!instance->IsSupported(&props))
        {
          props.dpiX = 0;
          props.dpiY = 0;
          props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
          props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
          props.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;
          props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
        }

        instance.reset();

        auto& factory = get_factory();
        com_ptr<ID2D1DCRenderTarget> temp;
        hresult_throw_on_error(factory.CreateDCRenderTarget(&props, temp.put()));
        instance.reset(temp.release());

        return false;
      }

      hresult_throw_on_error temp(result);

      return temp.result == S_OK;
    }


    ID2D1DCRenderTarget& object()
    {
      return (ID2D1DCRenderTarget&)*instance;
    }

    void bind_dc(win32::gdi::drawing_context_ref dc, RECT dimensions)
    {
      hresult_throw_on_error(object().BindDC(dc.get(), &dimensions));
    }
  };

}// namespace win32::direct2d

#endif