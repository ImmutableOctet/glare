#include "model_test.hpp"

#include <iostream>
#include <string>
#include <cmath>

#include <app/events.hpp>
#include <app/input/keyboard/keycodes.hpp>
#include <engine/engine.hpp>
#include <graphics/native/opengl.hpp>

#include <engine/world/debug/spin_component.hpp>

#include <engine/free_look.hpp>

#include <sdl2/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace unit_test
{
	ModelTest::ModelTest(bool auto_execute)
		: GraphicsApplication("Model Test", 1600, 900, app::WindowFlags::OpenGL, 60, false), world(delta_time)
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

	engine::Entity ModelTest::load_model(const std::string& path)
	{
		ref<graphics::Model> loaded_model;

		loaded_model = memory::allocate<graphics::Model>();
		*loaded_model = graphics::Model::Load(graphics.context, resource_manager, path, test_shader);

		return engine::create_model(world, loaded_model);
	}

	engine::Transform ModelTest::model(const std::string& path)
	{
		auto entity = load_model(path);

		return transform(entity);
	}

	engine::Transform ModelTest::transform(engine::Entity entity)
	{
		return world.get_transform(entity);
	}

	void ModelTest::make_camera(engine::World& world)
	{
		auto debug_camera = engine::debug::create_debug_camera(world, engine::CameraParameters::DEFAULT_FOV);
		transform(debug_camera).move({ 20.0f, 40.0f, -30.0f });
	}

	void ModelTest::make_lights(engine::World& world)
	{
		auto light = engine::create_light(world);
	}

	void ModelTest::make_models(engine::World& world)
	{
		//auto nanosuit = model("assets\\unit_tests\\deferred_test\\nanosuit\\nanosuit.obj");
		//nanosuit.move({ 14.0f, 10.0f, 30.0f });

		auto sphere = model("assets/unit_tests/model_test/Sonic/Sonic.b3d"); // model("assets/unit_tests/model_test/sphere.obj");
		sphere.move({ 0.0f, 10.0f, -10.0f });

		auto cube = load_model("assets/unit_tests/model_test/cube.b3d");
		world.get_registry().assign<engine::debug::SpinBehavior>(cube);

		transform(cube).move({ 45.0f, 10.0f, 39.0f });

		//auto geometry = model("assets/unit_tests/model_test/geometry.obj");
		//geometry.move({ 50.0f, 31.0f, 28.0f });
	}

	void ModelTest::setup_world(engine::World& world)
	{
		std::string path;

		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\Genesis Redux Project 1.1\\Stages\\GreenHill\\Stage\\Stage.b3d";
		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic-backups\\BlitzSonicv02\\Stages\\GreenHill\\Stage\\Stage.b3d";
		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\HAXY EDIT OF DOOM\\Stages\\Chao World\\Stage\\Stage.b3d";
		//path = "model_renderer/assets/unit_tests/model_test/Bianco/Stage2.b3d";
		//path = "assets/unit_tests/model_test/SeasideShortcut/Stage.obj";
		path = "assets/unit_tests/model_test/stage.b3d";
		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\HAXY EDIT OF DOOM\\Stages\\Emerald Coast\\1\\EC1.b3d";

		auto stage = load_model(path);

		make_models(world);
		make_camera(world);
		make_lights(world);
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
			// ...
		}
	}
}