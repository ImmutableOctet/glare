#pragma once

#include <string>
#include <type_traits>

#include <debug.hpp>

#include "resource.hpp"

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

			static PixelMap Load(const std::string& path, int color_channels=0);

			inline int width()    const { return image_width;    }
			inline int height()   const { return image_height;   }
			inline int channels() const { return image_channels; }

			inline int size() const { return (width() * height() * channels()); }
			inline const RawPtr data() const { return raw_data; }

			inline bool has_data() const { return ((data() != nullptr) && (size() > 0)); }
			inline operator bool() const { return has_data(); }
		protected:
			PixelMap(RawPtr&& raw_data, int width, int height, int color_channels);
			~PixelMap();

			PixelMap(const PixelMap&) = delete;
	};
}