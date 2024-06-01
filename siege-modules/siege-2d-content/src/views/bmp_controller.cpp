#include "bmp_controller.hpp"
#include "pal_controller.hpp"
#include <siege/content/bmp/bitmap.hpp>
#include <siege/content/bmp/image.hpp>
#include <deque>
#include <filesystem>
#include <future>
#include <execution>

namespace siege::views
{
	bool bmp_controller::is_bmp(std::istream& image_stream) noexcept
	{
		return siege::content::bmp::is_earthsiege_bmp(image_stream)
			|| siege::content::bmp::is_microsoft_bmp(image_stream)
			|| siege::content::bmp::is_phoenix_bmp(image_stream)
			|| siege::content::bmp::is_jpg(image_stream)
			|| siege::content::bmp::is_gif(image_stream)
			|| siege::content::bmp::is_png(image_stream);
	}

	std::mutex& get_lock()
	{
		static std::mutex palette_lock;

		return palette_lock;
	}

	auto get_default_palette()
	{
		siege::content::pal::palette temp;

		temp.colours.reserve(255);

		for (auto i = 0; i < 256; ++i)
		{
			temp.colours.emplace_back(std::byte(i), std::byte(i), std::byte(i), std::byte(i));
		}

		return temp;
	}

	std::future<void> bmp_controller::load_palettes_async(std::optional<std::filesystem::path> folder_hint)
	{
		namespace fs = std::filesystem;

		return std::async(std::launch::async, [this]() {

			std::lock_guard<std::mutex> guard(get_lock());
			std::deque<fs::path> pal_paths;

			for (auto entry = fs::recursive_directory_iterator(fs::current_path());
				entry != fs::recursive_directory_iterator();
				++entry)
			{
				if (entry.depth() > 3)
				{
					entry.disable_recursion_pending();
					continue;
				}

				if (!fs::is_directory(entry->path()))
				{
					auto is_pal = std::find_if(pal_controller::formats.begin(), pal_controller::formats.end(), [&](auto& ext) {
						return entry->path().extension() == ext;
						}) != pal_controller::formats.end();

						if (is_pal)
						{
							pal_paths.emplace_back(entry->path());

						}
				}
			}
			palettes.resize(pal_paths.size());

			std::transform(std::execution::par_unseq, pal_paths.begin(), pal_paths.end(), palettes.begin(), [](auto& path) {
				std::fstream temp(path);

				std::vector<content::pal::palette> results;
				if (siege::content::pal::is_microsoft_pal(temp))
				{
					results.emplace_back().colours = siege::content::pal::get_pal_data(temp);
				}
				else if (siege::content::pal::is_earthsiege_pal(temp))
				{
					results.emplace_back().colours = siege::content::pal::get_earthsiege_pal(temp);
				}
				else if (siege::content::pal::is_phoenix_pal(temp))
				{
					return siege::content::pal::get_ppl_data(temp);
				}

				if (results.empty())
				{
					auto& back = results.emplace_back(get_default_palette());
				}

				return results;
				});

			});
	}

	std::size_t bmp_controller::load_bitmap(std::istream& image_stream) noexcept
	{
		using namespace siege::content;

		try
		{
			if (bmp::is_microsoft_bmp(image_stream))
			{
				original_image.emplace(bmp::get_bmp_data(image_stream));
			}
			else if (bmp::is_phoenix_bmp(image_stream))
			{
				auto image = bmp::get_pbmp_data(image_stream);

				bmp::windows_bmp_data dest{};

				dest.info.bit_depth = 32;
				dest.info.width = image.bmp_header.width;
				dest.info.height = image.bmp_header.height;

				{
					std::lock_guard<std::mutex> guard(get_lock());

					if (!palettes.empty())
					{
						auto palette_iter = std::find_if(palettes.begin(), palettes.end(), [&](std::vector<pal::palette>& group) {
							return std::any_of(group.begin(), group.end(), [&](pal::palette& pal) {
								return pal.index == image.palette_index;
								});
							});

						if (palette_iter != palettes.end())
						{
							auto exepcted_pal = std::find_if(palette_iter->begin(), palette_iter->end(), [&](pal::palette& pal) {
								return pal.index == image.palette_index;
								});

							dest.colours = exepcted_pal->colours;
						}
						else
						{
							dest.colours = palettes.begin()->begin()->colours;
						}
					}
					else
					{
						dest.colours = get_default_palette().colours;
					}


				}

				dest.indexes.reserve(image.pixels.size());
				std::transform(image.pixels.begin(), image.pixels.end(), std::back_inserter(dest.indexes), [](auto& value) {
					return std::int32_t(value);
					});

				original_image.emplace(std::move(dest));
			}
			else if (bmp::is_earthsiege_bmp(image_stream))
			{
			}
			else
			{
				original_image.emplace(image_stream);
			}
		}
		catch (...)
		{
		}

		if (original_image)
		{
			return original_image->frame_count();
		}

		return 0;
	}

	std::size_t bmp_controller::convert(std::size_t frame, std::pair<int, int> size, int bits, std::span<std::byte> destination) const noexcept
	{
		if (original_image)
		{
			return original_image->convert(frame, size, bits, destination);
		}

		return 0;
	}
}