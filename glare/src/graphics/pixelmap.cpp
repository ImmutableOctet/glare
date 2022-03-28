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

	int PixelMap::get_format_channels(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::R:
				return 1;
			case TextureFormat::RG:
				return 2;
			case TextureFormat::RGB:
				return 3;
			case TextureFormat::RGBA:
				return 4;
		}

		return 0;
	}

	TextureFormat PixelMap::get_format_for(int channels)
	{
		switch (channels)
		{
			case 1:
				return TextureFormat::R;
			case 2:
				return TextureFormat::RG;
			case 3:
				return TextureFormat::RGB;
			case 4:
				return TextureFormat::RGBA;
		}

		return TextureFormat::Unknown;
	}

	PixelMap PixelMap::Load(const std::string& path, int desired_channels)
	{
		return Load(path.c_str(), desired_channels);
	}

	PixelMap PixelMap::Load(raw_string path, int desired_channels)
	{
		int width, height, color_channels;

		RawPtr data = reinterpret_cast<RawPtr>(stbi_load(path, &width, &height, &color_channels, desired_channels));

		if (desired_channels <= 0)
		{
			desired_channels = color_channels;
		}

		// TODO: Add proper handling for invalid 'PixelMap' objects.
		///assert(data);

		return PixelMap(std::move(data), width, height, desired_channels);
	}
}