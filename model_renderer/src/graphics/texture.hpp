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
		public:
			friend Context;
			friend ContextState;

			using Flags = TextureFlags;

			inline friend void swap(Texture& x, Texture& y)
			{
				swap(static_cast<Resource&>(x), static_cast<Resource&>(y));
			}

			Texture(pass_ref<Context> ctx, const std::string& path, Flags flags=Flags::Default);
			Texture(pass_ref<Context> ctx, const PixelMap& data, Flags flags=Flags::Default);
			Texture(pass_ref<Context> ctx, int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags=Flags::None);
			
			Texture(Texture&& texture) : Texture() { swap(*this, texture); }

			inline Texture() : Texture({}, {}) {}

			Texture(const Texture&) = delete;

			~Texture();

			inline Texture& operator=(Texture texture)
			{
				swap(*this, texture);

				return *this;
			}
		protected:
			Texture(weak_ref<Context> ctx, ContextHandle&& resource_handle);
	};
}