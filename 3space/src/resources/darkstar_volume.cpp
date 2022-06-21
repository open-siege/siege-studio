#include <fstream>
#include <map>
#include <filesystem>
#include <vector>
#include <array>
#include <utility>
#include <string>
#include <cstdlib>
#include "resources/darkstar_volume.hpp"

namespace studio::resources::vol::darkstar
{
  namespace endian = boost::endian;
  using namespace std::literals;

  using file_tag = std::array<std::byte, 4>;

  // TODO add some checks for these items
  constexpr auto vol_index_tag = shared::to_tag<4>({ 'v', 'o', 'l', 'i' });
  constexpr auto vol_string_tag = shared::to_tag<4>({ 'v', 'o', 'l', 's' });
  constexpr auto vol_block_tag = "vblk"sv;

  constexpr auto vol_file_tag = shared::to_tag<4>({ ' ', 'V', 'O', 'L' });
  constexpr auto alt_vol_file_tag = shared::to_tag<4>({ 'P', 'V', 'O', 'L' });
  constexpr auto old_vol_file_tag = shared::to_tag<4>({ 'V', 'O', 'L', ' ' });

  constexpr auto block_tag = shared::to_tag<4>({ 'V', 'B', 'L', 'K' });

  enum class volume_version
  {
    three_space_vol,
    darkstar_pvol,
    darkstar_vol
  };

  struct file_info
  {
    std::string filename;
    std::uint32_t offset;
    std::uint32_t size;
    compression_type compression_type;
  };

  struct volume_header
  {
    std::array<std::byte, 4> file_tag;
    endian::little_uint32_t footer_offset;
  };

  struct old_volume_header
  {
    std::array<std::byte, 4> file_tag;
    endian::little_uint24_t footer_offset;
    std::byte padding;
  };

  static_assert(sizeof(volume_header) == sizeof(old_volume_header));

  struct normal_footer
  {
    std::array<std::byte, 4> string_header_tag;
    endian::little_uint32_t dummy2;
    std::array<std::byte, 4> dummy3;
    endian::little_uint32_t dummy4;
    std::array<std::byte, 4> dummy5;
    endian::little_uint32_t file_list_size;
  };

  struct alternative_footer
  {
    std::array<std::byte, 4> string_header_tag;
    endian::little_uint32_t file_list_size;
  };

  struct old_header
  {
    std::array<std::byte, 4> header_tag;
    endian::little_uint24_t header_size;
    std::byte padding;
  };

  struct old_footer
  {
    old_header header;
    std::array<std::byte, 4> string_header_tag;
    endian::little_uint24_t buffer_size;
    std::byte padding2;
    endian::little_uint24_t file_list_size;
    std::byte padding3;
  };

  static_assert(sizeof(alternative_footer) * 2 + sizeof(std::array<std::byte, 4>)
                == sizeof(old_footer));

  struct file_index_header
  {
    std::array<std::byte, 4> index_tag;
    endian::little_uint32_t index_size;
  };

  struct block_header
  {
    std::array<std::byte, 4> block_tag;
    endian::little_uint24_t block_size;
    std::byte end_byte;
  };

  struct file_header
  {
    endian::little_uint32_t id;
    endian::little_uint32_t name_empty_space;
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
    compression_type compression_type;
  };

  static_assert(sizeof(file_header) == sizeof(std::array<std::byte, 17>));

  struct old_file_header
  {
    endian::little_uint32_t id;
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
    std::uint8_t compression_type;
    std::uint8_t padding;
  };

  static_assert(sizeof(old_file_header) == sizeof(std::array<std::byte, 14>));


