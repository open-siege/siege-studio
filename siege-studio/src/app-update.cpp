#include <SDKDDKVer.h>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/http.hpp>
#include <fstream>
#include <filesystem>
#include <regex>
#include <future>
#include <imagehlp.h>
#include <winhttp.h>

namespace fs = std::filesystem;

BOOL __stdcall enumerate_embedded_modules(HMODULE module,
  LPCWSTR type,
  LPWSTR name,
  LONG_PTR lParam);

void unload_core_module(HWND window);
void load_core_module(fs::path);
int register_and_create_main_window(DWORD thread_id, int nCmdShow);

struct discovery_info
{
  fs::path current_app_exe_path;
  fs::path launch_exe_path;
  std::optional<fs::path> matching_installed_exe_path;
  std::optional<fs::path> latest_installed_exe_path;
  std::optional<fs::path> extraction_target_path;
};

discovery_info discover_installation_info(int major_version, int minor_version, std::wstring channel);

struct embedded_module
{
  std::filesystem::path filename;
  HGLOBAL handle;
  WORD lang_id;
};

struct update_context
{

  struct version_info
  {
    SIZE available_version = {};
    std::atomic_bool new_version_is_available = false;
    std::atomic_size_t max_size = 0;
    std::atomic_size_t current_size = 0;
  };

  version_info stable_info{};
  version_info development_info{};

  BOOL update_in_progress = false;
  std::uint32_t update_stable_id = ::RegisterWindowMessageW(L"COMMAND_UPDATE_STABLE");
  std::uint32_t update_development_id = ::RegisterWindowMessageW(L"COMMAND_UPDATE_DEVELOPMENT");
  bool has_console = false;

} context;

std::wstring resolve_domain()
{
  std::wstring buffer(255, '\0');
  if (auto size = ::GetEnvironmentVariableW(L"SIEGE_STUDIO_UPDATE_SERVER_DOMAIN", buffer.data(), buffer.size() + 1); size != 0)
  {
    buffer.resize(size);

    return buffer;
  }

  return L"updates.thesiegehub.com";
}

std::optional<SIZE> get_core_version()
{
  auto parent_path = fs::path(win32::module_ref::current_module().GetModuleFileName()).parent_path();

  auto stem = parent_path.stem().string();

  if (stem.contains("."))
  {
    try
    {
      auto major = stem.substr(0, stem.find('.'));
      auto minor = stem.substr(major.size() + 1);

      return SIZE{ .cx = std::stoi(major), .cy = std::stoi(minor) };
    }
    catch (...)
    {
    }
  }

  return std::nullopt;
}

