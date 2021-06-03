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
			GBuffer(pass_ref<Context> context, int width, int height, GBufferFlags flags=GBufferFlags::Default, TextureFlags texture_flags=TextureFlags::None);

			inline GBuffer(pass_ref<Context> context, std::tuple<int, int> resolution, GBufferFlags flags=GBufferFlags::Default, TextureFlags texture_flags=TextureFlags::None)
				: GBuffer(context, std::get<0>(resolution), std::get<1>(resolution), flags, texture_flags) {}

			GBuffer(GBuffer&&) = default;
			GBuffer& operator=(GBuffer&&) = default;

			void resize(int width, int height);

			graphics::FrameBuffer framebuffer;

			graphics::Texture normal;
			graphics::Texture albedo_specular;

			std::optional<graphics::Texture> position;
			std::optional<graphics::Texture> depth;

			std::optional<graphics::Texture> render_flags;

			graphics::Mesh screen_quad;
	};
}