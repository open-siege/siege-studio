#include <nlohmann/json.hpp>
#include <istream>
#include "pal_settings.hpp"

using json = nlohmann::json;

namespace siege::content::pal
{
  bool is_pal_settings_file(std::istream& raw_data)
  {
      try
      {
        auto pos = raw_data.tellg();

        std::istream temp(reinterpret_cast<std::streambuf*>(raw_data.rdbuf()));

        json data;
        temp >> data;

        raw_data.seekg(pos, std::ios::beg);

        return data.type() == json::value_t::object;
      }
      catch (...)
      {
        return false;
      }
  }
}
