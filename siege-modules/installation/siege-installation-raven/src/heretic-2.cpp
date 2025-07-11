#include <array>
#include <siege/platform/installation_module.hpp>
#include <siege/platform/win/file.hpp>
#include <filesystem>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_rule = platform::path_rule;
namespace fs = std::filesystem;

extern "C" {
extern std::array<const fs_char*, 3> name_variations = {
  FSL "Heretic 2",
  FSL "Heretic II"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 1,
  .disc_names = { FSL "HERETIC_II" },
  .default_install_path = FSL "<systemDrive>/Games/Heretic II"
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL "siege-extension-heretic-2"
};

extern std::array<path_rule, 32> directory_mappings = { {
  { FSL "setup/data1.cab/*", FSL "." },
  { FSL "setup/Base/VIDEO/*.smk", FSL "Base/VIDEO" },
  { FSL "setup/Toolkit/data1.cab/*", FSL "Toolkit" },
  { FSL "setup/setup.bmp", FSL "=", path_rule::optional },
} };

std::errc apply_post_install_steps() noexcept
{
  std::error_code last_error;

  if (!fs::exists("ref_gl.dll"))
  {
    return {};
  }

  win32::file gl_dll("ref_gl.dll", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, std::nullopt, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);
  auto view = gl_dll.CreateFileMapping(std::nullopt, PAGE_READWRITE, {}, L"");

  if (!view)
  {
    return {};
  }

  std::size_t size = (std::size_t)gl_dll.GetFileSizeEx().value_or(LARGE_INTEGER{}).QuadPart;
  auto mapping = view->MapViewOfFile(FILE_MAP_WRITE, size);

  if (!mapping.get())
  {
    return {};
  }

  std::string_view raw_data((char*)mapping.get(), size);

  auto index = raw_data.find("GL_EXTENSIONS: %s");

  if (index != std::string_view::npos)
  {
    std::memcpy((char*)mapping.get() + index, "GL_EXTENSIONS: no", 18);
  }

  return {};
}
}