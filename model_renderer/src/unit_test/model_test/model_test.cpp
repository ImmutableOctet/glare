#include "model_test.hpp"

#include <iostream>
#include <string>
#include <cmath>

#include <app/events.hpp>
#include <app/input/keyboard/keycodes.hpp>
#include <engine/engine.hpp>
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

		*loaded_model = graphics::Model::Load(graphics.context, "assets/unit_tests/model_test/sphere.obj");

		model_entity = engine::create_pivot(world);

		auto model_transform = world.get_transform(model_entity);

		model_transform.move({3.0f, 10.0f, -30.0f});

		input.get_mouse().lock();

		if (auto_execute)
		{
			execute();
		}
	}

	void ModelTest::update()
	{
		auto& mouse = input.get_mouse();

		if (mouse.locked())
		{
			input.poll(world.get_event_handler());
		}

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

		auto camera_transform = world.get_transform(camera);

		auto& camera_params = registry.get<engine::CameraParameters>(camera);

		camera_params.aspect_ratio = window->horizontal_aspect_ratio();

		auto rotation = glm::degrees(camera_transform.get_rotation());
		
		//std::cout << rotation.x << ", " << rotation.y << ", " << rotation.z << '\r';

		math::Matrix inverse_world = camera_transform.get_inverse_matrix();

		graphics.context->use(*test_shader, [&]()
		{
			this->uniforms.projection = glm::perspective(camera_params.fov, camera_params.aspect_ratio, camera_params.near, camera_params.far);
			this->uniforms.view = inverse_world; // glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -1.0f));

			graphics.context->update(*test_shader, uniforms.projection);
			graphics.context->update(*test_shader, uniforms.view);

			auto model_transform = world.get_transform(model_entity);

			//model_transform.move({0.0f, (0.2f * std::sin(milliseconds() / 100)), 0.0f});

			auto model_matrix = model_transform.get_matrix();

			this->uniforms.model = model_matrix;

			graphics.context->update(*test_shader, uniforms.model);

			graphics.canvas->draw(*loaded_model, model_matrix);

			/*
			for (auto& m : loaded_model->get_meshes())
			{
				auto& mesh = *m.first;

				graphics.context->use(mesh, [&]()
				{
					graphics.context->draw();
				});
			}
			*/
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

	void ModelTest::on_keydown(const keyboard_event_t& event)
	{
		switch (event.keysym.sym)
		{
			case SDLK_0:
				auto camera_transform = world.get_transform(camera);

				auto model = this->uniforms.model.get_value();
				auto v = math::get_translation(model);

				std::cout << v.x << ", " << v.y << ", " << v.z << '\n';

				camera_transform.look_at(v);

				break;
		}
	}
}