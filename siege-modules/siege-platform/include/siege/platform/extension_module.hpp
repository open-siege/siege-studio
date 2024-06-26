#ifndef SIEGE_EXTENSION_MODULE_HPP
#define SIEGE_EXTENSION_MODULE_HPP

#include <optional>
#include <filesystem>
#include <siege/platform/win/core/module.hpp>
#include <siege/platform/win/core/com/collection.hpp>

namespace siege
{
  // TODO cross platform versions of this code will use "xcom" (cross com) and char16_t instead of wchar_t
  using ExecutableIsSupported = HRESULT __stdcall(const wchar_t* filename) noexcept;

#if WIN32
  using LaunchGameWithExtension = HRESULT __stdcall(const wchar_t* exe_path_str, std::uint32_t argc, const wchar_t** argv, PROCESS_INFORMATION*) noexcept;
  using GetGameScriptHost = HRESULT __stdcall(const wchar_t* game, ::IDispatch** host) noexcept;
#endif

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class game_extension_module : public win32::module
  {
    using base = win32::module;
    ExecutableIsSupported* ExecutableIsSupportedProc = nullptr;

  public:

#if WIN32
    GetGameScriptHost* GetGameScriptHost = nullptr;
    LaunchGameWithExtension* LaunchGameWithExtension = nullptr;
#endif

    game_extension_module(std::filesystem::path module_path) : base(module_path)
    {
      // In theory, it's optional, because dlls may be injected that don't come from this project.
      // In practice, all siege extension modules should have this.
      ExecutableIsSupportedProc = GetProcAddress<decltype(ExecutableIsSupportedProc)>("ExecutableIsSupported");

      // These functions are very Windows specific because the games being launched would all be Windows-based.
#if WIN32
      this->GetGameScriptHost = GetProcAddress<decltype(game_extension_module::GetGameScriptHost)>("GetGameScriptHost");
      this->LaunchGameWithExtension = GetProcAddress<decltype(game_extension_module::LaunchGameWithExtension)>("LaunchGameWithExtension");
#endif
      if (!(this->GetGameScriptHost || this->LaunchGameWithExtension))
      {
        throw std::runtime_error("Could not find module functions");
      }
    }

    std::optional<bool> ExecutableIsSupported(std::filesystem::path exe_path)
    {
      if (ExecutableIsSupportedProc)
      {
        return ExecutableIsSupportedProc(exe_path.c_str()) == S_OK;
      }

      return std::nullopt;
    }
  };


}// namespace siege

#endif// !SIEGE_EXTENSION_MODULE_HPP
