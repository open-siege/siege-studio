#ifndef VOL_CONTROLLER_HPP
#define VOL_CONTROLLER_HPP

#include <istream>
#include <vector>
#include <variant>
#include <filesystem>
#include <sstream>
#include <siege/platform/archive_plugin.hpp>

namespace siege::views
{
	class vol_controller
	{		
	public:

		static bool is_vol(std::istream& image_stream) noexcept;

	    std::size_t load_volume(std::istream& image_stream, std::optional<std::filesystem::path> path);

	private: 
		std::unique_ptr<siege::platform::archive_plugin> resource;
		std::vector<siege::platform::archive_plugin::content_info> contents;
		std::variant<std::monostate, std::filesystem::path, std::stringstream> storage;
	};
}

#endif // !VOL_CONTROLLER_HPP
