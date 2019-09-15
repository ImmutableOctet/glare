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

		texture1 = graphics::Texture(graphics.context, "assets/unit_tests/shader_test/texture1.png");
		texture2 = graphics::Texture(graphics.context, "assets/unit_tests/shader_test/texture2.jpg");

		uniforms = {};

		/*
		cube = graphics::Mesh::Generate
		(
			graphics.context,
			graphics::Mesh::Data<graphics::TexturedVertex>
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
		*/

		quad = graphics::Mesh::GenerateTexturedQuad(graphics.context);

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

		auto& shader = *test_shader;

		//std::sinf(milliseconds())
		graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, graphics::BufferType::Color|graphics::BufferType::Depth); // gfx

		graphics.context->use(shader, [&, this]()
		{
			graphics.context->set_uniform(shader, "texture1", 0);
			graphics.context->set_uniform(shader, "texture2", 1);

			graphics.context->use(texture1, [&, this]()
			{
				graphics.context->use(texture2, [&, this]()
				{
					graphics.context->use(quad, [&, this]()
					{
						uniforms.projection = glm::perspective(glm::radians(45.0f), window->horizontal_aspect_ratio(), 0.1f, 100.0f);
						uniforms.view = glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -3.0f));

						uniforms.model = glm::mat4(1.0f);
						uniforms.model = glm::translate(uniforms.model.get_value(), math::vec3(0.0, 0.0, -2));

						float angle = (3.0f) * (milliseconds() / 16);

						uniforms.model = glm::rotate(uniforms.model.get_value(), glm::radians(angle), glm::vec3(0.45f, 1.0f, 0.45f));

						//color = { std::sin(milliseconds()), 0.5, 0.5 };

						graphics.context->update(shader, uniforms.projection);
						graphics.context->update(shader, uniforms.view);
						graphics.context->update(shader, uniforms.model);

						graphics.context->draw();
					});
				});
			});
		});

		gfx.flip(wnd);
	}
}