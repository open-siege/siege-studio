#ifndef OPEN_SIEGE_BMP_SHARED_HPP
#define OPEN_SIEGE_BMP_SHARED_HPP

#include <vector>
#include <string_view>

namespace studio::views
{
  inline std::vector<std::string_view> get_bmp_extensions()
  {
    return { ".bmp", ".dib", ".pba", ".dbm", ".dba", ".db0", ".db1", ".db2", ".hba", ".hb0", ".hb1", ".hb2" };
  }

}

#endif// OPEN_SIEGE_BMP_SHARED_HPP
