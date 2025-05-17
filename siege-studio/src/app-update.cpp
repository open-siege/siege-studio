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


BOOL __stdcall extract_embedded_dlls(HMODULE module,
  LPCWSTR type,
  LPWSTR name,
  LONG_PTR lParam);

void unload_core_module(HWND window);
void load_core_module();
int register_and_create_main_window(DWORD thread_id, int nCmdShow);

namespace fs = std::filesystem;

struct embedded_dll
{
  std::filesystem::path filename;
  HGLOBAL handle;
};

struct update_context
{
  std::atomic_bool new_version_is_available = false;
  BOOL update_in_progress = false;
  SIZE available_version = {};
  std::uint32_t update_stable_id = ::RegisterWindowMessageW(L"COMMAND_UPDATE_STABLE");
  std::uint32_t update_development_id = ::RegisterWindowMessageW(L"COMMAND_UPDATE_DEVELOPMENT");
  bool has_console = false;
  std::atomic_size_t max_size = 0;
  std::atomic_size_t current_size = 0;
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

extern "C" {
__declspec(dllexport) void detect_update(std::uint32_t update_type)
{
  if (!(update_type == context.update_development_id || update_type == context.update_stable_id))
  {
    return;
  }

  win32::queue_user_work_item([update_type]() {
    if (!context.has_console)
    {
      context.has_console = ::GetConsoleWindow() != nullptr;
    }
    alloc_console();

    auto temp_file = fs::temp_directory_path() / "latest-siege-studio-version.txt";
    auto channel = update_type == context.update_development_id ? "development" : "stable";
    std::string server = resolve_url() + "/" + channel + "/latest.txt";

    std::error_code last_error;

    context.max_size = fs::file_size(win32::module_ref::current_application().GetModuleFileName(), last_error);

#if _DEBUG
    context.max_size = context.max_size * 10;
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

        // TODO use siege-studio-core so that the version can be dynamically updated
        if (major >= SIEGE_MAJOR_VERSION && minor > SIEGE_MINOR_VERSION)
        {
          context.new_version_is_available = true;
          context.available_version.cx = major;
          context.available_version.cy = minor;
        }
      }
    }
  });
}

__declspec(dllexport) BOOL can_update()
{
  return TRUE;
}

__declspec(dllexport) BOOL has_update()
{
  return context.new_version_is_available == true ? TRUE : FALSE;
}

__declspec(dllexport) SIZE get_update_version()
{
  return context.available_version;
}

__declspec(dllexport) std::size_t get_max_update_size()
{
  return context.max_size;
}

__declspec(dllexport) std::size_t get_current_update_size()
{
  return context.current_size;
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

  if (!context.new_version_is_available)
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
    std::stringstream final_url;
    auto channel = update_type == context.update_development_id ? "development" : "stable";
    final_url << resolve_url() << "/" << channel << "/" << context.available_version.cx << "." << context.available_version.cy << "/" << "siege-studio.exe";
    std::error_code last_error;

    auto value = std::to_string(context.available_version.cx) + "." + std::to_string(context.available_version.cy);

    auto temp_folder = fs::temp_directory_path() / L"siege-studio" / value;

    fs::create_directories(temp_folder, last_error);
    auto temp_file = temp_folder / "siege-studio.exe";
    fs::remove(temp_file, last_error);

    auto task = std::async(std::launch::async, [temp_file]() {
      using namespace std::chrono_literals;

      std::error_code last_error;
      while (true)
      {
        std::shared_ptr<void> deferred{ nullptr, [](...) { std::this_thread::sleep_for(500ms); } };

        if (!fs::exists(temp_file, last_error))
        {
          continue;
        }

        context.current_size = fs::file_size(temp_file, last_error);

        if (last_error)
        {
          break;
        }

        if (context.current_size >= context.max_size)
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

    ::EnumResourceNamesW(module, RT_RCDATA, extract_embedded_dlls, (LONG_PTR)&embedded_dlls);

    auto window_thread_id = ::GetWindowThreadProcessId(window, nullptr);

    if (!embedded_dlls.empty())
    {
      ::EnableWindow(window, TRUE);
      unload_core_module(window);
    }

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

      auto copy_and_patch = [&]() {
        fs::copy_file(temp_folder / filename_lower, filename_lower, fs::copy_options::update_existing, last_error);

        LOADED_IMAGE image{};
        if (::MapAndLoad(siege::platform::to_lower(dll.filename.string()).c_str(), nullptr, &image, TRUE, FALSE))
        {
          image.FileHeader->OptionalHeader.MajorImageVersion = context.available_version.cx;
          image.FileHeader->OptionalHeader.MinorImageVersion = context.available_version.cy;
          ::UnMapAndLoad(&image);
        }
      };

      if (!fs::exists(filename_lower, last_error))
      {
        copy_and_patch();
        return;
      }

      LOADED_IMAGE image{};
      if (::MapAndLoad(siege::platform::to_lower(dll.filename.string()).c_str(), nullptr, &image, TRUE, TRUE))
      {
        auto existing_major_version = image.FileHeader->OptionalHeader.MajorImageVersion;
        auto existing_minor_version = image.FileHeader->OptionalHeader.MajorImageVersion;
        ::UnMapAndLoad(&image);

        if (context.available_version.cx >= existing_major_version && context.available_version.cy > existing_minor_version)
        {
          copy_and_patch();
        }
      }
    });
    ::FreeLibrary(module);

    load_core_module();
    register_and_create_main_window(window_thread_id, SW_SHOWNORMAL);
  });
}
}