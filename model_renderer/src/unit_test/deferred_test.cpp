#include "deferred_test.hpp"

#include <iostream>
#include <string>
#include <cmath>

#include <graphics/native/opengl.hpp>

#include <sdl2/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace unit_test
{
	DeferredTest::DeferredTest(bool auto_execute)
		: GraphicsApplication("Deferred Shading", 1600, 900)
	{
		static const std::string asset_folder = "assets/unit_tests/deferred_test/";

		shaders.geometry_pass = graphics::Shader(graphics.context, (asset_folder + "g_buffer.vs"), (asset_folder + "g_buffer.fs"));
		shaders.lighting_pass = graphics::Shader(graphics.context, (asset_folder + "deferred_shading.vs"), (asset_folder + "deferred_shading.fs"));
		shaders.light_box     = graphics::Shader(graphics.context, (asset_folder + "deferred_light_box.vs"), (asset_folder + "deferred_light_box.fs"));

		screen_quad = graphics::Mesh::GenerateTexturedQuad(graphics.context);

		gBuffer = graphics::FrameBuffer(graphics.context);

		graphics.context->use(gBuffer, []()
		{
			//gBuffer.attach();
		});

		model = memory::allocate<graphics::Model>();
		
		*model = std::move(graphics::Model::Load(graphics.context, asset_folder + "nanosuit/nanosuit.obj"));

		if (auto_execute)
		{
			execute();
		}
	}

	void DeferredTest::update()
	{

	}

	void DeferredTest::render()
	{
		auto& gfx = *graphics.canvas;
		auto& wnd = *window;

		int width, height;

		window->get_size(width, height);

		graphics.context->set_viewport(0, 0, width, height);

		graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, graphics::BufferType::Color | graphics::BufferType::Depth); // gfx

		graphics.context->flip(wnd);
	}
}