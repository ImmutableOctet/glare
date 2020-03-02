#include "model_test.hpp"

#include <iostream>
#include <string>
#include <cmath>

#include <app/events.hpp>
#include <app/input/keyboard/keycodes.hpp>
#include <graphics/native/opengl.hpp>

#include <sdl2/SDL_video.h>

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

		*loaded_model = graphics::Model::Load(graphics.context, "assets/unit_tests/model_test/sphere.obj"); // "checkpoint/checkpoint.b3d"

		input.get_mouse().lock();

		if (auto_execute)
		{
			execute();
		}
	}

	void ModelTest::update()
	{
		input.poll(world.get_event_handler());

		world.update(1.0f);
	}

	void ModelTest::render()
	{
		auto& gfx = *graphics.canvas;
		auto& wnd = *window;

		//std::sinf(milliseconds())
		graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, graphics::BufferType::Color|graphics::BufferType::Depth); // gfx

		//graphics.canvas.draw();

		//loaded_model.draw();

		auto& registry = world.get_registry();

		//auto& camera_transform = registry.get<engine::TransformComponent>(camera);
		//auto inverse_world = camera_transform.inverse_world;

		auto camera_local_transform = world.get_transform(camera);

		//camera_local_transform.set_scale({0.5f, 1.0f, 0.5f});
		//camera_local_transform.set_position({0.0, 5.0, -30.0});

		math::Matrix inverse_world = glm::inverse(camera_local_transform.get_matrix());

		graphics.context->use(*test_shader, [this, inverse_world]()
		{
			this->uniforms.projection = glm::perspective(glm::radians(75.0f), window->horizontal_aspect_ratio(), 0.1f, 1000.0f);
			this->uniforms.view = inverse_world; // glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -1.0f));

			graphics.context->update(*test_shader, uniforms.projection);
			graphics.context->update(*test_shader, uniforms.view);

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

					graphics.context->update(*test_shader, uniforms.model);

					//color = { std::sin(milliseconds()), 0.5, 0.5 };

					graphics.context->draw();
				});
			}
		});

		gfx.flip(wnd);
	}

	void ModelTest::on_keyup(const keyboard_event_t& event)
	{
		switch (event.keysym.sym)
		{
			case SDLK_ESCAPE:
				stop();

				break;
			case SDLK_SPACE:
			{
				auto& mouse = input.get_mouse();

				mouse.toggle_lock();

				break;
			}
		}
	}
}