#include "shader_test.hpp"

#include <iostream>
#include <string>
#include <cmath>

#include <graphics/native/opengl.hpp>

#include <sdl2/SDL_video.h>
#include <sdl2/SDL_events.h>

#include <sdl2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace unit_test
{
	ShaderTest::ShaderTest(bool auto_execute)
		: GraphicsApplication("Shader Test", 1600, 900)
	{
		std::cout << "Generating test shader...\n";

		key_state = reinterpret_cast<decltype(key_state)>(SDL_GetKeyboardState(nullptr));

		test_shader = memory::allocate<graphics::Shader>
		(
			graphics.context,
			
			"assets/unit_tests/shader_test/test.vs",
			"assets/unit_tests/shader_test/test.fs"
		);

		std::cout << "Loading textures...\n";

		texture1 = graphics::Texture(graphics.context, "assets/unit_tests/shader_test/texture1.png");
		texture2 = graphics::Texture(graphics.context, "assets/unit_tests/shader_test/texture2.jpg");

		uniforms = {};

		std::cout << "Generating test mesh...\n";

		quad = graphics::Mesh::Generate
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
			},
			
			graphics::Primitive::Triangle
		);

		//quad = graphics::Mesh::GenerateTexturedQuad(graphics.context);

		//rotation = glm::quat_cast(glm::mat4());

		//graphics.context;

		if (auto_execute)
		{
			execute();
		}
	}

	void ShaderTest::update()
	{
		if (key_state[SDL_SCANCODE_SPACE])
		{
			//std::cout << milliseconds() << '\n';

			
		}
	}

	void ShaderTest::on_keydown(const keyboard_event_t& event)
	{
		//std::cout << "KeyDown Event: " << event.keysym.sym << '\n';
	}

	void ShaderTest::on_keyup(const keyboard_event_t& event)
	{
		std::cout << "KeyUp Event: " << event.keysym.sym << '\n';

		switch (event.keysym.sym)
		{
			case SDLK_ESCAPE:
				stop();

				break;
		}
	}

	void ShaderTest::render()
	{
		auto& gfx = *graphics.canvas;
		auto& wnd = *window;

		auto& shader = *test_shader;

		int width, height;

		window->get_size(width, height);

		auto& ctx = graphics.context;

		ctx->set_viewport(0, 0, width, height);

		//std::sinf(milliseconds())
		ctx->clear(0.1f, 0.33f, 0.25f, 1.0f, (graphics::BufferType::Color|graphics::BufferType::Depth)); // gfx

		ctx->use(shader, [&, this]()
		{
			ctx->set_uniform(shader, "texture1", 0);
			ctx->set_uniform(shader, "texture2", 1);

			ctx->use(texture1, [&, this]()
			{
				ctx->use(texture2, [&, this]()
				{
					ctx->use(quad, [&, this]()
					{
						uniforms.projection = glm::perspective(glm::radians(45.0f), window->horizontal_aspect_ratio(), 0.1f, 100.0f);
						uniforms.view = glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -4.0f));

						auto model = glm::mat4(1.0f);
						model = glm::translate(model, math::vec3(0.0, 0.0, -2));

						//float angle = (3.0f) * (milliseconds() / 16);

						//model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

						//model *= glm::mat4_cast(rotation);

						//color = { std::sin(milliseconds()), 0.5, 0.5 };

						uniforms.model = model;

						ctx->update(shader, uniforms.projection);
						ctx->update(shader, uniforms.view);
						ctx->update(shader, uniforms.model);

						ctx->draw();
					});
				});
			});
		});

		gfx.flip(wnd);
	}
}