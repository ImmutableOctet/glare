#pragma once

#include "types.hpp"
#include "texture_type.hpp"
#include "viewport.hpp"

#include <math/types.hpp>

#include <memory>

namespace graphics
{
	class Context;
	class Texture;
	class FrameBuffer;

	class ShadowMap
	{
		public:
			ShadowMap(const std::shared_ptr<graphics::Context>& context, int width, int height, TextureType texture_type=TextureType::Texture2D);
			ShadowMap(const std::shared_ptr<graphics::Texture>& depth_map, const std::shared_ptr<graphics::FrameBuffer>& framebuffer={});

			ShadowMap(ShadowMap&&) noexcept = default;

			ShadowMap& operator=(ShadowMap&&) noexcept = default;

			math::vec2i get_resolution() const;
			graphics::Viewport get_viewport() const;

			std::shared_ptr<graphics::Texture>& get_depth_map() { return depth_map; }
			std::shared_ptr<graphics::FrameBuffer>& get_framebuffer() { return framebuffer; }
		protected:
			std::shared_ptr<graphics::Texture> depth_map;
			std::shared_ptr<graphics::FrameBuffer> framebuffer;
	};
}