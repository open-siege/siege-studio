#ifndef VOL_CONTROLLER_HPP
#define VOL_CONTROLLER_HPP

#include <istream>
#include <vector>
#include <variant>
#include <filesystem>
#include <sstream>
#include <span>
#include <siege/platform/resource.hpp>

namespace siege::views
{
	class vol_controller
	{		
	public:
		constexpr static auto formats = std::array<std::wstring_view, 20>{{
				L".vol", L".rmf", L".mis", L".map", L".rbx", L".tbv", L".zip", L".vl2", L".pk3",
					L".iso", L".mds", L".cue", L".nrg", L".7z", L".tgz", L".rar", L".cab", L".z", L".cln", L".atd"
			}};
		static bool is_vol(std::istream& image_stream) noexcept;

	    std::size_t load_volume(std::istream& image_stream, std::optional<std::filesystem::path> path);

		std::span<siege::platform::resource_reader::content_info> get_contents();
	private: 
		std::unique_ptr<siege::platform::resource_reader> resource;
		std::vector<siege::platform::resource_reader::content_info> contents;
		std::variant<std::monostate, std::filesystem::path, std::stringstream> storage;
	};
}

#endif // !VOL_CONTROLLER_HPP
