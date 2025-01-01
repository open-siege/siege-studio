// BWD - MechWarrior 2 complete 3D model data (referencing individual WTB meshes)
// Uses file IDs from the PRJ file instead of file names.

#include <array>
#include <any>
#include <map>
#include <vector>
#include <string>
#include <spanstream>
#include <optional>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::bwd
{
  namespace endian = siege::platform;

  constexpr auto header_tag = platform::to_tag<4>({ 'B', 'W', 'D', '\0' });
  constexpr auto rev_tag = platform::to_tag<4>({ 'R', 'E', 'V', '\0' });
  constexpr auto dtbl_tag = platform::to_tag<4>({ 'D', 'T', 'B', 'L' });
  constexpr auto mon_tag = platform::to_tag<4>({ 'M', 'O', 'N', '\0' });
  constexpr auto object_tag = platform::to_tag<4>({ 'O', 'B', 'J', '\0' });
  constexpr auto repr_tag = platform::to_tag<4>({ 'R', 'E', 'P', 'R' });
  constexpr auto endr_tag = platform::to_tag<4>({ 'E', 'N', 'D', 'R' });
  constexpr auto thing_tag = platform::to_tag<4>({ 'T', 'H', 'N', 'G' });
  constexpr auto object_link_tag = platform::to_tag<4>({ 'O', 'B', 'J', 'L' });
  constexpr auto anim_tag = platform::to_tag<4>({ 'A', 'N', 'I', 'M' });
  constexpr auto tsk_tag = platform::to_tag<4>({ 'T', 'S', 'K', '\0' });
  constexpr auto pofo_tag = platform::to_tag<4>({ 'P', 'O', 'F', 'O' });
  constexpr auto eye_tag = platform::to_tag<4>({ 'E', 'Y', 'E', 'O' });
  constexpr auto vpt_file_tag = platform::to_tag<4>({ 'V', 'P', 'T', 'F' });
  constexpr auto cockpit_file_tag = platform::to_tag<4>({ 'C', 'P', 'T', 'F' });
  constexpr auto hud_file_tag = platform::to_tag<4>({ 'H', 'U', 'D', 'F' });
  constexpr auto mgd_file_tag = platform::to_tag<4>({ 'M', 'G', 'D', 'F' });
  constexpr auto pit_file_tag = platform::to_tag<4>({ 'P', 'I', 'T', 'F' });
  constexpr auto mof_file_tag = platform::to_tag<4>({ 'M', 'O', 'F', 'F' });
  constexpr auto asnd_tag = platform::to_tag<4>({ 'A', 'S', 'N', 'D' });

  struct iff_tag
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t size;
  };

  struct dtbl_info
  {
    std::array<endian::little_uint16_t, 2> unknown;
    endian::little_uint16_t something_count;
    endian::little_uint16_t object_count;
    std::array<endian::little_uint16_t, 4> unknown2;
    endian::little_uint16_t tsk_count;
    endian::little_uint16_t popo_count;
  };

  struct vec_3s
  {
    endian::little_int16_t x;
    endian::little_int16_t y;
    endian::little_int16_t z;
  };

  struct dtb_object
  {
    endian::little_int16_t id;
    vec_3s origin;
    std::array<endian::little_uint16_t, 11> unknown;
    vec_3s scale;
    vec_3s translation;
    std::array<endian::little_uint16_t, 3> unknown2;
    endian::little_int16_t prf_file_index;
    endian::little_int16_t padding;
  };

  struct file_info
  {
    endian::little_int16_t prj_file_index;
    std::array<char, 10> string_tag;
  };

  struct bwd_model
  {
    std::array<char, 4> version;
    std::vector<dtb_object> objects;
    std::map<int, std::vector<dtb_object>> lod_objects;
    file_info animation_file;
    file_info vpt_file;
    file_info cockpit_file;
    file_info hud_file;
    file_info mgd_file;
    file_info pit_file;
    file_info sound_file;
  };


  bool is_bwd(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);

    iff_tag tag;

    stream.read((char*)&tag, sizeof(tag));

    return tag.tag == header_tag;
  }

  std::any load_bwd(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    bwd_model result;
    iff_tag tag;

    stream.read((char*)&tag, sizeof(tag));

    if (tag.tag == header_tag)
    {
      stream.seekg(sizeof(std::uint32_t), std::ios::cur);
      std::vector<char> bwd_data(tag.size - sizeof(tag) - sizeof(std::uint32_t));
      stream.read(bwd_data.data(), bwd_data.size());

      std::ispanstream bwd_stream(bwd_data);

      std::optional<int> lod_index = std::nullopt;

      while (!(bwd_stream.eof() || bwd_stream.fail()))
      {
        iff_tag child_tag;
        bwd_stream.read((char*)&child_tag, sizeof(child_tag));

        if (bwd_stream.eof() || bwd_stream.fail())
        {
          break;
        }

        if (child_tag.tag == rev_tag && child_tag.size == 12)
        {
          bwd_stream.read(result.version.data(), sizeof(result.version));
        }
        else if (child_tag.tag == object_tag && child_tag.size == 60)
        {
          if (!lod_index)
          {
            auto& object = result.objects.emplace_back();
            bwd_stream.read((char*)&object, sizeof(object));
          }
          else
          {
            auto& object = result.lod_objects[*lod_index].emplace_back();
            bwd_stream.read((char*)&object, sizeof(object));
          }
        }
        else if (child_tag.tag == repr_tag && child_tag.size == 8)
        {
          if (!lod_index)
          {
            lod_index = 0;
          }
          else
          {
            lod_index = *lod_index + 1;
          }

          result.lod_objects[*lod_index] = std::vector<dtb_object>{};
        }
        else if (child_tag.tag == anim_tag && child_tag.size == 20)
        {
          bwd_stream.read((char*)&result.animation_file, sizeof(result.animation_file));
        }
        else if (child_tag.tag == vpt_file_tag && child_tag.size == 20)
        {
          bwd_stream.read((char*)&result.vpt_file, sizeof(result.vpt_file));
        }
        else if (child_tag.tag == cockpit_file_tag && child_tag.size == 20)
        {
          bwd_stream.read((char*)&result.cockpit_file, sizeof(result.cockpit_file));
        }
        else if (child_tag.tag == hud_file_tag && child_tag.size == 20)
        {
          bwd_stream.read((char*)&result.hud_file, sizeof(result.hud_file));
        }
        else if (child_tag.tag == pit_file_tag && child_tag.size == 20)
        {
          bwd_stream.read((char*)&result.pit_file, sizeof(result.pit_file));
        }
        else if (child_tag.tag == mgd_file_tag && child_tag.size == 20)
        {
          bwd_stream.read((char*)&result.mgd_file, sizeof(result.mgd_file));
        }
        else if (child_tag.tag == asnd_tag && child_tag.size == 24)
        {
          bwd_stream.read((char*)&result.sound_file, sizeof(result.sound_file));
          bwd_stream.seekg(sizeof(std::uint32_t), std::ios::cur);
        }
        else if (child_tag.tag == endr_tag && child_tag.size == 8)
        {
          lod_index = std::nullopt;
        }
        else
        {
          bwd_stream.seekg(child_tag.size - sizeof(iff_tag), std::ios::cur);
        }
      }
    }

    return result;
  }
}// namespace siege::content::bwd
