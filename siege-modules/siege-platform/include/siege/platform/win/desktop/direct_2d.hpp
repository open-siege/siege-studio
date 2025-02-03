#ifndef WIN_DESKTOP_DIRECT_2D_HPP
#define WIN_DESKTOP_DIRECT_2D_HPP

#include <vector>
#include <cstdint>
#include <span>
#include <siege/platform/win/core/com/base.hpp>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <d2d1.h>
#include <d2d1_3.h>

namespace win32::direct2d
{
  using namespace win32::com;

  auto& get_factory()
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
  class gdi_interop_render_target
  {
  public:
    friend class dc_render_target;

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
    gdi_interop_render_target(com_ptr<ID2D1DCRenderTarget>& target)
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
    friend class dc_render_target;

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

  class dc_render_target
  {
  public:
    dc_render_target(D2D1_RENDER_TARGET_PROPERTIES props)
    {
      auto& factory = get_factory();
      hresult_throw_on_error(factory.CreateDCRenderTarget(&props, instance.put()));
    }

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

    void begin_draw()
    {
      instance->BeginDraw();
    }

    bool end_draw()
    {
      auto result = instance->EndDraw();

      if (result == D2DERR_RECREATE_TARGET)
      {
        D2D1_RENDER_TARGET_PROPERTIES props;
        hresult_throw_on_error(instance->IsSupported(&props));

        instance.reset();

        auto& factory = get_factory();
        hresult_throw_on_error(factory.CreateDCRenderTarget(&props, instance.put()));


        return false;
      }

      hresult_throw_on_error temp(result);

      return temp.result == S_OK;
    }


    ID2D1DCRenderTarget& object()
    {
      return *instance;
    }

    void bind_dc(win32::gdi::drawing_context_ref dc, RECT dimensions)
    {
     hresult_throw_on_error(instance->BindDC(dc.get(), &dimensions));
    }

  private:
    win32::com::com_ptr<ID2D1DCRenderTarget> instance;
  };

}// namespace win32::direct2d

#endif