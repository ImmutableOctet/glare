#include "shader_test.hpp"

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
	ShaderTest::ShaderTest(bool auto_execute)
		: GraphicsApplication("Shader Test", 1600, 900)
	{
		test_shader = memory::allocate<graphics::Shader>
		(
			graphics.context,
			
			"assets/unit_tests/shader_test/test.vs",
			"assets/unit_tests/shader_test/test.fs"
		);
				
		texture1 = memory::allocate<graphics::Texture>(graphics.context, "assets/unit_tests/shader_test/texture1.png");
		texture2 = memory::allocate<graphics::Texture>(graphics.context, "assets/unit_tests/shader_test/texture2.jpg");

		texture1_sampler = Var<int>("texture1", test_shader, 0);
		texture2_sampler = Var<int>("texture2", test_shader, 1);

		projection = Var<math::mat4>("projection", test_shader, {});
		view       = Var<math::mat4>("view",       test_shader, {});
		model      = Var<math::mat4>("model",      test_shader, {});

		//color = {"color", test_shader, {}};

		cube = graphics::Mesh::Generate
		(
			graphics.context,
			graphics::Mesh::Data<graphics::TextureVertex>
			{
				{
					{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 0.0f}},
					{{{ 0.5f, -0.5f, -0.5f}},  {1.0f, 0.0f}},
					{{{ 0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
					{{{ 0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
					{{{-0.5f,  0.5f, -0.5f}},  {0.0f, 1.0f}},
					{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 0.0f}},

					{{{-0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},
					{{{ 0.5f, -0.5f,  0.5f}},  {1.0f, 0.0f}},
					{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 1.0f}},
					{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 1.0f}},
					{{{-0.5f,  0.5f,  0.5f}},  {0.0f, 1.0f}},
					{{{-0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},

					{{{-0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},
					{{{-0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
					{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
					{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
					{{{-0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},
					{{{-0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},

					{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},
					{{{ 0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
					{{{ 0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
					{{{ 0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
					{{{ 0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},
					{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},

					{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
					{{{ 0.5f, -0.5f, -0.5f}},  {1.0f, 1.0f}},
					{{{ 0.5f, -0.5f,  0.5f}},  {1.0f, 0.0f}},
					{{{ 0.5f, -0.5f,  0.5f}},  {1.0f, 0.0f}},
					{{{-0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},
					{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},

					{{{-0.5f,  0.5f, -0.5f}},  {0.0f, 1.0f}},
					{{{ 0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
					{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},
					{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},
					{{{-0.5f,  0.5f,  0.5f}},  {0.0f, 0.0f}},
					{{{-0.5f,  0.5f, -0.5f}},  {0.0f, 1.0f}}
				},
						
				// Raw vertices; no index data.
				{}
			}
		);

		if (auto_execute)
		{
			execute();
		}
	}

	void ShaderTest::update()
	{
				
	}

	void ShaderTest::render()
	{
		auto& gfx = *graphics.canvas;
		auto& wnd = *window;

		glEnable(GL_DEPTH_TEST);

		//std::sinf(milliseconds())
		graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, graphics::BufferType::Color|graphics::BufferType::Depth); // gfx

		graphics.context->use(*texture1, [this]()
		{
			graphics.context->use(*texture2, [this]()
			{
				graphics.context->use(*test_shader, [this]()
				{
					graphics.context->use(cube, [this]()
					{
						texture1_sampler.upload();
						texture2_sampler.upload();

						int width, height;

						SDL_GetWindowSize(window->get_handle(), &width, &height);

						projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
						view = glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -3.0f));

						glm::mat4 model = glm::mat4(1.0f);
						model = glm::translate(model, math::vec3(0.0, 0.0, -0.1));

						float angle = 20.0f * (milliseconds() / 32);

						model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
						this->model = model;

						//color = { std::sin(milliseconds()), 0.5, 0.5 };

						graphics.context->draw();
					});
				});
			});
		});

		gfx.flip(wnd);
	}
}