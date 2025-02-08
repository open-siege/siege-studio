#ifndef WIN_ANIMATION_HPP
#define WIN_ANIMATION_HPP

#include <siege/platform/win/core/com/base.hpp>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <uianimation.h>

namespace win32::animation
{
  using namespace win32::com;

  struct animation_objects
  {
    IUIAnimationManager* manager;
    IUIAnimationTransitionLibrary* library;
    IUIAnimationTimer* timer;
  };

  inline animation_objects& get_ojects()
  {
    static animation_objects objects = [] {
      auto existing = ::OpenFileMappingW(FILE_MAP_WRITE, FALSE, L"SiegeAppAnimationObjects");

      animation_objects objects{
        .manager = nullptr,
        .library = nullptr,
        .timer = nullptr,
      };
      if (::CoCreateInstance(CLSID_UIAnimationManager, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAnimationManager), (void**)&objects.manager) != S_OK)
      {
        throw std::exception("Could not create imaging factory");
      }

      if (::CoCreateInstance(CLSID_UIAnimationTransitionLibrary, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAnimationTransitionLibrary), (void**)&objects.library) != S_OK)
      {
        throw std::exception("Could not create imaging factory");
      }

      if (::CoCreateInstance(CLSID_UIAnimationTimer, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAnimationTimer), (void**)&objects.timer) != S_OK)
      {
        throw std::exception("Could not create imaging factory");
      }

      return objects;
    }();


    return objects;
  }

  inline auto& get_manager()
  {
    static com_ptr<IUIAnimationManager> manager(get_ojects().manager);

    return *manager;
  }

  inline auto& get_transition_library()
  {
    static com_ptr<IUIAnimationTransitionLibrary> library(get_ojects().library);

    return *library;
  }

  inline auto& get_timer()
  {
    static com_ptr<IUIAnimationTimer> timer(get_ojects().timer);

    return *timer;
  }

  class animation_variable
  {
  public:
    animation_variable(double initial_value) : instance([initial_value] {
                                                 com_ptr<IUIAnimationVariable> temp;
                                                 get_manager().CreateAnimationVariable(initial_value, temp.put());
                                                 return temp.release();
                                               }())
    {
    }

  private:
    com_ptr<IUIAnimationVariable> instance;
  };
}// namespace win32::animation

#endif