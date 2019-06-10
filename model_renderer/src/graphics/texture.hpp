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

			Texture(pass_ref<Context> ctx, const std::string& path, Flags flags=Flags::Default);
			Texture(pass_ref<Context> ctx, const PixelMap& data, Flags flags=Flags::Default);

			inline Texture() : Texture({}, {}) {}

			~Texture();
		protected:
			Texture(weak_ref<Context> ctx, Context::Handle&& resource_handle);
	};
}