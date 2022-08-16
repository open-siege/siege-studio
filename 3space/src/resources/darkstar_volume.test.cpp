#include <catch2/catch.hpp>
#include <sstream>
#include "resources/darkstar_volume.hpp"
#include "shared.hpp"

namespace darkstar = studio::resources::vol::darkstar;

TEST_CASE("With one text file, creates a Darkstar Volume file with the correct byte layout", "[vol.darkstar]")
{
  SECTION("When data is 4 byte aligned, no padding is added.")
  {
    std::stringstream mem_buffer;

    std::vector<darkstar::volume_file_info> files;
    auto memory_file_info = new std::stringstream();

    (*memory_file_info) << "Hello Darkness, my old friend...";
    files.emplace_back(darkstar::volume_file_info{ "hello.txt", 32 , std::nullopt, darkstar::compression_type::none, std::unique_ptr<std::istream>(memory_file_info) });

    darkstar::create_vol_file(mem_buffer, files);

    REQUIRE(studio::resources::vol::darkstar::vol_file_archive::is_supported(mem_buffer) == true);

    mem_buffer.seekg(4, std::ios::beg);

    std::array<std::byte, 4> temp = {};

    constexpr auto little_endian_48_in_hex = std::array<std::byte, 4>{ std::byte(0x30), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_48_in_hex);

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == studio::shared::to_tag<4>({ 'V', 'B', 'L', 'K' }));

    constexpr auto little_endian_32_in_hex_with_end_byte_tag = std::array<std::byte, 4>{ std::byte(0x20), std::byte(0x00), std::byte(0x00), std::byte(0x80) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_32_in_hex_with_end_byte_tag);

    std::string content(32, '\0');
    mem_buffer.read(reinterpret_cast<char*>(content.data()), content.size());
    REQUIRE(content == std::string_view{ "Hello Darkness, my old friend..." });

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == studio::shared::to_tag<4>({ 'v', 'o', 'l', 's' }));

    constexpr auto little_endian_10_in_hex = std::array<std::byte, 4>{ std::byte(0x0A), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_10_in_hex);

    std::array<char, 10> filename = {};
    mem_buffer.read(reinterpret_cast<char*>(filename.data()), filename.size());
    REQUIRE(filename == std::array<char, 10>{ "hello.txt" });

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == studio::shared::to_tag<4>({ 'v', 'o', 'l', 'i' }));

    constexpr auto little_endian_17_in_hex = std::array<std::byte, 4>{ std::byte(0x11), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_17_in_hex);

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == std::array<std::byte, 4>());

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == std::array<std::byte, 4>());

    constexpr auto little_endian_8_in_hex = std::array<std::byte, 4>{ std::byte(0x08), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_8_in_hex);

    constexpr auto little_endian_32_in_hex = std::array<std::byte, 4>{ std::byte(0x20), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_32_in_hex);

    std::byte file_type{};
    mem_buffer.read(&file_type, 1);
    REQUIRE(file_type == std::byte{ 0x00 });
  }

  SECTION("When is data not is 4 byte aligned, padding is added.")
  {
    std::stringstream mem_buffer;

    std::vector<darkstar::volume_file_info> files;
    auto memory_file_info = new std::stringstream();

    (*memory_file_info) << "I've come to talk with you again..";
    files.emplace_back(darkstar::volume_file_info{ "hello.txt", 34, std::nullopt, darkstar::compression_type::none, std::unique_ptr<std::istream>(memory_file_info) });

    darkstar::create_vol_file(mem_buffer, files);

    REQUIRE(studio::resources::vol::darkstar::vol_file_archive::is_supported(mem_buffer) == true);

    mem_buffer.seekg(4, std::ios::beg);

    std::array<std::byte, 4> temp = {};

    constexpr auto little_endian_52_in_hex = std::array<std::byte, 4>{ std::byte(0x34), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_52_in_hex);

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == studio::shared::to_tag<4>({ 'V', 'B', 'L', 'K' }));

    constexpr auto little_endian_34_in_hex_with_end_byte_tag = std::array<std::byte, 4>{ std::byte(0x22), std::byte(0x00), std::byte(0x00), std::byte(0x80) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_34_in_hex_with_end_byte_tag);

    std::string content(34, '\0');
    mem_buffer.read(reinterpret_cast<char*>(content.data()), content.size());
    REQUIRE(content == std::string_view{ "I've come to talk with you again.." });

    mem_buffer.read(temp.data(), 2);
    REQUIRE(temp[0] == std::byte(0x00));
    REQUIRE(temp[1] == std::byte(0x00));

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == studio::shared::to_tag<4>({ 'v', 'o', 'l', 's' }));

    constexpr auto little_endian_10_in_hex = std::array<std::byte, 4>{ std::byte(0x0A), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_10_in_hex);

    std::array<char, 10> filename = {};
    mem_buffer.read(reinterpret_cast<char*>(filename.data()), filename.size());
    REQUIRE(filename == std::array<char, 10>{ "hello.txt" });

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == studio::shared::to_tag<4>({ 'v', 'o', 'l', 'i' }));

    constexpr auto little_endian_17_in_hex = std::array<std::byte, 4>{ std::byte(0x11), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_17_in_hex);

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == std::array<std::byte, 4>());

    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == std::array<std::byte, 4>());

    constexpr auto little_endian_8_in_hex = std::array<std::byte, 4>{ std::byte(0x08), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_8_in_hex);

    constexpr auto little_endian_32_in_hex = std::array<std::byte, 4>{ std::byte(0x22), std::byte(0x00), std::byte(0x00), std::byte(0x00) };
    mem_buffer.read(temp.data(), temp.size());
    REQUIRE(temp == little_endian_32_in_hex);

    std::byte file_type{};
    mem_buffer.read(&file_type, 1);
    REQUIRE(file_type == std::byte{ 0x00 });
  }

  SECTION("When multiple files are present, the file info can be parsed correctly.")
  {
    std::stringstream mem_buffer;

    std::vector<darkstar::volume_file_info> files;
    auto memory_file_info = new std::stringstream();

    (*memory_file_info) << "Hello Darkness, my old friend...";
    files.emplace_back(darkstar::volume_file_info{ "hello.txt", 32, std::nullopt, darkstar::compression_type::none, std::unique_ptr<std::istream>(memory_file_info) });

    auto memory_file_info2 = new std::stringstream();

    (*memory_file_info2) << "Hey, hey, hey";
    files.emplace_back(darkstar::volume_file_info{ "test.txt", 13, std::nullopt, darkstar::compression_type::lzh, std::unique_ptr<std::istream>(memory_file_info2) });


    auto memory_file_info3 = new std::stringstream();

    (*memory_file_info3) << "Beep, beep, beep";
    files.emplace_back(darkstar::volume_file_info{ "beep.txt", 16, std::nullopt, darkstar::compression_type::none, std::unique_ptr<std::istream>(memory_file_info3) });

    darkstar::create_vol_file(mem_buffer, files);

    darkstar::vol_file_archive archive;

    auto parsed_files = archive.get_content_listing(mem_buffer, { std::filesystem::path(), std::filesystem::path() });
    REQUIRE(parsed_files.size() == 3);

    std::visit([&](auto& info) {
      using info_type = std::decay_t<decltype(info)>;
      if constexpr (std::is_same_v<info_type, studio::resources::file_info>)
      {
        REQUIRE(info.size == 32);
        REQUIRE(info.offset == 8);
        REQUIRE(info.filename == "hello.txt");
        REQUIRE(info.compression_type == studio::resources::compression_type::none);
      }
    },
      parsed_files.at(0));

    std::visit([&](auto& info) {
      using info_type = std::decay_t<decltype(info)>;
      if constexpr (std::is_same_v<info_type, studio::resources::file_info>)
      {
        REQUIRE(info.size == 13);
        REQUIRE(info.offset == 48);
        REQUIRE(info.filename == "test.txt");
        REQUIRE(info.compression_type == studio::resources::compression_type::lzh);
      }
    },
      parsed_files.at(1));

    std::visit([&](auto& info) {
      using info_type = std::decay_t<decltype(info)>;
      if constexpr (std::is_same_v<info_type, studio::resources::file_info>)
      {
        REQUIRE(info.size == 16);
        REQUIRE(info.offset == 72);
        REQUIRE(info.filename == "beep.txt");
        REQUIRE(info.compression_type == studio::resources::compression_type::none);
      }
    },
      parsed_files.at(2));
  }
}
