module;
#include <siege/platform/windows.hpp>
export module win32.module;
import std;
export import win32.utils;

export namespace win32
{
  using ::HMODULE;
  using ::HINSTANCE;

  export template<typename TDeleter>
  struct module_base : win32::auto_handle<HMODULE, TDeleter>
  {
    using base = win32::auto_handle<HMODULE, TDeleter>;
    using base::base;

    template<typename TPointer = void*>
    auto GetProcAddress(WORD ordinal) const
    {
      static_assert(std::is_pointer_v<TPointer>);
      return reinterpret_cast<TPointer>(::GetProcAddress(*this, MAKEINTRESOURCEA(ordinal)));
    }

    template<typename TPointer = void*>
    auto GetProcAddress(std::string name) const
    {
      static_assert(std::is_pointer_v<TPointer>);
      return reinterpret_cast<TPointer>(::GetProcAddress(*this, name.c_str()));
    }

    // Named GetModuleFileNameW so call sites still compile when Windows.h
    // defines GetModuleFileName -> GetModuleFileNameW.
    template<typename TChar = wchar_t>
    std::basic_string<TChar> GetModuleFileNameW() const
    {
      if constexpr (sizeof(TChar) == sizeof(char16_t))
      {
        std::basic_string<TChar> result(256, '\0');
        ::GetModuleFileNameW(this->get(), reinterpret_cast<wchar_t*>(result.data()), result.size());

        result.erase(result.find(TChar('\0')));
        return result;
      }
      else if constexpr (sizeof(TChar) == sizeof(char8_t))
      {
        std::basic_string<TChar> result(256, '\0');
        ::GetModuleFileNameA(this->get(), reinterpret_cast<char*>(result.data()), result.size());

        result.erase(result.find(TChar('\0')));
        return result;
      }

      return std::basic_string<TChar>{};
    }

    template<typename TChar = wchar_t>
    std::basic_string<TChar> GetModuleFileName() const
    {
      return GetModuleFileNameW<TChar>();
    }
  };

  export struct module_no_deleter
  {
    void operator()(HMODULE lib)
    {
    }
  };

  export struct module_ref : module_base<module_no_deleter>
  {
    using base = module_base<module_no_deleter>;
    using base::base;

    explicit module_ref(void* func) : base([=]() {
                                        HMODULE temp = nullptr;

                                        ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                          (LPCWSTR)func,
                                          &temp);

                                        if (!temp)
                                        {
                                          throw std::system_error(std::error_code(::GetLastError(), std::system_category()));
                                        }

                                        return temp;
                                      }())
    {
    }

    static module_ref current_module()
    {
      return module_ref((void*)&current_module);
    }

    static module_ref current_application()
    {
      return module_ref(::GetModuleHandleW(nullptr));
    }
  };

  export struct module_deleter
  {
    void operator()(HMODULE lib)
    {
      ::FreeLibrary(lib);
    }
  };

  export struct module : module_base<module_deleter>
  {
    using base = module_base<module_deleter>;
    using base::base;

    explicit module(std::filesystem::path path, bool is_system = false) : base([=]() {
                                                                            if (!is_system && !std::filesystem::exists(path))
                                                                            {
                                                                              throw std::invalid_argument("path");
                                                                            }

                                                                            HMODULE temp = ::LoadLibraryW(path.c_str());

                                                                            if (!temp)
                                                                            {
                                                                              throw std::system_error(std::error_code(::GetLastError(), std::system_category()));
                                                                            }

                                                                            return temp;
                                                                          }())
    {
    }

    auto ref()
    {
      return module_ref(get());
    }
  };

  export DLL_DIRECTORY_COOKIE add_dll_directory(const wchar_t* path)
  {
    static auto module = ::GetModuleHandleW(L"kernel32.dll");

    using add_dll_proc = std::add_pointer_t<decltype(::AddDllDirectory)>;
    static auto proc = (add_dll_proc)::GetProcAddress(module, "AddDllDirectory");

    if (proc)
    {
      return proc(path);
    }

    // TODO add the path to the PATH environment variable as a fallback
    return 0;
  }

  export BOOL remove_dll_directory(DLL_DIRECTORY_COOKIE token)
  {
    static auto module = ::GetModuleHandleW(L"kernel32.dll");

    using add_dll_proc = std::add_pointer_t<decltype(::RemoveDllDirectory)>;
    static auto proc = (add_dll_proc)::GetProcAddress(module, "RemoveDllDirectory");

    if (proc)
    {
      return proc(token);
    }

    // TODO add the path to the PATH environment variable as a fallback
    return FALSE;
  }

  export struct search_context
  {
    std::filesystem::path module_name;
    bool search_user_dll_paths = true;
    bool search_app_path = true;
    bool search_dll_search_path = true;
    bool search_current_path = true;
    bool search_env_paths = false;
    bool search_temp_path = false;
  };

  export std::optional<std::filesystem::path> find_binary_module(search_context context)
  {
    if (context.search_user_dll_paths)
    {
      auto module = win32::module(::LoadLibraryExW(context.module_name.c_str(), nullptr, LOAD_LIBRARY_SEARCH_USER_DIRS));

      if (module)
      {
        auto result = module.GetModuleFileName();

        if (!result.empty())
        {
          return result;
        }
      }
    }

    std::vector<std::filesystem::path> search_paths;

    if (context.search_app_path)
    {
      auto path = std::filesystem::path{ win32::module_ref::current_application().GetModuleFileName() };
      search_paths.emplace_back(path.parent_path());
    }

    if (context.search_dll_search_path)
    {
      std::wstring temp;
      temp.resize(1024);
      temp.resize(GetDllDirectoryW((DWORD)temp.size(), temp.data()));

      if (!temp.empty())
      {
        search_paths.emplace_back(std::move(temp));
      }
    }

    if (context.search_current_path)
    {
      search_paths.emplace_back(std::filesystem::current_path());
    }

    if (context.search_env_paths)
    {
      std::wstring env;
      env.resize(::GetEnvironmentVariableW(L"Path", env.data(), 0));
      env.resize(::GetEnvironmentVariableW(L"Path", env.data(), (DWORD)env.size()));

      if (!env.empty())
      {
        std::wstring_view env_view = env;

        do
        {
          auto next = env_view.find(L";");
          auto sub = env_view.substr(0, next);

          if (!sub.empty())
          {
            search_paths.emplace_back(sub);
          }

          env_view = env_view.substr(next + 1);
        } while (env_view.contains(L";"));
      }
    }

    if (context.search_temp_path)
    {
      search_paths.emplace_back(std::filesystem::temp_directory_path());
    }

    std::error_code last_error;
    for (auto& parent : search_paths)
    {
      if (auto potential_path = parent / context.module_name; std::filesystem::exists(potential_path, last_error))
      {
        return potential_path;
      }
    }

    return std::nullopt;
  }

  export std::expected<std::filesystem::path, DWORD> get_dll_directory()
  {
    std::wstring temp;
    temp.resize(1024);
    temp.resize(::GetDllDirectoryW((DWORD)temp.size(), temp.data()));

    if (temp.size() == 0)
    {
      return std::unexpected(::GetLastError());
    }

    return temp;
  }
}
