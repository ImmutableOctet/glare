#include "gbuffer.hpp"

namespace graphics
{
	GBuffer::GBuffer(pass_ref<Context> context, int width, int height, GBufferFlags flags, TextureFlags texture_flags) :
		screen_quad(Mesh::GenerateTexturedQuad(context)),
		framebuffer(context)
	{
		auto& fb = framebuffer;

		context->use(fb, [&, this]()
		{
			if ((flags & GBufferFlags::Position))
			{
				position = Texture(context, width, height, TextureFormat::RGB, ElementType::Half, texture_flags); // Float
				//auto _pos = graphics.context->use(g_buffer.position);
				fb.attach(*position);
			}

			normal = Texture(context, width, height, TextureFormat::RGB, ElementType::Half, texture_flags); // Float
			fb.attach(normal);

			albedo_specular = Texture(context, width, height, TextureFormat::RGBA, ElementType::UByte, texture_flags);
			fb.attach(albedo_specular);

			if ((flags & GBufferFlags::DepthTexture))
			{
				depth = Texture
				(
					context, width, height,
					TextureFormat::Depth, ElementType::Float,
					(texture_flags | graphics::TextureFlags::Clamp),
					TextureType::Texture2D, graphics::ColorRGBA{ 1.0, 1.0, 1.0, 1.0 }
				);

				fb.attach(*depth);
			}
			else
			{
				fb.attach(RenderBufferType::Depth, width, height);
			}

			fb.link();

			//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			//	std::cout << "Framebuffer not complete!" << std::endl;
		});

		context->clear_textures(true);

		//auto op = context->use(graphics.context->get_default_framebuffer());
	}

	void GBuffer::resize(int width, int height)
	{
		framebuffer.resize(width, height);

		if (depth)
		{
			depth->resize(width, height);
		}
	}
}