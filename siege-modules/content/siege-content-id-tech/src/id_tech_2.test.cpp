#include <catch2/catch_test_macros.hpp>
import std;
import siege.configuration.id_tech;

namespace id_tech = siege::configuration::id_tech;

using namespace std::literals;

TEST_CASE("When data is empty for id Tech 2.5 nullopt returned")
{
  std::stringstream data;
  auto value = id_tech::id_tech_2::load_config(data, 0);

  REQUIRE(value == std::nullopt);
}

TEST_CASE("With sample id Tech 2.5 line, with single key and value, values convert correctly")
{
  std::stringstream raw_config("keya value");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find("keya"sv) == "value");
}

TEST_CASE("With sample id Tech 2.5 line, with single quoted key and value, values convert correctly")
{
  std::stringstream raw_config("\"keya\" value");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find("keya"sv) == "value");
}

TEST_CASE("With sample id Tech 2.5 line, with single key and quoted value, values convert correctly")
{
  std::stringstream raw_config("keya \"value\"");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find("keya"sv) == "value");
}

TEST_CASE("With sample id Tech 2.5 line, with single quoted key and quoted value,  values convert correctly")
{
  std::stringstream raw_config("\"keya\" \"value\"");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find("keya"sv) == "value");
}

TEST_CASE("With sample id Tech 2.5 data, with multi part quoted key and value, values convert correctly")
{
  std::stringstream raw_config("\"multi key\" value");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find("multi key"sv) == "value");
}

TEST_CASE("With sample id Tech 2.5 data, with multi part quoted key and multi part quoted value, values convert correctly")
{
  std::stringstream raw_config("\"multi key\" \"multi value\"");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find("multi key"sv) == "multi value");
}

TEST_CASE("With sample id Tech 2.5 three word line, with single key and value, values convert correctly")
{
  std::stringstream raw_config("keya keyb value");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "keya", "keyb" }) == "value");
}

TEST_CASE("With sample id Tech 2.5 three word line, with single quoted key, single unquoted and unquoted value, values convert correctly")
{
  std::stringstream raw_config("\"keya\" keyb value");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "keya", "keyb" }) == "value");
}

TEST_CASE("With sample id Tech 2.5 three word line, with single unquoted key, single quoted key and unquoted value, values convert correctly")
{
  std::stringstream raw_config("keya \"keyb\" value");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "keya", "keyb" }) == "value");
}

TEST_CASE("With sample id Tech 2.5 three word line, with single key and quoted value, values convert correctly")
{
  std::stringstream raw_config("keya keyb \"value\"");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "keya", "keyb" }) == "value");
}

TEST_CASE("With sample id Tech 2.5 three word line, all key and values quoted, values convert correctly")
{
  std::stringstream raw_config("\"keya\" \"keyb\" \"value\"");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.value().find({ "keya", "keyb" }) == "value");
}

TEST_CASE("With sample id Tech 2.5 data, values convert correctly")
{
  std::stringstream raw_config;
  raw_config << "unbindall"
             << "\r\n";

  raw_config << "bind MOUSE1 \"+attack\""
             << "\r\n";
  raw_config << "bind MOUSE2 \"+altattack\""
             << "\r\n";
  raw_config << "bind W \"+forward\""
             << "\r\n";
  raw_config << "bind A \"+moveleft\""
             << "\r\n";
  raw_config << "bind S \"+backward\""
             << "\r\n";
  raw_config << "bind D \"+moveright\""
             << "\r\n";

  raw_config << "set log_file_name \"\""
             << "\r\n";
  raw_config << "set bestweap \"safe\""
             << "\r\n";
  raw_config << "set name \"Mohn Jullins\""
             << "\r\n";

  raw_config << "set net_socksPort \"1000\""
             << "\r\n";
  raw_config << "set ai_maxcorpses \"4\""
             << "\r\n";
  raw_config << "set gl_swapinterval \"1\""
             << "\r\n";

  raw_config << "set gl_offsetunits \"-2.0\""
             << "\r\n";
  raw_config << "set ghl_shadow_darkness \"0.75\""
             << "\r\n";
  raw_config << "set vid_brightness \"0.600000\""
             << "\r\n";

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.tellp());

  REQUIRE(value.has_value() == true);

  REQUIRE(value.value().keys().size() == 16);

  REQUIRE(value.value().find("unbindall"sv) == "");

  REQUIRE(value.value().find({ "bind", "MOUSE1" }) == "+attack");
  REQUIRE(value.value().find({ "bind", "MOUSE2" }) == "+altattack");
  REQUIRE(value.value().find({ "bind", "W" }) == "+forward");
  REQUIRE(value.value().find({ "bind", "A" }) == "+moveleft");
  REQUIRE(value.value().find({ "bind", "S" }) == "+backward");
  REQUIRE(value.value().find({ "bind", "D" }) == "+moveright");

  REQUIRE(value.value().find({ "set", "log_file_name" }) == "");
  REQUIRE(value.value().find({ "set", "bestweap" }) == "safe");
  REQUIRE(value.value().find({ "set", "name" }) == "Mohn Jullins");

  REQUIRE(value.value().find({ "set", "net_socksPort" }) == "1000");
  REQUIRE(value.value().find({ "set", "ai_maxcorpses" }) == "4");
  REQUIRE(value.value().find({ "set", "gl_swapinterval" }) == "1");

  REQUIRE(value.value().find({ "set", "gl_offsetunits" }) == "-2.0");
  REQUIRE(value.value().find({ "set", "ghl_shadow_darkness" }) == "0.75");
  REQUIRE(value.value().find({ "set", "vid_brightness" }) == "0.600000");
}

