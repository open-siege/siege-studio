#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "configurations/dosbox.hpp"

namespace dosbox = siege::configuration::dosbox;

using namespace std::literals;

TEST_CASE("Parsing of an dosbox mapper config file", "[cfg.dosbox.mapper]")
{
  SECTION("With sample mapper data, values convert correctly")
  {
    std::stringstream raw_config;

    raw_config << "hand_shutdown \"key 290 mod1\"" << "\r\n";
    raw_config << "key_a \"key 97\"" << "\r\n";
    raw_config << "key_s \"key 115\"" << "\r\n";
    raw_config << "key_d \"key 100\"" << "\r\n";
    raw_config << "key_f \"key 102\"" << "\r\n";
    raw_config << "j_button_0_0 \"stick_0 axis 2 0\"" << "\r\n";
    raw_config << "j_button_0_1 \"stick_0 axis 2 1\"" << "\r\n";
    raw_config << "jaxis_0_1-" << "\r\n";
    raw_config << "jaxis_0_1+" << "\r\n";
    raw_config << "mod_1 \"key 305\" \"key 306\"" << "\r\n";
    raw_config << "mod_2 \"key 307\" \"key 308\"" << "\r\n";
    raw_config << "mod_3" << "\r\n";

    auto value = dosbox::mapper::load_config(raw_config, raw_config.tellp());

    REQUIRE(value.has_value() == true);

    REQUIRE(value.value().keys().size() == 12);

    REQUIRE(value.value().find("hand_shutdown"sv) == "key 290 mod1");
  //  REQUIRE(value.value().find("key_a"sv) == "key 97");
  //  REQUIRE(value.value().find("key_s"sv) == "key 115");
 //   REQUIRE(value.value().find("key_d"sv) == "key 100");
 //   REQUIRE(value.value().find("key_f"sv) == "key 102");
 //   REQUIRE(value.value().find("j_button_0_0"sv) == "stick_0 axis 2 0");
  //  REQUIRE(value.value().find("j_button_0_1"sv) == "stick_0 axis 2 1");
  //  REQUIRE(value.value().find("jaxis_0_1-"sv) == "");
  //  REQUIRE(value.value().find("jaxis_0_1+"sv) == "");
  //  REQUIRE(value.value().find("mod_1"sv).at(0) == "key 305");
  //  REQUIRE(value.value().find("mod_1"sv).at(1) == "key 306");
  //  REQUIRE(value.value().find("mod_2"sv) == "\"key 307\" \"key 308\"");
  //  REQUIRE(value.value().find("mod_2"sv).at(0) == "key 307");
  //  REQUIRE(value.value().find("mod_2"sv).at(1) == "key 308");
  //  REQUIRE(value.value().find("mod_3"sv) == "");
  }
}
