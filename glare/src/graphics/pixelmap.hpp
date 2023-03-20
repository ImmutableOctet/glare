#pragma once

#include "resource.hpp"
#include "texture_format.hpp"

#include <cstdint>
#include <string>
#include <type_traits>

namespace graphics
{
	class Texture;

	class PixelMap
	{
		public:
			using RawPtr = std::uint8_t*;
		protected: // private:
			RawPtr raw_data;

			int image_width, image_height;
			int image_channels;
		public:
			friend Texture;

			static int get_format_channels(TextureFormat format);
			static TextureFormat get_format_for(int channels);

			static PixelMap Load(const std::string& path, int color_channels=0);
			static PixelMap Load(const char* path, int color_channels=0);

			inline int width()    const { return image_width;    }
			inline int height()   const { return image_height;   }
			inline int channels() const { return image_channels; }

			inline TextureFormat format() const { return get_format_for(channels()); }

			inline int size() const { return (width() * height() * channels()); }
			inline const RawPtr data() const { return raw_data; }

			inline bool has_data() const { return ((data() != nullptr) && (size() > 0)); }
			inline explicit operator bool() const { return has_data(); }
		protected:
			PixelMap(RawPtr&& raw_data, int width, int height, int color_channels);
			~PixelMap();

			PixelMap(const PixelMap&) = delete;
	};
}