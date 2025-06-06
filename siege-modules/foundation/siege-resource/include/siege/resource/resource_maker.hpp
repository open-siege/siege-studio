#ifndef RESOURCE_MAKER_HPP
#define RESOURCE_MAKER_HPP

#include <memory>
#include <istream>
#include <siege/platform/resource.hpp>

namespace siege::resource
{
  bool is_resource_reader(std::istream&);
  std::unique_ptr<siege::platform::resource_reader> make_resource_reader(std::istream&);
}


#endif// !RESOURCE_MAKER_HPP
