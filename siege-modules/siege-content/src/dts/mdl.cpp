#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::mdl
{
  namespace endian = siege::platform;

  bool is_mdl(std::istream& stream)
  {
    return false;
  }

  bool is_md2(std::istream& stream)
  {
    return false;
  }

}// namespace siege::content::mdl
// TODO complete MDL + MD2 parsers