#pragma once

#include "types.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "framebuffer.hpp"
#include "gbuffer.hpp"

namespace glare
{
	class Glare;
}

namespace graphics
{
	class Context;

	class GBuffer
	{
		public:
			GBuffer(pass_ref<Context> context, int width, int height, TextureFlags flags=TextureFlags::None);

			inline GBuffer(pass_ref<Context> context, std::tuple<int, int> resolution, TextureFlags flags=TextureFlags::None)
				: GBuffer(context, std::get<0>(resolution), std::get<1>(resolution), flags) {}

			GBuffer(GBuffer&&) = default;
			GBuffer& operator=(GBuffer&&) = default;

			void resize(int width, int height);

			graphics::FrameBuffer framebuffer;

			graphics::Texture position;
			graphics::Texture normal;
			graphics::Texture albedo_specular;

			graphics::Mesh screen_quad;
	};
}