#include "model_test.hpp"

#include <iostream>
#include <string>
#include <cmath>

#include <app/events.hpp>
#include <app/input/keyboard/keycodes.hpp>
#include <engine/engine.hpp>
#include <graphics/native/opengl.hpp>

#include <engine/free_look.hpp>

#include <sdl2/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace unit_test
{
	ModelTest::ModelTest(bool auto_execute)
		: GraphicsApplication("Model Test", 1600, 900, app::WindowFlags::OpenGL, 60, true), world(delta_time)
	{
		test_shader = memory::allocate<graphics::Shader>
		(
			graphics.context,
			
			"assets/unit_tests/model_test/test.vs",
			"assets/unit_tests/model_test/test.fs"
		);

		input.get_mouse().lock();

		setup_world(world);

		if (auto_execute)
		{
			execute();
		}
	}

	void ModelTest::setup_world(engine::World& world)
	{
		loaded_model = memory::allocate<graphics::Model>();

		std::string path;

		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\Genesis Redux Project 1.1\\Stages\\GreenHill\\Stage\\Stage.b3d";
		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic-backups\\BlitzSonicv02\\Stages\\GreenHill\\Stage\\Stage.b3d";
		path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\HAXY EDIT OF DOOM\\Stages\\Emerald Coast\\1\\EC1.b3d";
		//path = "model_renderer/assets/unit_tests/model_test/Bianco/Stage2.b3d";

		//path = "assets\\unit_tests\\model_test\\Checkpoint\\Checkpoint.b3d";

		//path = "assets/unit_tests/model_test/custom_sonic.b3d";
		//path = "assets/unit_tests/model_test/Stage.obj";
		//path = "assets/unit_tests/model_test/Young Samus/rtm_zero_samus_old.obj";
		//path = "assets\\unit_tests\\deferred_test\\nanosuit\\nanosuit.obj";
		//path = "assets/unit_tests/model_test/SeasideShortcut/Stage.obj";
		//path = "assets/unit_tests/model_test/sphere.obj";
		//path = "assets/unit_tests/model_test/Sonic.obj";
		//path = "assets/unit_tests/model_test/Sonic/Sonic.obj";

		*loaded_model = graphics::Model::Load(graphics.context, resource_manager, path, test_shader);

		model_entity = engine::create_model(world, loaded_model);

		auto debug_camera = engine::debug::create_debug_camera(world, engine::CameraParameters::DEFAULT_FOV);

		auto transform = world.get_transform(debug_camera);

		transform.move({ 0.0f, 0.0f, -30.0f });
	}

	void ModelTest::update(const app::DeltaTime& delta_time)
	{
		auto& mouse = input.get_mouse();

		if (mouse.locked())
		{
			input.poll(world.get_event_handler());
		}

		world.update();
	}

	void ModelTest::render()
	{
		auto& gfx = *graphics.canvas;
		auto& wnd = *window;

		//std::sinf(milliseconds())
		graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, graphics::BufferType::Color|graphics::BufferType::Depth); // gfx

		auto& shader = *test_shader;

		graphics.context->use(shader, [&]()
		{
			world.render(*graphics.canvas, true);
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
			case SDLK_TAB:
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
			case SDLK_1:
			{
				auto model_transform = world.get_transform(model_entity);
				model_transform.rotateX(45.0f * (3.14159f / 180.0f));

				break;
			}
			case SDLK_2:
			{
				auto model_transform = world.get_transform(model_entity);
				model_transform.rotateY(45.0f * (3.14159f / 180.0f));

				break;
			}
			case SDLK_3:
			{
				auto model_transform = world.get_transform(model_entity);
				model_transform.rotateZ(45.0f * (3.14159f / 180.0f));

				break;
			}
			case SDLK_4:
			{
				auto model_transform = world.get_transform(model_entity);
				model_transform.set_rotation({0.0f, 0.0f, 0.0f});

				break;
			}
		}
	}
}