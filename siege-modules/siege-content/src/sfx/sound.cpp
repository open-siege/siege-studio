#include <siege/content/sfx/sound.hpp>

namespace siege::content::sfx
{
	constexpr file_tag ogg_tag = platform::to_tag<4>({ 'O', 'g', 'g', 'S' });

	constexpr file_tag flac_tag = platform::to_tag<4>({ 'f', 'L', 'a', 'C' });

	bool is_ogg(std::istream& stream)
	{
		std::array<std::byte, 4> tag{};
		stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

		stream.seekg(-int(sizeof(tag)), std::ios::cur);

		return tag == ogg_tag;
	}

	bool is_flac(std::istream& stream)
	{
		std::array<std::byte, 4> tag{};
		stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

		stream.seekg(-int(sizeof(tag)), std::ios::cur);

		return tag == flac_tag;
	}
}