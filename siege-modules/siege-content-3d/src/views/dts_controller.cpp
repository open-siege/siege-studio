#include <siege/content/dts/darkstar.hpp>
#include <siege/content/dts/3space.hpp>
#include "dts_controller.hpp"


namespace siege::views
{
	bool dts_controller::is_shape(std::istream& image_stream)
	{
		return siege::content::dts::darkstar::is_darkstar_dts(image_stream)
			|| siege::content::dts::three_space::v1::is_3space_dts(image_stream);
	}

	std::size_t dts_controller::load_shape(std::istream& image_stream)
	{
		return 1;
	}
}