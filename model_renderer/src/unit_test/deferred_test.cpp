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
		: GraphicsApplication("Shader Test", 1600, 900)
	{
		test_shader = memory::allocate<graphics::Shader>(graphics.context, "assets/unit_tests/shader_test/test.vs", "assets/unit_tests/shader_test/test.fs");

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

		auto& shader = *test_shader;

		int width, height;

		window->get_size(width, height);

		graphics.context->set_viewport(0, 0, width, height);

		graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, graphics::BufferType::Color | graphics::BufferType::Depth); // gfx

		gfx.flip(wnd);
	}
}