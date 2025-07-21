#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <siege/configuration/3space.hpp>

namespace three_space = siege::configuration::three_space;

using namespace std::literals;

TEST_CASE("3space: When data is empty nullopt returned")
{
  std::stringstream data;
  auto value = three_space::three_space_3::load_config(data, 0);

  REQUIRE(value == std::nullopt);
}

TEST_CASE("3space: With comment, config is returned with no keys")
{
  std::stringstream raw_config("# some comment");

  auto value = three_space::three_space_3::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().keys().empty() == true);
}

TEST_CASE("3space: With comment with whitespace, config is returned with no keys")
{
  std::stringstream raw_config(" # some comment");

  auto value = three_space::three_space_3::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().keys().empty() == true);
}

TEST_CASE("3space: With function call and constant value, turned into a key with a single value")
{
  std::stringstream raw_config("newActionMap(Herc);");

  auto value = three_space::three_space_3::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().contains({ "newActionMap", "Herc" }));
}

TEST_CASE("3space: With function call and constant value with spacing, turned into a key with a single value")
{
  std::stringstream raw_config("newActionMap( Herc );");

  auto value = three_space::three_space_3::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().contains({ "newActionMap", "Herc" }));
}

TEST_CASE("3space: With bind function, splits keys and values")
{
  std::stringstream raw_config("bindaction(keyboard,make,space,TO,IDACTION_FIRE,1);");

  auto value = three_space::three_space_3::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "bindaction", "keyboard", "make", "space", "TO" }) == siege::configuration::key_type({ "IDACTION_FIRE", "1" }));
}

TEST_CASE("3space: With bind function with spacing, splits keys and values")
{
  std::stringstream raw_config("bindaction( keyboard, make,  space, TO, IDACTION_FIRE, 1 );");

  auto value = three_space::three_space_3::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "bindaction", "keyboard", "make", "space", "TO" }) == siege::configuration::key_type({ "IDACTION_FIRE", "1" }));
}

TEST_CASE("3space: With multiple function calls per line, each are parsed correct")
{
  std::stringstream raw_config("exec(\"other.cs\");\r\nnewActionMap(Herc);\r\nbindaction( keyboard, make,  space, TO, IDACTION_FIRE, 1 );");

  auto value = three_space::three_space_3::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().contains({ "exec", "\"other.cs\"" }));
  REQUIRE(value.value().contains({ "newActionMap", "Herc" }));
  REQUIRE(value.value().find({ "bindaction", "keyboard", "make", "space", "TO" }) == siege::configuration::key_type({ "IDACTION_FIRE", "1" }));
}

TEST_CASE("3space: With multiple function calls on one line, each are parsed correctly")
{
  std::stringstream raw_config("exec(\"other.cs\");newActionMap(Herc);bindaction( keyboard, make,  space, TO, IDACTION_FIRE, 1 );");

  auto value = three_space::three_space_3::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().contains({ "exec", "\"other.cs\"" }));
  REQUIRE(value.value().contains({ "newActionMap", "Herc" }));
  REQUIRE(value.value().find({ "bindaction", "keyboard", "make", "space", "TO" }) == siege::configuration::key_type({ "IDACTION_FIRE", "1" }));
}