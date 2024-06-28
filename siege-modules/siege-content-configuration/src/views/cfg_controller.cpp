#include <siege/configuration/id_tech.hpp>
#include <siege/configuration/unreal.hpp>
#include <siege/platform/stream.hpp>
#include "cfg_controller.hpp"

namespace siege::views
{
	bool cfg_controller::is_cfg(std::istream& stream)
	{
        auto config = siege::configuration::id_tech::id_tech_2::load_config(stream, siege::platform::get_stream_size(stream));

		if (config)
		{
			return true;
		}

		return false;
	}

	std::size_t cfg_controller::load_config(std::istream& image_stream) noexcept
	{
		return 0;
	}
}