  void create_vol_file(std::basic_ostream<std::byte>& output, const std::vector<volume_file_info>& files)
  {
    auto start = std::size_t(output.tellp());

    endian::little_uint32_t size = 0;
    output.write(alt_vol_file_tag.data(), alt_vol_file_tag.size());

    output.write(reinterpret_cast<std::byte*>(&size), sizeof(size));

    std::map<std::string_view, endian::little_uint32_t> file_locations;

    for (auto& file : files)
    {
      file_locations.emplace(file.filename, endian::little_uint32_t(int(output.tellp())));
      output.write(block_tag.data(), block_tag.size());
      endian::little_uint24_t narrowed_size = file.compressed_size.has_value() ? file.compressed_size.value() : file.size;
      output.write(reinterpret_cast<std::byte*>(&narrowed_size), sizeof(narrowed_size));
      auto tag = std::byte(0x80);
      output.write(&tag, 1);

      std::copy_n(std::istreambuf_iterator<std::byte>(*file.stream),
                  narrowed_size,
                  std::ostreambuf_iterator<std::byte>(output));

      auto size_for_padding = int(output.tellp());
      while (size_for_padding % 4 != 0)
      {
        std::byte padding{0x00};
        output.write(&padding, 1);
        size_for_padding++;
      }
    }

    auto current_position = std::size_t(output.tellp());

    output.seekp(std::size_t(start) + alt_vol_file_tag.size(), std::ios_base::beg);
    size = std::int32_t(current_position - start);
    output.write(reinterpret_cast<std::byte*>(&size), sizeof(size));

    output.seekp(current_position, std::ios_base::beg);

    std::string filenames;
    filenames.reserve(10 * filenames.size());

    for (auto& file : files)
    {
      filenames.append(file.filename);
      filenames.push_back('\0');
    }

    endian::little_uint32_t string_size = std::int32_t(filenames.size());

    output.write(vol_string_tag.data(), vol_string_tag.size());
    output.write(reinterpret_cast<std::byte*>(&string_size), sizeof(string_size));
    output.write(reinterpret_cast<std::byte*>(filenames.data()), string_size);

    auto size_for_padding = string_size;
    while (size_for_padding % 2 != 0)
    {
      std::byte padding{0x00};
      output.write(&padding, 1);
      size_for_padding++;
    }

    string_size = std::int32_t(files.size() * sizeof(file_header));
    output.write(vol_index_tag.data(), vol_index_tag.size());
    output.write(reinterpret_cast<std::byte*>(&string_size), sizeof(string_size));

    for (auto& file : files)
    {
      endian::little_uint32_t value = 0;
      output.write(reinterpret_cast<std::byte*>(&value), sizeof(value));
      output.write(reinterpret_cast<std::byte*>(&value), sizeof(value));
      output.write(reinterpret_cast<std::byte*>(&file_locations[file.filename]), sizeof(value));

      value = file.size;
      output.write(reinterpret_cast<std::byte*>(&value), sizeof(value));
      output.write(reinterpret_cast<const std::byte*>(&file.compression_type), 1);
    }
  }

  std::tuple<volume_version, std::size_t, std::optional<std::size_t>> get_file_list_offsets(std::basic_istream<std::byte>& raw_data)
  {
    volume_header header{};

    raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

    if (header.file_tag == vol_file_tag)
    {
      normal_footer footer{};
      raw_data.seekg(header.footer_offset, std::ios_base::beg);
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      return std::make_tuple(volume_version::darkstar_vol, footer.file_list_size, std::nullopt);
    }
    else if (header.file_tag == alt_vol_file_tag)
    {
      alternative_footer footer{};
      raw_data.seekg(header.footer_offset, std::ios_base::beg);
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      return std::make_tuple(volume_version::darkstar_pvol, footer.file_list_size, std::nullopt);
    }
    else if (header.file_tag == old_vol_file_tag)
    {
      old_footer footer{};
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      if (footer.header.header_size > 0)
      {
        raw_data.seekg(-int(sizeof(footer)), std::ios::cur);
        raw_data.read(reinterpret_cast<std::byte*>(&footer.header), sizeof(footer.header));
        std::vector<std::byte> temp(footer.header.header_size);
        raw_data.read(temp.data(), temp.size());

        raw_data.read(reinterpret_cast<std::byte*>(&footer) + sizeof(footer.header), sizeof(footer) - sizeof(footer.header));
      }

      if (footer.string_header_tag != vol_string_tag)
      {
        throw std::invalid_argument("Volume file is not a Darkstar/Phoenix VOL file.");
      }

      auto amount_to_skip = footer.buffer_size - sizeof(int32_t) - footer.file_list_size;

      return std::make_tuple(volume_version::three_space_vol, footer.file_list_size, amount_to_skip);
    }
    else
    {
      throw std::invalid_argument("The file provided is not a valid Darkstar VOL file.");
    }
  }

