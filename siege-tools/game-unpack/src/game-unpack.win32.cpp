#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
#include <siege/platform/installation_module.hpp>
#include <siege/platform/extension_module.hpp>
#include <span>
#include <string_view>
#include <array>
#include <ranges>
#include <filesystem>

namespace fs = std::filesystem;

bool is_standalone_console()
{
  std::array<DWORD, 4> ids{};

  return GetConsoleProcessList(ids.data(), (DWORD)ids.size()) == 1;
}

int main(int argc, const char* argv[])
{
  auto args = std::span(argv, argc) | std::views::transform([](char const* v) { return std::string_view(v); });

  auto installation_modules = siege::platform::installation_module::load_modules(fs::current_path());

  if (args.size() == 1 && is_standalone_console())
  {
    ::ShowWindow(::GetConsoleWindow(), SWP_HIDEWINDOW);
    // TODO create task dialog window
  }




  return 0;
}