extern "C" {
__declspec(dllexport) void detect_update(std::uint32_t update_type)
{
  if (!(update_type == context.update_development_id || update_type == context.update_stable_id))
  {
    return;
  }

  auto& info = update_type == context.update_development_id ? context.development_info : context.stable_info;
  info.new_version_is_available = false;

  win32::queue_user_work_item([update_type]() {
    if (!context.has_console)
    {
      context.has_console = ::GetConsoleWindow() != nullptr;
    }

    auto& info = update_type == context.update_development_id ? context.development_info : context.stable_info;

    auto channel = update_type == context.update_development_id ? L"development" : L"stable";
    auto domain = resolve_domain();
    std::wstring remote_path = std::wstring(L"/") + channel + L"/latest.txt";

    std::error_code last_error;

    info.max_size = fs::file_size(win32::module_ref::current_application().GetModuleFileName(), last_error);

    std::ostringstream sstr;
    siege::platform::http_client_context context;
    DWORD transmitted = siege::platform::download_http_data(context, domain, remote_path, sstr);

    if (transmitted == 0)
    {
      return;
    }

    auto value = std::regex_replace(sstr.str(), std::regex("^ +| +$|( ) +"), "$1");

    if (value.contains("."))
    {
      auto major = std::stoi(value.substr(0, value.find(".")));
      auto minor = std::stoi(value.substr(value.find(".") + 1));

      auto version_to_check = get_core_version().value_or(SIZE{ SIEGE_MAJOR_VERSION, SIEGE_MINOR_VERSION });

      if (major >= version_to_check.cx && minor > version_to_check.cy)
      {
        info.new_version_is_available = true;
        info.available_version.cx = major;
        info.available_version.cy = minor;
      }
    }
  });
}

__declspec(dllexport) BOOL can_update()
{
  win32::com::com_string known_folder;
  std::error_code last_error;

  if (auto hresult = ::SHGetKnownFolderPath(FOLDERID_UserProgramFiles, 0, nullptr, known_folder.put()); hresult == S_OK)
  {
    // TODO this should be centralised and possibly stored in an environment variable
    auto root = fs::path(known_folder) / "The Siege Hub" / "Siege Studio";
    auto channel_type = fs::path(SIEGE_CHANNEL_TYPE);
    auto path_info = discover_installation_info(SIEGE_MAJOR_VERSION, SIEGE_MINOR_VERSION, channel_type.wstring());


    auto possible_path = path_info.matching_installed_exe_path
                           .or_else([&]() { return path_info.latest_installed_exe_path; })
                           .or_else([&]() { return path_info.extraction_target_path; })
                           .or_else([&]() { return std::make_optional(fs::path(win32::module_ref::current_module().GetModuleFileName())); });

    if (!(possible_path))
    {
      return FALSE;
    }

    auto temp = fs::relative(*possible_path, root, last_error);

    return !temp.empty() && temp.native()[0] != L'.' ? TRUE : FALSE;
  }
  return FALSE;
}

__declspec(dllexport) BOOL has_update(std::uint32_t update_type)
{
  auto& info = update_type == context.update_development_id ? context.development_info : context.stable_info;
  return info.new_version_is_available;
}

__declspec(dllexport) SIZE get_update_version(std::uint32_t update_type)
{
  auto& info = update_type == context.update_development_id ? context.development_info : context.stable_info;
  return info.available_version;
}

__declspec(dllexport) std::size_t get_max_update_size(std::uint32_t update_type)
{
  auto& info = update_type == context.update_development_id ? context.development_info : context.stable_info;
  return info.max_size;
}

__declspec(dllexport) std::size_t get_current_update_size(std::uint32_t update_type)
{
  auto& info = update_type == context.update_development_id ? context.development_info : context.stable_info;
  return info.current_size;
}

__declspec(dllexport) BOOL is_updating()
{
  return context.update_in_progress;
}

__declspec(dllexport) void apply_update(std::uint32_t update_type, HWND window)
{
  if (!(update_type == context.update_development_id || update_type == context.update_stable_id))
  {
    return;
  }
  auto& info = update_type == context.update_development_id ? context.development_info : context.stable_info;

  if (!info.new_version_is_available)
  {
    return;
  }
  context.update_in_progress = TRUE;
  ::EnableWindow(window, FALSE);
  win32::queue_user_work_item([update_type, window]() mutable {
    std::shared_ptr<void> deferred{ nullptr, [&](...) {
                                     context.update_in_progress = FALSE;
                                     ::EnableWindow(window, TRUE);
                                   } };
    auto& info = update_type == context.update_development_id ? context.development_info : context.stable_info;
    std::wstringstream final_path;
    auto channel = update_type == context.update_development_id ? L"development" : L"stable";
    final_path << channel << L"/" << info.available_version.cx << L"." << info.available_version.cy << L"/" << L"siege-studio.exe";
    std::error_code last_error;

    auto value = std::to_string(info.available_version.cx) + "." + std::to_string(info.available_version.cy);

    auto temp_folder = fs::temp_directory_path() / L"siege-studio" / value;

    fs::create_directories(temp_folder, last_error);
    auto temp_file = temp_folder / "siege-studio.exe";
    fs::remove(temp_file, last_error);

    std::ofstream downloaded_file(temp_file, std::ios::binary | std::ios::trunc);

    siege::platform::http_client_context http_context;
    std::size_t transmitted = download_http_data(http_context, resolve_domain(), final_path.str(), downloaded_file, siege::platform::http_callbacks{ [&info](auto value) {
                                                                                                                                                      info.max_size = value;
                                                                                                                                                    },
                                                                                                                      [&info](auto value) {
                                                                                                                        info.current_size = value;
                                                                                                                      } });

    info.current_size = transmitted;
    if (transmitted == 0)
    {
      return;
    }

    downloaded_file.close();

    auto module = ::LoadLibraryExW(temp_file.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);

    if (!module)
    {
      return;
    }

    std::vector<embedded_module> embedded_modules;
    embedded_modules.reserve(128);

    ::EnumResourceNamesW(module, RT_RCDATA, enumerate_embedded_modules, (LONG_PTR)&embedded_modules);

    auto window_thread_id = ::GetWindowThreadProcessId(window, nullptr);

    if (!embedded_modules.empty())
    {
      ::EnableWindow(window, TRUE);
      unload_core_module(window);
    }
    auto install_info = discover_installation_info(info.available_version.cx, info.available_version.cy, channel);

    if (install_info.extraction_target_path)
    {
      fs::copy_file(temp_file, *install_info.extraction_target_path / temp_file.filename(), fs::copy_options::update_existing, last_error);

      std::for_each(embedded_modules.begin(), embedded_modules.end(), [&](embedded_module& dll) {
        auto entry = ::FindResourceW(module, dll.filename.c_str(), RT_RCDATA);

        if (!entry)
        {
          return;
        }

        auto size = ::SizeofResource(module, entry);
        auto bytes = ::LockResource(dll.handle);
        auto filename_lower = siege::platform::to_lower(dll.filename.wstring());

        {
          std::ofstream output(temp_folder / filename_lower, std::ios::trunc | std::ios::binary);
          output.write((const char*)bytes, size);
        }

        fs::copy_file(temp_folder / filename_lower, *install_info.extraction_target_path / filename_lower, fs::copy_options::update_existing, last_error);
      });
      ::FreeLibrary(module);
      load_core_module(*install_info.extraction_target_path);
      register_and_create_main_window(window_thread_id, SW_SHOWNORMAL);
    }
    else
    {
      ::FreeLibrary(module);
    }
  });
}
}