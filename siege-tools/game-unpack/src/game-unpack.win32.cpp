#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
#include <siege/platform/installation_module.hpp>
#include <siege/platform/extension_module.hpp>
#include <span>
#include <string_view>
#include <ranges>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, const char* argv[])
{
  auto args = std::span(argv, argc) | std::views::transform([](char const* v) { return std::string_view(v); });

  

  auto installation_modules = siege::platform::installation_module::load_modules(fs::current_path());

  return 0;
}