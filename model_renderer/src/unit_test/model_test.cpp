#include "model_test.hpp"

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
	ModelTest::ModelTest(bool auto_execute)
		: GraphicsApplication("Shader Test", 1600, 900)
	{
		test_shader = memory::allocate<graphics::Shader>
		(
			graphics.context,
			
			"assets/unit_tests/model_test/test.vs",
			"assets/unit_tests/model_test/test.fs"
		);

		projection = Var<math::mat4>("projection", test_shader, {});
		view       = Var<math::mat4>("view",       test_shader, {});
		model      = Var<math::mat4>("model",      test_shader, {});

		//color = {"color", test_shader, {}};

		loaded_model = memory::allocate<graphics::Model>();

		*loaded_model = graphics::Model::Load(graphics.context, "assets/unit_tests/model_test/sphere.obj");

		if (auto_execute)
		{
			execute();
		}
	}

	void ModelTest::update()
	{
				
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
			projection = glm::perspective(glm::radians(45.0f), window->horizontal_aspect_ratio(), 0.1f, 100.0f);
			view = glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -10.0f));

			//graphics.canvas.draw(*loaded_model);

			for (auto& m : loaded_model->get_meshes())
			{
				auto& mesh = *m.first;

				graphics.context->use(mesh, [this]()
				{
					glm::mat4 model = glm::mat4(1.0f);
					model = glm::translate(model, math::vec3(0.0, 0.0, -2));

					float angle = (3.0f) * (milliseconds() / 16);

					model = glm::rotate(model, glm::radians(angle), glm::vec3(0.45f, 1.0f, 0.45f));
					this->model = model;

					//color = { std::sin(milliseconds()), 0.5, 0.5 };

					graphics.context->draw();
				});
			}
		});

		gfx.flip(wnd);
	}
}