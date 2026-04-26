#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/configuration/xml.hpp>

#include <siege/platform/win/com.hpp>
namespace xml = siege::configuration::common::xml;

using namespace std::literals;

#if WIN32
TEST_CASE("When data is empty for xml nullopt returned")
{
  std::stringstream data;
  auto value = xml::load_config(data, 0);

  REQUIRE(value == std::nullopt);
}

TEST_CASE("load_config returns nested element text value")
{
  win32::com::init_com();
  std::stringstream data{
    "<test><value>Some special value</value></test>"
  };
  auto value = xml::load_config(data, 46);

  REQUIRE(value.value().find({ "test"sv, "value"sv }) == "Some special value");
}

TEST_CASE("entries_for returns bind attributes from KeyMapping xml")
{
  win32::com::init_com();
  std::stringstream data{
    "<KeyMapping>\n"
    "<ActionKeys>\n"
    "<bind action = \"use_item\" key1 = \"key_mouse1\" key2 = \"\" />\n"
    "<bind action = \"run\" key1 = \"key_mouse2\" key2 = \"\" />\n"
    "</ActionKeys>\n"
    "</KeyMapping>\n"
  };
  auto value = xml::load_config(data, 167);

  auto entries = value.value().entries_for({ "KeyMapping"sv, "ActionKeys"sv, "bind"sv, "@"sv, "action"sv });

  REQUIRE(entries.at(0).value == "use_item");
  REQUIRE(entries.at(0).raw_line == "<bind action = \"use_item\" key1 = \"key_mouse1\" key2 = \"\" />");

  REQUIRE(entries.at(1).value == "run");
  

  entries = value.value().entries_for({ "KeyMapping"sv, "ActionKeys"sv, "bind"sv, "@"sv, "key1"sv });
  REQUIRE(entries.at(0).value == "key_mouse1");
  REQUIRE(entries.at(0).raw_line == "<bind action = \"use_item\" key1 = \"key_mouse1\" key2 = \"\" />");
  REQUIRE(entries.at(1).value == "key_mouse2");


  entries = value.value().entries_for({ "KeyMapping"sv, "ActionKeys"sv, "bind"sv, "@"sv, "key2"sv });
  REQUIRE(entries.at(0).value == "");
  REQUIRE(entries.at(0).raw_line == "<bind action = \"use_item\" key1 = \"key_mouse1\" key2 = \"\" />");
  REQUIRE(entries.at(1).value == "");
}
#endif