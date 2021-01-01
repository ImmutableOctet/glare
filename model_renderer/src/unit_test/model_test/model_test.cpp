#include "model_test.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <cmath>
#include <cstdlib>

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
		: GraphicsApplication("Model Test", 1600, 900, (app::WindowFlags::OpenGL|app::WindowFlags::Resizable), 60, true),
		  world(delta_time)
	{
		using namespace graphics;

		/*
		glDebugMessageCallback([]
		(
			GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam)
		{
			std::cout << "OpenGL (" << severity << "):" << '\n';
			std::cout << message << '\n';
		}, this);
		*/

		shaders.forward = memory::allocate<Shader>
		(
			graphics.context,
			
			"assets/unit_tests/model_test/shaders/forward.vs",
			"assets/unit_tests/model_test/shaders/forward.fs"
		);

		shaders.forward_test = memory::allocate<Shader>
		(
			graphics.context,
			
			//"assets/unit_tests/model_test/shaders/forward.vs",
			"assets/unit_tests/model_test/shaders/8.1.deferred_shading.vs",
			"assets/unit_tests/model_test/shaders/forward_test.fs"
		);

		shaders.geometry = memory::allocate<Shader>
		(
			graphics.context,

			"assets/unit_tests/model_test/shaders/8.1.g_buffer.vs",
			"assets/unit_tests/model_test/shaders/8.1.g_buffer.fs"
		);

		shaders.lighting_pass = memory::allocate<Shader>
		(
			graphics.context,

			"assets/unit_tests/model_test/shaders/8.1.deferred_shading.vs",
			"assets/unit_tests/model_test/shaders/8.1.deferred_shading.fs"
		);

		shaders.framebuffer_dbg = memory::allocate<Shader>
		(
			graphics.context,

			"assets/unit_tests/model_test/shaders/8.1.fbo_debug.vs",
			"assets/unit_tests/model_test/shaders/8.1.fbo_debug.fs"
		);

		/*
		shaders.light_box = memory::allocate<Shader>
		(
			graphics.context,

			"assets/unit_tests/model_test/shaders/8.1.deferred_light_box.vs",
			"assets/unit_tests/model_test/shaders/8.1.deferred_light_box.fs"
		);
		*/

		int screen_width, screen_height;

		window->get_size(screen_width, screen_height);

		gBuffer.screen_quad = Mesh::GenerateTexturedQuad(graphics.context);

		const auto gBuffer_flags = TextureFlags::None;

		//test_texture = Texture(graphics.context, "assets/unit_tests/model_test/Bianco/T (1).png");

		gBuffer.framebuffer = FrameBuffer(graphics.context);

		auto& fb = gBuffer.framebuffer;

		graphics.context->use(fb, [&, this]()
		{
			gBuffer.position = Texture(graphics.context, screen_width, screen_height, TextureFormat::RGB, ElementType::Half, gBuffer_flags); // Float
			//auto _pos = graphics.context->use(gBuffer.position);
			fb.attach(gBuffer.position);
			
			gBuffer.normal = Texture(graphics.context, screen_width, screen_height, TextureFormat::RGB, ElementType::Half, gBuffer_flags); // Float
			fb.attach(gBuffer.normal);

			gBuffer.albedo_specular = Texture(graphics.context, screen_width, screen_height, TextureFormat::RGBA, ElementType::UByte, gBuffer_flags);
			fb.attach(gBuffer.albedo_specular);

			fb.link();

			// TODO: Refactor
			fb.attach(RenderBufferType::Depth, screen_width, screen_height);

			//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			//	std::cout << "Framebuffer not complete!" << std::endl;
		});

		graphics.context->clear_textures(true);

		//auto op = graphics.context->use(graphics.context->get_default_framebuffer());

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
		*loaded_model = graphics::Model::Load(graphics.context, resource_manager, path, shaders.forward);

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

	engine::Entity ModelTest::make_camera(engine::World& world)
	{
		auto debug_camera = engine::debug::create_debug_camera(world, engine::CameraParameters::DEFAULT_FOV);

		auto t = transform(debug_camera);

		//t.set_position({ 1.14704f, 3.97591f, -8.47185f });
		//t.set_rotation(math::radians(math::Vector{ -27.0993f, -175.95f, 0.0f }));

		//t.set_position({ 16.3587f, 48.3307f, -1.65829f });
		//t.set_rotation(math::radians(math::Vector { -7.54972f, 3.50007f, 0.0f }));

		t.set_position({ 10.0781f, 59.5706f, 4.06396f });
		t.set_rotation(math::radians(math::Vector{ -8.19875f, 1.60009f, 0.0f }));


		//t.rotateY(math::radians(180.0f));

		return debug_camera;
	}

	void ModelTest::make_lights(engine::World& world)
	{
		using namespace std;

		math::Vector offset = { 0.0f, 0.0f, 0.0f }; // { 0.0f, 64.0f, 0.0f };

		srand(13); // 100

		for (unsigned int i = 0; i < NR_LIGHTS; i++)
		{
			math::Vector position =
			{
				((static_cast<float>(rand() % 100) / 100.0f) * 60.0f - 30.0f), // 60.0f - 30.0f
				((static_cast<float>(rand() % 100) / 100.0f) * 60.0f - 40.0f),
				((static_cast<float>(rand() % 100) / 100.0f) * 60.0f - 30.0f)
			};

			// also calculate random color
			graphics::ColorRGB color =
			{
				1.0f, // (static_cast<float>(rand() % 100) / 200.0f) + 0.5f, // between 0.5 and 1.0
				1.0f, // (static_cast<float>(rand() % 100) / 200.0f) + 0.5f, // between 0.5 and 1.0
				1.0f, // (static_cast<float>(rand() % 100) / 200.0f) + 0.5f // between 0.5 and 1.0
			};

			engine::create_light(world, (position + offset), color);
		}
	}

	void ModelTest::make_models(engine::World& world, engine::Entity camera)
	{
		//return;

		auto& registry = world.get_registry();

		//auto nanosuit = load_model("assets/unit_tests/model_test/Sonic/Sonic.b3d"); // assets/characters/nanosuit/nanosuit.obj // b3d
		auto nanosuit = load_model("assets/characters/nanosuit/nanosuit.obj");
		auto nanosuit_t = transform(nanosuit);

		registry.assign<engine::NameComponent>(nanosuit, "Player");

		//world.set_parent(camera, nanosuit);

		//auto nanosuit = model("assets/unit_tests/model_test/cube_textured.b3d");
		nanosuit_t.move({ 10.7f, 46.0f, -13.0f });
		//nanosuit_t.set_scale(math::Vector {1.0, 1.0, 1.0} * 10.0f);

		auto sphere = load_model("assets/unit_tests/model_test/Sonic/Sonic.b3d"); // model("assets/unit_tests/model_test/sphere.obj");
		auto sphere_t = transform(sphere);

		sphere_t.move({ 14.0f, 10.0f, 30.0f });

		//world.set_parent(sphere, nanosuit);

		///return;

		auto cube_model = memory::allocate<graphics::Model>();
		*cube_model = graphics::Model::Load(graphics.context, resource_manager, "assets/unit_tests/model_test/cube.b3d", shaders.forward);

		auto cube = engine::create_model(world, cube_model, nanosuit);
		//auto cube = load_model("assets/unit_tests/model_test/cube.b3d");

		auto cube_rel = registry.get<engine::Relationship>(cube);
		auto nanosuit_rel = registry.get<engine::Relationship>(nanosuit);

		registry.assign<engine::debug::SpinBehavior>(cube);
		transform(cube).set_position({ 25.0f, 46.0f, -20.0f });

		// Light boxes:
		/*
		registry.view<engine::LightComponent>().each([&](auto light, const auto& light_component)
		{
			attach_model(world, light, cube_model, { light_component.color, 1.0f });
		});
		*/

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
		//path = "assets/unit_tests/model_test/stage.b3d";
		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\HAXY EDIT OF DOOM\\Stages\\Emerald Coast\\1\\EC1.b3d";
		path = "assets/maps/test01/stage.b3d";

		auto stage = load_model(path);

		auto camera = make_camera(world);
		make_lights(world);
		make_models(world, camera);

		world.register_event<app::input::KeyboardState, &ModelTest::on_user_keyboard_input>(*this);
	}

	engine::Transform ModelTest::get_named_transform(std::string_view name)
	{
		auto entity = world.get_by_name(name);
		auto transform = world.get_transform(entity);

		return transform;
	}

	void ModelTest::on_user_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		std::string_view target_obj = "Player";

		auto camera_t = transform(world.get_camera());
		auto camera_pos = camera_t.get_position();

		auto transform = get_named_transform(target_obj);

		auto angle = glm::normalize(camera_pos - transform.get_position());

		float delta = delta_time;

		if (keyboard.get_key(SDL_SCANCODE_Q))
		{
			
		}

		if (keyboard.get_key(SDL_SCANCODE_E))
		{
			
		}

		if (keyboard.get_key(SDL_SCANCODE_2))
		{
			transform.move(delta * 0.1f * angle, true);
		}

		if (keyboard.get_key(SDL_SCANCODE_Z))
		{
			transform.move(delta * 0.1f * angle * -1.0f, true);
		}

		if (keyboard.get_key(SDL_SCANCODE_R))
		{
			auto transform = get_named_transform("Player");

			transform.rotateY(math::radians(10.0f * delta_time));
		}
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
		using namespace graphics;

		auto& gfx = *graphics.canvas;
		auto& wnd = *window;

		//std::sinf(milliseconds())

		// Backbuffer clear; for gbuffer, see below.

		//graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, BufferType::Color|BufferType::Depth); // gfx
		graphics.context->clear(0.0f, 0.0f, 0.0f, 1.0f, BufferType::Color | BufferType::Depth);
		//graphics.context->clear(1.0f, 1.0f, 1.0f, 1.0f, BufferType::Color | BufferType::Depth);

		auto camera = world.get_camera();

		if (camera == engine::null)
		{
			return;
		}

		auto camera_transform = world.get_transform(camera);

		auto viewport = update_viewport(camera);

		// Forward rendering:
		/*
		{
			auto& shader = *shaders.forward;

			graphics.context->use(shader, [&]()
			{
				//graphics.context->use(gBuffer.screen_quad, [&, this]()
				//{
				//	graphics.context->draw();
				//});

				world.render(*graphics.canvas, true);

				//graphics.context->use(gBuffer.screen_quad, [&, this]()
				//{
				//	auto gAlbedoSpec = graphics.context->use(gBuffer.albedo_specular, "diffuse");
				//
				//	graphics.context->draw();
				//});
			});
		}

		gfx.flip(wnd);

		return;

		*/

		///*
		{
			// Geometry pass:
			{
				auto& shader = *shaders.geometry;

				//graphics.context->clear_textures(false); // true // <-- May not be needed.

				graphics.context->use(gBuffer.framebuffer, [&, this]()
				{
					graphics.context->clear(0.0f, 0.0f, 0.0f, 1.0f, (graphics::BufferType::Color | graphics::BufferType::Depth)); // gfx // 1.0f, 0.0f, 0.0f
					//graphics.context->clear(1.0f, 1.0f, 1.0f, 1.0f, (graphics::BufferType::Color | graphics::BufferType::Depth)); // gfx // 1.0f, 0.0f, 0.0f

					graphics.context->use(shader, [&, this]()
					{
						world.render(*graphics.canvas, false); // , true

						/*
						auto texture = graphics.context->use(test_texture);

						graphics.context->use(gBuffer.screen_quad, [&, this]()
						{
							graphics.context->draw();
						});
						*/
					});
				});
			}

			graphics.context->clear(0.0f, 0.0f, 0.0f, 1.0f, BufferType::Color | BufferType::Depth); // gfx
			//graphics.context->clear(1.0f, 1.0f, 1.0f, 1.0f, BufferType::Color | BufferType::Depth); // gfx

			// Lighting pass:
			{
				auto& shader = *shaders.lighting_pass;

				graphics.context->use(shader, [&, this]()
				{
					graphics.context->clear_textures(true); // false

					auto gPosition   = graphics.context->use(gBuffer.position, "gPosition");
					auto gNormal     = graphics.context->use(gBuffer.normal, "gNormal");
					auto gAlbedoSpec = graphics.context->use(gBuffer.albedo_specular, "gAlbedoSpec");

					auto& registry = world.get_registry();

					// update attenuation parameters and calculate radius
					const float constant  = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
					const float linear    = 0.7;
					const float quadratic = 1.8;

					unsigned int light_idx = 0;

					registry.view<engine::LightComponent, engine::TransformComponent, engine::Relationship>().each([&](auto entity, const auto& light, auto& transform, const auto& relationship) // const auto&
					{
						auto light_transform = engine::Transform(registry, entity, relationship, transform);

						auto uniform_prefix = ("lights[" + std::to_string(light_idx) + "].");

						auto attr = [&](const std::string& attr_name, auto&& value) // std::string_view
						{
							shader[(uniform_prefix + attr_name)] = value;
						};

						const auto& light_color = light.color;

						auto light_position = light_transform.get_position();

						//std::cout << light_position.x << ',' << light_position.y << ',' << light_position.z << '\n';

						attr("Position", light_position);
						attr("Color", light_color);
						attr("Linear", linear); // light.linear
						attr("Quadratic", quadratic); // light.linear
						
						///*
						// then calculate radius of light volume/sphere
						const float maxBrightness = std::fmaxf(std::fmaxf(light_color.r, light_color.g), light_color.b);
						float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);

						//std::cout << "Radius: " << radius << '\n';
						attr("Radius", radius);
						//*/

						light_idx++;
					});

					shader["viewPos"] = camera_transform.get_position();

					graphics.context->use(gBuffer.screen_quad, [&, this]()
					{
						graphics.context->draw();
					});

					graphics.context->copy_framebuffer(gBuffer.framebuffer, viewport, viewport, BufferType::Depth);
				});
			}
			

			// Framebuffer test.
			if (gBuffer.display_mode != GBufferDisplayMode::None)
			{
				auto& shader = *shaders.framebuffer_dbg;

				graphics.context->clear_textures(false); // true

				graphics.context->use(shader, [&, this]()
				{
					auto get_texture = [&, this]() -> const Texture&
					{
						switch (this->gBuffer.display_mode)
						{
							case GBufferDisplayMode::Normal:
								return gBuffer.normal;
							case GBufferDisplayMode::AlbedoSpecular:
								return gBuffer.albedo_specular;
						}

						return gBuffer.position;
					};

					auto fb_texture = graphics.context->use(get_texture()); // test_texture // gBuffer.normal

					//graphics.context->use(gBuffer.position, [&, this]() // normal // albedo_specular
					//{
						graphics.context->use(gBuffer.screen_quad, [&, this]()
						{
							graphics.context->draw();
						});
					//});
				});
			}
		}
		//*/

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

			case SDLK_f:
			{
				auto t = transform(world.get_camera());

				std::cout << "\nCamera:\n";
				std::cout << "Position: " << t.get_position() << '\n';
				std::cout << "Rotation: " << math::degrees(t.get_rotation()) << '\n';
				std::cout << "Scale: "    << t.get_scale()    << '\n';

				break;
			}

			case SDLK_c:
				gBuffer.display_mode = static_cast<GBufferDisplayMode>((static_cast<int>(gBuffer.display_mode) + 1) % static_cast<int>(GBufferDisplayMode::Modes));

				break;
		}
	}

	void ModelTest::on_keydown(const keyboard_event_t& event)
	{
		/*
		switch (event.keysym.sym)
		{
		}
		*/
	}
	
	graphics::PointRect ModelTest::update_viewport(engine::Entity camera)
	{
		graphics::PointRect viewport = {};

		window->get_size(viewport.end.x, viewport.end.y);

		if ((viewport.width != 0) && (viewport.height != 0))
		{
			graphics.context->set_viewport(0, 0, viewport.width, viewport.height);

			auto& camera_params = world.get_registry().get<engine::CameraParameters>(camera);

			camera_params.update_aspect_ratio(viewport.width, viewport.height);
		}

		return viewport;
	}
}