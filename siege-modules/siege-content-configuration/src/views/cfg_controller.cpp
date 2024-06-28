#include <siege/configuration/id_tech.hpp>
#include <siege/configuration/unreal.hpp>
#include <siege/configuration/build.hpp>
#include <siege/configuration/serious.hpp>
#include <siege/platform/stream.hpp>
#include "cfg_controller.hpp"

namespace siege::views
{
	bool cfg_controller::is_cfg(std::istream& stream)
	{
		return siege::configuration::is_ascii_text_config(stream);
	}

	std::size_t cfg_controller::load_config(std::istream& stream) noexcept
	{
		using namespace siege::configuration::id_tech;
		using namespace siege::configuration::unreal;
		using namespace siege::configuration::serious;

		auto size = siege::platform::get_stream_size(stream);

		auto result = id_tech_2::load_config(stream, size)
                                .or_else([&] { return unreal_1::load_config(stream, size); })
                                .or_else([&] { return serious_1::load_config(stream, size); });

		if (!result)
		{
			return 0;
		}

		text_config.emplace(std::move(*result));

		return 1;
	}
}