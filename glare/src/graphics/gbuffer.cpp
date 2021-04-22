#include "gbuffer.hpp"

namespace graphics
{
	GBuffer::GBuffer(pass_ref<Context> context, int width, int height, TextureFlags flags) :
		screen_quad(Mesh::GenerateTexturedQuad(context)),
		framebuffer(context)
	{
		auto& fb = framebuffer;

		context->use(fb, [&, this]()
		{
			position = Texture(context, width, height, TextureFormat::RGB, ElementType::Half, flags); // Float
			//auto _pos = graphics.context->use(g_buffer.position);
			fb.attach(position);

			normal = Texture(context, width, height, TextureFormat::RGB, ElementType::Half, flags); // Float
			fb.attach(normal);

			albedo_specular = Texture(context, width, height, TextureFormat::RGBA, ElementType::UByte, flags);
			fb.attach(albedo_specular);

			fb.link();

			fb.attach(RenderBufferType::Depth, width, height);

			//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			//	std::cout << "Framebuffer not complete!" << std::endl;
		});

		context->clear_textures(true);

		//auto op = context->use(graphics.context->get_default_framebuffer());
	}
}