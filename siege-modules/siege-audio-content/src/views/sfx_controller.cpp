#include <siege/content/sfx/wave.hpp>
#include "sfx_controller.hpp"

namespace siege::views
{
	bool sfx_controller::is_sfx(std::istream& image_stream)
	{
		return false;
	}

	std::size_t sfx_controller::load_sound(std::istream& image_stream)
	{
		return 0;
	}
}