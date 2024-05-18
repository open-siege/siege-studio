#include <siege/content/sfx/wave.hpp>
#include "sfx_controller.hpp"

namespace siege::views
{
	bool sfx_controller::is_sfx(std::istream& stream)
	{
		return siege::content::sfx::is_wav(stream)
			|| siege::content::sfx::is_flac(stream)
			|| siege::content::sfx::is_ogg(stream);
	}

	std::size_t sfx_controller::load_sound(std::istream& image_stream) noexcept
	{
		using namespace siege::content;

        try
        {
            original_sound.emplace(image_stream);
        }
        catch(...)
        {
        }

        if (original_sound)
        {
            return original_sound->track_count();
        }

        return 1;
	}
}