  std::pair<volume_version, std::vector<std::string>> get_file_names(std::basic_istream<std::byte>& raw_data)
  {
    auto [volume_type, buffer_size, amount_to_skip] = get_file_list_offsets(raw_data);
    std::vector<char> raw_chars(buffer_size);

    raw_data.read(reinterpret_cast<std::byte*>(raw_chars.data()), raw_chars.size());

    if (volume_type != volume_version::three_space_vol && (buffer_size) % 2 != 0)
    {
      raw_data.seekg(1, std::ios::cur);
    }

    std::vector<std::string> results;

    if (!raw_chars.empty() && raw_chars.back() != '\0')
    {
      raw_chars.back() = '\0';
    }

    const auto max_filesize = volume_type == volume_version::three_space_vol ? 25 : 100;

    std::size_t index = 0;
    while (index < raw_chars.size())
    {
      results.emplace_back(raw_chars.data() + index);

      if (results.back().size() > max_filesize)
      {
        results.pop_back();
        break;
      }

      index += results.back().size() + 1;
    }

    if (amount_to_skip.has_value())
    {
      raw_data.seekg(amount_to_skip.value(), std::ios::cur);
    }

    return std::make_pair(volume_type, results);
  }

  std::vector<file_info> get_file_metadata(std::basic_istream<std::byte>& raw_data)
  {
    auto [volume_type, filenames] = get_file_names(raw_data);
    file_index_header header{};

    if (volume_type == volume_version::three_space_vol)
    {
      old_volume_header raw_header{};
      raw_data.read(reinterpret_cast<std::byte*>(&raw_header), sizeof(raw_header));
      header.index_tag = raw_header.file_tag;
      header.index_size = raw_header.footer_offset;
    }
    else
    {
      raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));
    }

    if (header.index_tag != vol_index_tag)
    {
      throw std::runtime_error("Could not load the VOL file index.");
    }

    std::vector<std::byte> raw_bytes(header.index_size);
    raw_data.read(raw_bytes.data(), raw_bytes.size());

    std::size_t index = 0;

    std::vector<file_info> results;
    results.reserve(filenames.size());

    while (index < raw_bytes.size())
    {
      endian::little_uint32_t offset;
      endian::little_uint32_t size;
      compression_type compression_type;

      if (volume_type == volume_version::three_space_vol)
      {
        old_file_header file{};

        std::copy(raw_bytes.data() + index,
          raw_bytes.data() + index + sizeof(old_file_header),
          reinterpret_cast<std::byte*>(&file));
        offset = file.offset;
        size = file.size;
        compression_type = vol::darkstar::compression_type(file.compression_type);

        index += sizeof(old_file_header);
      }
      else
      {
        file_header file{};
        std::copy(raw_bytes.data() + index,
          raw_bytes.data() + index + sizeof(file_header),
          reinterpret_cast<std::byte*>(&file));
        offset = file.offset;
        size = file.size;
        compression_type = file.compression_type;

        index += sizeof(file_header);
      }

      file_info info;

      info.filename = std::move(filenames[results.size()]);
      info.offset = offset;
      info.size = size;
      info.compression_type = compression_type;

      results.emplace_back(info);

      if (results.size() == filenames.size())
      {
        break;
      }
    }

    return results;
  }

  using folder_info = studio::resources::folder_info;

  bool vol_file_archive::is_supported(std::basic_istream<std::byte>& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(tag.data(), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == vol_file_tag || tag == alt_vol_file_tag || tag == old_vol_file_tag;
  }

  bool vol_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream) const
  {
    return is_supported(stream);
  }

  std::vector<vol_file_archive::content_info> vol_file_archive::get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const
  {
    std::vector<vol_file_archive::content_info> results;

    auto raw_results = get_file_metadata(stream);

    results.reserve(raw_results.size());

    std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](const auto& value) {
      studio::resources::file_info info{};
      info.filename = value.filename;
      info.offset = value.offset;
      info.size = value.size;
      info.compression_type = studio::resources::compression_type(value.compression_type);
      info.folder_path = archive_or_folder_path;
      return info;
    });

    return results;
  }

  void vol_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const
  {
    if (std::size_t(stream.tellg()) == info.offset)
    {
      stream.seekg(sizeof(vol::darkstar::file_index_header), std::ios::cur);
    }
    else if (std::size_t(stream.tellg()) != info.offset + sizeof(vol::darkstar::file_index_header))
    {
      stream.seekg(info.offset + sizeof(vol::darkstar::file_index_header), std::ios::beg);
    }
  }

  void vol_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const
  {
    if (info.compression_type == studio::resources::compression_type::none)
    {

      set_stream_position(stream, info);
      std::copy_n(std::istreambuf_iterator<std::byte>(stream),
                  info.size,
                  std::ostreambuf_iterator<std::byte>(output));
    }
    else
    {
      std::vector<std::filesystem::path> files_to_remove;
      std::stringstream command;

      auto volume_filename = info.folder_path;

      auto folder_path = volume_filename.parent_path() / "temp";
      {
        volume_header header{};

        auto volume_stream = std::unique_ptr<std::basic_istream<std::byte>>(new std::basic_ifstream<std::byte>(volume_filename, std::ios::binary));
        volume_stream->read(reinterpret_cast<std::byte*>(&header), sizeof(header));

        if (header.file_tag == old_vol_file_tag)
        {
          volume_filename = folder_path / (info.filename.filename().string() + ".vol");
          volume_stream->seekg(info.offset, std::ios::beg);

          block_header block{};
          volume_stream->read(reinterpret_cast<std::byte*>(&block), sizeof(block));

          if (block.block_tag != block_tag)
          {
            return;
          }

          std::vector<volume_file_info> files;
          volume_file_info temp_info{};
          temp_info.filename = info.filename.filename().string();
          temp_info.size = std::int32_t(info.size);
          temp_info.compressed_size = block.block_size;
          temp_info.compression_type = darkstar::compression_type(info.compression_type);
          temp_info.stream = std::move(volume_stream);

          files.emplace_back(std::move(temp_info));
          std::basic_ofstream<std::byte> new_volume_stream(volume_filename, std::ios::binary);
          create_vol_file(new_volume_stream, files);
          files_to_remove.emplace_back(volume_filename);
        }
      }

      auto new_path = folder_path / (info.filename.string() + ".tmp");

      std::filesystem::create_directory(folder_path);

      auto extract_path = std::filesystem::path("extract.exe");

      if (std::filesystem::exists(std::filesystem::current_path() / extract_path))
      {
        extract_path = std::filesystem::current_path() / extract_path;
      }
      else
      {
        auto root_path = volume_filename.root_path();
        auto parent_path = volume_filename.parent_path();

        do
        {
          if (std::filesystem::exists(parent_path / extract_path))
          {
            extract_path = parent_path / extract_path;
            break;
          }
          parent_path = parent_path.parent_path();
        }
        while(parent_path != root_path);
      }

      auto extract_path_str = extract_path.string();

      if (extract_path_str.find(" ") != std::string::npos)
      {
        command << "cd /d " << extract_path.parent_path() << " && " << extract_path.filename().string()
                << ' ' << volume_filename << ' ' << info.filename << ' ' << new_path;
      }
      else
      {
        command << extract_path.string() << ' ' << volume_filename << ' ' << info.filename << ' ' << new_path;
      }

      std::system(command.str().c_str());

      if (std::filesystem::exists(new_path) && std::filesystem::file_size(new_path) > info.size)
      {
        files_to_remove.emplace_back(new_path);
        {
          auto new_file = std::basic_ifstream<std::byte>{ new_path, std::ios::binary };

          std::copy_n(std::istreambuf_iterator<std::byte>(new_file),
            info.size,
            std::ostreambuf_iterator<std::byte>(output));
        }

        for (auto& file : files_to_remove)
        {
          std::filesystem::remove(file);
        }
      }
    }
  }
}// namespace darkstar::vol