TEST_CASE("Bare unbindall keeps final character and saves without empty quotes")
{
  std::stringstream raw_config("unbindall\r\n");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.has_value());
  REQUIRE(value.value().contains("unbindall"sv));
  REQUIRE(value.value().find("unbindall"sv) == "");

  std::stringstream saved;
  value.value().save(saved);

  REQUIRE(saved.str() == "unbindall\r\n");
}

TEST_CASE("Comment lines are saved using the original raw line")
{
  std::stringstream raw_config("// generated by Soldier of Fortune, do not modify\r\n");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.has_value());

  std::stringstream saved;
  value.value().save(saved);

  REQUIRE(saved.str() == "// generated by Soldier of Fortune, do not modify\r\n");
}

TEST_CASE("Quoted empty cvar values round-trip with empty quotes")
{
  std::stringstream raw_config("set log_file_name \"\"\r\n");

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.has_value());
  REQUIRE(value.value().contains(std::vector{ "set"sv, "log_file_name"sv }));
  REQUIRE(value.value().find({ "set", "log_file_name" }) == "");

  std::stringstream saved;
  value.value().save(saved);

  REQUIRE(saved.str() == "set log_file_name \"\"\r\n");
}

TEST_CASE("Soldier of Fortune style header and unbindall round-trip")
{
  std::stringstream raw_config;
  raw_config << "// generated by Soldier of Fortune, do not modify\r\n";
  raw_config << "\r\n";
  raw_config << "unbindall\r\n";
  raw_config << "bind MOUSE1 \"+attack\"\r\n";

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());

  REQUIRE(value.has_value());
  REQUIRE(value.value().find("unbindall"sv) == "");
  REQUIRE(value.value().find({ "bind", "MOUSE1" }) == "+attack");

  std::stringstream saved;
  value.value().save(saved);

  REQUIRE(saved.str() == "// generated by Soldier of Fortune, do not modify\r\nunbindall\r\nbind MOUSE1 +attack\r\n");
}

TEST_CASE("Saving more than 4096 bytes through ofstream keeps the full file")
{
  std::stringstream raw_config;
  raw_config << "// generated by Soldier of Fortune, do not modify\r\n";
  raw_config << "unbindall\r\n";

  for (int i = 0; i < 250; ++i)
  {
    raw_config << "set cvar_" << i << " \"" << i << "\"\r\n";
  }

  raw_config << "set snd_dll \"defsnd.dll\"\r\n";

  REQUIRE(raw_config.str().size() > 4096);

  auto value = id_tech::id_tech_2::load_config(raw_config, raw_config.str().size());
  REQUIRE(value.has_value());

  auto path = std::filesystem::temp_directory_path() / "siege_id_tech_2_ofstream.cfg";
  {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    REQUIRE(out);
    value.value().save(out);
  }

  auto saved_size = std::filesystem::file_size(path);
  REQUIRE(saved_size > 4096);

  std::string saved;
  {
    std::ifstream in(path, std::ios::binary);
    std::stringstream verify;
    verify << in.rdbuf();
    saved = verify.str();
  }

  REQUIRE(saved.find("unbindall") != std::string::npos);
  REQUIRE(saved.find("set snd_dll \"defsnd.dll\"") != std::string::npos);
  REQUIRE(saved.back() == '\n');

  std::filesystem::remove(path);
}