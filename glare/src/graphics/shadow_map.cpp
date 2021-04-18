#include "shadow_map.hpp"

#include "context.hpp"
#include "texture.hpp"
#include "framebuffer.hpp"

namespace graphics
{
	ShadowMap::ShadowMap(pass_ref<graphics::Texture> depth_map, pass_ref<graphics::FrameBuffer> framebuffer)
		: depth_map(depth_map), framebuffer(framebuffer)
	{}

	ShadowMap::ShadowMap(pass_ref<graphics::Context>& context, int width, int height) :
		depth_map
		(
			std::make_shared<graphics::Texture>
			(
				context,
				width, height,
				graphics::TextureFormat::Depth, graphics::ElementType::Float,
				graphics::TextureFlags::Clamp, graphics::TextureType::CubeMap,
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