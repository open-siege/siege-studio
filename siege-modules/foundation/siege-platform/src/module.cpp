#include <siege/platform/win/module.hpp>
#include <filesystem>
#include <optional>

namespace win32
{
  std::optional<std::filesystem::path> find_binary_module(search_context context)
  {
    if (context.search_user_dll_paths)
    {
      auto module = win32::module(::LoadLibraryExW(context.module_name.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_USER_DIRS));

      if (module)
      {
        return module.GetModuleFileName();
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
}