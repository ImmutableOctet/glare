#pragma once

#include "types.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "framebuffer.hpp"
#include "gbuffer_flags.hpp"

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
			GBuffer(const std::shared_ptr<Context>& context, int width, int height, GBufferFlags flags=GBufferFlags::Default, TextureFlags texture_flags=TextureFlags::None);

			inline GBuffer(const std::shared_ptr<Context>& context, std::tuple<int, int> resolution, GBufferFlags flags=GBufferFlags::Default, TextureFlags texture_flags=TextureFlags::None)
				: GBuffer(context, std::get<0>(resolution), std::get<1>(resolution), flags, texture_flags) {}

			GBuffer(GBuffer&&) noexcept = default;
			GBuffer& operator=(GBuffer&&) noexcept = default;

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