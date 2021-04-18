#pragma once

#include "types.hpp"

namespace graphics
{
	class Context;
	class Texture;
	class FrameBuffer;

	class ShadowMap
	{
		public:
			ShadowMap(pass_ref<graphics::Context>& context, int width, int height);
			ShadowMap(pass_ref<graphics::Texture> depth_map, pass_ref<graphics::FrameBuffer> framebuffer={});

			ShadowMap(ShadowMap&&) noexcept = default;

			ShadowMap& operator=(ShadowMap&&) noexcept = default;

			math::vec2i get_resolution() const;
			graphics::Viewport get_viewport() const;

			ref<graphics::Texture>& get_depth_map() { return depth_map; }
			ref<graphics::FrameBuffer>& get_framebuffer() { return framebuffer; }
		protected:
			ref<graphics::Texture> depth_map;
			ref<graphics::FrameBuffer> framebuffer;
	};
}