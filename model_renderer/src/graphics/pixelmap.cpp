#include "pixelmap.hpp"

//#include <algorithm>
#include <stb/stb_image.h>

namespace graphics
{
	// PixelMap:
	PixelMap::PixelMap(RawPtr&& raw_data, int width, int height, int color_channels)
		: raw_data(std::move(raw_data)), image_width(width), image_height(height), image_channels(color_channels) {}

	PixelMap::~PixelMap()
	{
		stbi_image_free(raw_data);
	}

	PixelMap PixelMap::Load(const std::string& path, int desired_channels)
	{
		int width, height, color_channels;

		RawPtr data = reinterpret_cast<RawPtr>(stbi_load(path.c_str(), &width, &height, &color_channels, desired_channels));

		if (desired_channels <= 0)
		{
			desired_channels = color_channels;
		}

		// TODO: Add proper handling for invalid 'PixelMap' objects.
		ASSERT(data);

		return PixelMap(std::move(data), width, height, desired_channels);
	}
}