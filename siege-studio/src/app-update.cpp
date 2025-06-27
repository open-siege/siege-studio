#include <SDKDDKVer.h>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/shared.hpp>
#include <fstream>
#include <filesystem>
#include <regex>
#include <future>
#include <imagehlp.h>


namespace fs = std::filesystem;

BOOL __stdcall enumerate_embedded_dlls(HMODULE module,
  LPCWSTR type,
  LPWSTR name,
  LONG_PTR lParam);

void unload_core_module(HWND window);
void load_core_module(fs::path);
int register_and_create_main_window(DWORD thread_id, int nCmdShow);
fs::path resolve_install_path(int major_version, int minor_version);


struct embedded_dll
{
  std::filesystem::path filename;
  HGLOBAL handle;
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

std::string resolve_url()
{
  std::string buffer(255, '\0');
  if (auto size = ::GetEnvironmentVariableA("SIEGE_STUDIO_UPDATE_SERVER_URL", buffer.data(), buffer.size() + 1); size != 0 && buffer.starts_with("https:://"))
  {
    buffer.resize(size);

    return buffer;
  }

  return "https://updates.thesiegehub.com";
}

void alloc_console()
{
  if (!context.has_console)
  {
    if (::AllocConsole())
    {
      context.has_console = true;
      ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
    }
  }
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
    alloc_console();

    auto temp_file = fs::temp_directory_path() / "latest-siege-studio-version.txt";
    auto channel = update_type == context.update_development_id ? "development" : "stable";
    std::string server = resolve_url() + "/" + channel + "/latest.txt";

    std::error_code last_error;

    info.max_size = fs::file_size(win32::module_ref::current_application().GetModuleFileName(), last_error);

#if _DEBUG
    info.max_size = info.max_size * 10;
#endif

    fs::remove(temp_file, last_error);

    std::string command = "powershell -WindowStyle hidden -Command wget " + server + " -OutFile " + temp_file.string();
    std::system(command.c_str());

    if (fs::exists(temp_file, last_error))
    {
      std::ostringstream sstr;
      std::ifstream file(temp_file);
      sstr << file.rdbuf();

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
    }
  });
}

__declspec(dllexport) BOOL can_update()
{
  return TRUE;
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
    std::stringstream final_url;
    auto channel = update_type == context.update_development_id ? "development" : "stable";
    final_url << resolve_url() << "/" << channel << "/" << info.available_version.cx << "." << info.available_version.cy << "/" << "siege-studio.exe";
    std::error_code last_error;

    auto value = std::to_string(info.available_version.cx) + "." + std::to_string(info.available_version.cy);

    auto temp_folder = fs::temp_directory_path() / L"siege-studio" / value;

    fs::create_directories(temp_folder, last_error);
    auto temp_file = temp_folder / "siege-studio.exe";
    fs::remove(temp_file, last_error);

    auto task = std::async(std::launch::async, [temp_file, &info]() {
      using namespace std::chrono_literals;

      std::error_code last_error;
      while (true)
      {
        std::shared_ptr<void> deferred{ nullptr, [](...) { std::this_thread::sleep_for(500ms); } };

        if (!fs::exists(temp_file, last_error))
        {
          continue;
        }

        info.current_size = fs::file_size(temp_file, last_error);

        if (last_error)
        {
          break;
        }

        if (info.current_size >= info.max_size)
        {
          break;
        }
      }
    });
    std::string command = "powershell -WindowStyle hidden -Command wget " + final_url.str() + " -OutFile " + temp_file.string();
    std::system(command.c_str());

    if (!fs::exists(temp_file, last_error))
    {
      return;
    }

    auto module = ::LoadLibraryExW(temp_file.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);

    if (!module)
    {
      return;
    }

    std::vector<embedded_dll> embedded_dlls;
    embedded_dlls.reserve(128);

    ::EnumResourceNamesW(module, RT_RCDATA, enumerate_embedded_dlls, (LONG_PTR)&embedded_dlls);

    auto window_thread_id = ::GetWindowThreadProcessId(window, nullptr);

    if (!embedded_dlls.empty())
    {
      ::EnableWindow(window, TRUE);
      unload_core_module(window);
    }

    auto install_path = resolve_install_path(info.available_version.cx, info.available_version.cy);

    std::for_each(embedded_dlls.begin(), embedded_dlls.end(), [&](embedded_dll& dll) {
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

      fs::copy_file(temp_folder / filename_lower, install_path / filename_lower, fs::copy_options::update_existing, last_error);
    });
    ::FreeLibrary(module);

    load_core_module(install_path);
    register_and_create_main_window(window_thread_id, SW_SHOWNORMAL);
  });
}
}