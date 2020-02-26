#include "model_test.hpp"

#include <iostream>
#include <string>
#include <cmath>

#include <graphics/native/opengl.hpp>

#include <sdl2/SDL_video.h>
#include <sdl2/SDL_hints.h>
#include <sdl2/SDL_mouse.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace unit_test
{
	ModelTest::ModelTest(bool auto_execute)
		: GraphicsApplication("Shader Test", 1600, 900, app::WindowFlags::OpenGL)
	{
		test_shader = memory::allocate<graphics::Shader>
		(
			graphics.context,
			
			"assets/unit_tests/model_test/test.vs",
			"assets/unit_tests/model_test/test.fs"
		);

		loaded_model = memory::allocate<graphics::Model>();

		*loaded_model = graphics::Model::Load(graphics.context, "assets/unit_tests/model_test/checkpoint/checkpoint.b3d"); // "stage.obj"

		SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
		SDL_SetRelativeMouseMode(SDL_TRUE);

		if (auto_execute)
		{
			execute();
		}
	}

	void ModelTest::update()
	{
		int x, y;

		auto buttons = SDL_GetRelativeMouseState(&x, &y);

		std::cout << x << ", " << y << '\n';
	}

	void ModelTest::render()
	{
		auto& gfx = *graphics.canvas;
		auto& wnd = *window;

		//std::sinf(milliseconds())
		graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, graphics::BufferType::Color|graphics::BufferType::Depth); // gfx

		//graphics.canvas.draw();

		//loaded_model.draw();

		graphics.context->use(*test_shader, [this]()
		{
			this->uniforms.projection = glm::perspective(glm::radians(75.0f), window->horizontal_aspect_ratio(), 0.1f, 1000.0f);
			this->uniforms.view = glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -1.0f));

			//graphics.canvas.draw(*loaded_model);

			for (auto& m : loaded_model->get_meshes())
			{
				auto& mesh = *m.first;

				graphics.context->use(mesh, [this]()
				{
					glm::mat4 model = glm::mat4(1.0f);
					model = glm::translate(model, math::vec3(0.0, 0.0, -40.0));

					float angle = 45.0f; // (3.0f)* (milliseconds() / 16);

					//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.1f, 0.0f));

					//model = glm::scale(model, {0.1, 0.1, 0.1});
					
					this->uniforms.model = model;

					graphics.context->update(*test_shader, uniforms.projection);
					graphics.context->update(*test_shader, uniforms.view);
					graphics.context->update(*test_shader, uniforms.model);

					//color = { std::sin(milliseconds()), 0.5, 0.5 };

					graphics.context->draw();
				});
			}
		});

		gfx.flip(wnd);
	}
}