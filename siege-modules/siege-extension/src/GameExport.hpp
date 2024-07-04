#ifndef GAME_EXPORT_HPP
#define GAME_EXPORT_HPP

#include <array>
#include <utility>
#include <string_view>
#include <type_traits>

namespace siege
{
	template<std::size_t verification_size = 3, std::size_t function_name_size = 4, std::size_t variable_name_size = 1>
	struct game_export
	{
		std::size_t preferred_base_address;
		std::size_t module_size;
        std::array<std::pair<std::string_view, std::size_t>, verification_size> verification_strings;
        std::array<std::pair<std::string_view, std::string_view>, function_name_size> function_name_ranges;
        std::array<std::pair<std::string_view, std::string_view>, variable_name_size> variable_name_ranges;
        std::size_t console_eval;
	};
}

#endif