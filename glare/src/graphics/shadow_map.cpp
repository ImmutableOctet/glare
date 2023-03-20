#include "shadow_map.hpp"

#include "context.hpp"
#include "texture.hpp"
#include "framebuffer.hpp"

namespace graphics
{
	ShadowMap::ShadowMap(const std::shared_ptr<graphics::Texture>& depth_map, const std::shared_ptr<graphics::FrameBuffer>& framebuffer)
		: depth_map(depth_map), framebuffer(framebuffer)
	{}

	ShadowMap::ShadowMap(const std::shared_ptr<graphics::Context>& context, int width, int height, TextureType texture_type) :
		depth_map
		(
			std::make_shared<graphics::Texture>
			(
				context,
				width, height,
				graphics::TextureFormat::Depth, graphics::ElementType::Float,
				graphics::TextureFlags::Clamp, texture_type, // graphics::TextureType::CubeMap
				graphics::ColorRGBA { 1.0, 1.0, 1.0, 1.0 }
			)
		),

		framebuffer(std::make_shared<graphics::FrameBuffer>(context))
	{
		context->use(*framebuffer, [&, this]()
		{
			framebuffer->attach(*depth_map);
			//framebuffer.attach(graphics::RenderBufferType::Depth, width, height);

			framebuffer->link();
		});
	}

	math::vec2i ShadowMap::get_resolution() const
	{
		//return framebuffer->...
		return depth_map->get_size();
	}

	graphics::Viewport ShadowMap::get_viewport() const
	{
		return depth_map->rect();
	}
}