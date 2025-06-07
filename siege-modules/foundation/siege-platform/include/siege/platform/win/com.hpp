#ifndef WIN32_COM_HPP
#define WIN32_COM_HPP

#include <memory>
#include <optional>
#include <algorithm>
#include <string_view>
#include <stdexcept>
#include <combaseapi.h>

namespace win32::com
{
  struct hresult_throw_on_error
  {
    HRESULT result;
    hresult_throw_on_error(HRESULT result)
    {
      if (result != S_OK)
      {
        std::string err_msg_a(255, '\0');

        err_msg_a.resize(FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
          nullptr,
          result,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          err_msg_a.data(),
          err_msg_a.size(),
          nullptr));
        
        throw std::runtime_error(err_msg_a);
      }
      this->result = result;
    }

    operator HRESULT() const
    {
      return result;
    }
  };

  template<typename TUnknown>
  struct com_deleter
  {
    void operator()(TUnknown* self)
    {
      self->Release();
    }
  };

  struct com_string_deleter
  {
    void operator()(wchar_t* str)
    {
      CoTaskMemFree(str);
    }
  };

  struct com_string : std::unique_ptr<wchar_t[], com_string_deleter>
  {
    using base = std::unique_ptr<wchar_t[], com_string_deleter>;
    using base::base;

    com_string(std::string_view ascii)
    {
      this->reset((wchar_t*)::CoTaskMemAlloc(ascii.size()));

      if (this->get())
      {
        std::transform(ascii.begin(), ascii.end(), this->get(), [](char value) {
          return static_cast<wchar_t>(value);
        });
      }
    }

    operator std::wstring_view()
    {
      return this->get() == nullptr ? L"" : this->get();
    }

    operator std::wstring()
    {
      return this->get() == nullptr ? L"" : this->get();
    }

    wchar_t** put()
    {
      static_assert(sizeof(base) == sizeof(void*));
      return reinterpret_cast<wchar_t**>(this);
    }
  };

  template<typename TUnknown>
  struct com_ptr : std::unique_ptr<TUnknown, com_deleter<TUnknown>>
  {
    using base = std::unique_ptr<TUnknown, com_deleter<TUnknown>>;
    using base::base;

    com_ptr(TUnknown* value) : base(value)
    {
    }

    com_ptr(const com_ptr& other) : base([&]() -> base {
                                      if (other)
                                      {
                                        other->AddRef();
                                      }
                                      return base(other.get());
                                    }())
    {
    }

    template<typename TOther>
    com_ptr<TOther> as()
    {
      static_assert(std::is_same_v<TUnknown, TOther> || std::is_base_of_v<TUnknown, TOther> || std::is_base_of_v<TOther, TUnknown>);

      if (this->get())
      {
        this->get()->AddRef();
      }
      return com_ptr<TOther>(static_cast<TOther*>(this->get()));
    }

    TUnknown** put()
    {
      static_assert(sizeof(base) == sizeof(void*));
      static_assert(std::is_standard_layout_v<com_ptr>);
      return reinterpret_cast<TUnknown**>(this);
    }

    void** put_void()
    {
      static_assert(sizeof(base) == sizeof(void*));
      static_assert(std::is_standard_layout_v<com_ptr>);
      return reinterpret_cast<void**>(this);
    }
  };

  HRESULT init_com(COINIT apartment_model = COINIT_MULTITHREADED);
}// namespace win32::com

#endif