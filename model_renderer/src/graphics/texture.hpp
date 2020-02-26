#pragma once

#include <string>
#include <type_traits>

#include <debug.hpp>

#include "types.hpp"
#include "resource.hpp"

namespace graphics
{
	class PixelMap;

	class Texture : public Resource
	{
		protected:
			int width, height;

			TextureFormat format;
			ElementType element_type;
			TextureFlags flags;
		public:
			friend Context;
			friend ContextState;

			using Flags = TextureFlags;

			friend void swap(Texture& x, Texture& y);

			Texture(pass_ref<Context> ctx, const std::string& path, Flags flags=Flags::Default);
			Texture(pass_ref<Context> ctx, const PixelMap& data, Flags flags=Flags::Default);
			Texture(pass_ref<Context> ctx, int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags=Flags::None);
			
			Texture(Texture&& texture) : Texture() { swap(*this, texture); }

			inline Texture() : Texture({}, {}, 0, 0, TextureFormat::Unknown, ElementType::Unknown, TextureFlags::None) {}

			Texture(const Texture&) = delete;

			~Texture();

			inline Texture& operator=(Texture texture)
			{
				swap(*this, texture);

				return *this;
			}

			inline int get_width() const { return width; }
			inline int get_height() const { return height; }

			inline TextureFormat get_format() const { return format; }
			inline ElementType get_element_type() const { return element_type; }
			inline TextureFlags get_flags() const { return flags;  }
		protected:
			Texture(weak_ref<Context> ctx, ContextHandle&& resource_handle, int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags);
	};
}