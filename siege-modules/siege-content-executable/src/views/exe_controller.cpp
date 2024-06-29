#include <vector>
#include <istream>
#include "exe_controller.hpp"

namespace siege::views
{
  bool exe_controller::is_exe(std::istream& stream)
  {
    auto position = stream.tellg();

    thread_local std::vector<char> data(1024, '\0');

    stream.read(data.data(), data.size());
    stream.seekg(position, std::ios::beg);

    if (data[0] == 'M' && data[1] == 'Z')
    {
      auto e_iter = std::find(data.rbegin(), data.rend(), 'E');
      auto p_iter = std::find(e_iter, data.rend(), 'P');


      return e_iter != data.rend() && p_iter != data.rend() && std::distance(e_iter, p_iter) == 1;
    }

    return false;
  }

  std::size_t exe_controller::load_executable(std::istream& image_stream, std::optional<std::filesystem::path> path) noexcept
  {
    if (path)
    {
      loaded_module.reset(::LoadLibraryExW(path->c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE));
      return loaded_module ? 1 : 0;
    }

    return 0;
  }
}// namespace siege::views