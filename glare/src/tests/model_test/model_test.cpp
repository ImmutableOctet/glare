#include "model_test.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <cmath>
#include <cstdlib>

#include <app/events.hpp>
#include <app/input/keycodes.hpp>
#include <engine/engine.hpp>
#include <graphics/native/opengl.hpp>

#include <engine/world/spin_component.hpp>
#include <engine/world/follow_component.hpp>

#include <engine/free_look.hpp>

#include <sdl2/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glare::tests
{
	ModelTest::init_shaders::init_shaders(Graphics& graphics)
	{
		// Not an initializer list for readability/testing purposes:
		forward = memory::allocate<graphics::Shader>
		(
			graphics.context,
			
			"assets/tests/model_test/shaders/forward.vs",
			"assets/tests/model_test/shaders/forward.fs"
		);

		forward_test = memory::allocate<graphics::Shader>
		(
			graphics.context,
			
			//"assets/tests/model_test/shaders/forward.vs",
			"assets/tests/model_test/shaders/8.1.deferred_shading.vs",
			"assets/tests/model_test/shaders/forward_test.fs"
		);

		geometry = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/tests/model_test/shaders/8.1.g_buffer.vs",
			"assets/tests/model_test/shaders/8.1.g_buffer.fs"
		);

		lighting_pass = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/tests/model_test/shaders/8.1.deferred_shading.vs",
			"assets/tests/model_test/shaders/8.1.deferred_shading.fs"
		);

		framebuffer_dbg = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/tests/model_test/shaders/8.1.fbo_debug.vs",
			"assets/tests/model_test/shaders/8.1.fbo_debug.fs"
		);

		/*
		light_box = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/tests/model_test/shaders/8.1.deferred_light_box.vs",
			"assets/tests/model_test/shaders/8.1.deferred_light_box.fs"
		);
		*/
	}

	ModelTest::ModelTest(bool auto_execute)
		: GraphicsApplication("Project Glare", 1600, 900, (app::WindowFlags::OpenGL|app::WindowFlags::Resizable), TARGET_UPDATE_RATE, false), // true
		  shaders(graphics),
		  resource_manager(graphics.context, shaders.forward),
		  world(resource_manager, TARGET_UPDATE_RATE)
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

		int screen_width, screen_height;

		window->get_size(screen_width, screen_height);

		gBuffer.screen_quad = Mesh::GenerateTexturedQuad(graphics.context);

		const auto gBuffer_flags = TextureFlags::None;

		//test_texture = Texture(graphics.context, "assets/tests/model_test/Bianco/T (1).png");

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
		return engine::load_model(world, path);
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

	void ModelTest::make_models(engine::World& world, engine::Entity player)
	{
		//return;

		auto& registry = world.get_registry();

		auto sphere = load_model("assets/tests/model_test/Sonic/Sonic.b3d"); // model("assets/tests/model_test/sphere.obj");
		auto sphere_t = transform(sphere);

		sphere_t.move({ 14.0f, 10.0f, 30.0f });

		auto cube = load_model("assets/tests/model_test/cube.b3d");
		
		registry.emplace<engine::NameComponent>(cube, "Spinning Cube");
		registry.emplace<engine::SpinBehavior>(cube);

		world.set_parent(cube, player);

		transform(cube).set_position({ 25.0f, 46.0f, -20.0f });

		// Light boxes:
		/*
		registry.view<engine::LightComponent>().each([&](auto light, const auto& light_component)
		{
			attach_model(world, light, cube_model, { light_component.color, 1.0f });
		});
		*/

		//auto geometry = model("assets/tests/model_test/geometry.obj");
		//geometry.move({ 50.0f, 31.0f, 28.0f });
	}

	void ModelTest::setup_world(engine::World& world)
	{
		std::string path;

		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\Genesis Redux Project 1.1\\Stages\\GreenHill\\Stage\\Stage.b3d";
		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic-backups\\BlitzSonicv02\\Stages\\GreenHill\\Stage\\Stage.b3d";
		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\HAXY EDIT OF DOOM\\Stages\\Chao World\\Stage\\Stage.b3d";
		//path = "model_renderer/assets/tests/model_test/Bianco/Stage2.b3d";
		//path = "assets/tests/model_test/SeasideShortcut/Stage.obj";
		//path = "assets/tests/model_test/stage.b3d";
		//path = "Q:\\Projects\\BlitzSonic Projects\\Blitzsonic Related\\HAXY EDIT OF DOOM\\Stages\\Emerald Coast\\1\\EC1.b3d";
		//path = "assets/maps/test01/stage.b3d";

		//auto stage = load_model(path);

		auto stage = world.load("assets/maps/test01");

		auto player = world.get_player(1);

		make_lights(world);
		make_models(world, player);

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
		std::string_view target_obj = engine::DEFAULT_PLAYER_NAME;

		auto camera_t = transform(world.get_camera());
		auto camera_pos = camera_t.get_position();

		//auto transform = world.get_transform(world.get_camera());
		auto transform = get_named_transform(target_obj);

		auto angle = glm::normalize(camera_pos - transform.get_position());

		float delta = world.delta();

		/*
		if (keyboard.get_key(SDL_SCANCODE_Q))
		{
			world.set_parent(world.get_camera(), world.get_by_name(engine::DEFAULT_PLAYER_NAME));
		}
		*/

		if (keyboard.get_key(SDL_SCANCODE_V))
		{
			auto t = world.get_transform(world.get_root());

			t.rotateY(0.1f);
		}

		if (keyboard.get_key(SDL_SCANCODE_2))
		{
			transform.move(delta * 0.1f * angle, true);
		}

		if (keyboard.get_key(SDL_SCANCODE_Z))
		{
			//dbg->info("Angle: {}", angle);

			//transform.move(angle * delta * 0.001f * -1.0f, true);

			//transform.move({0.0f, 0.01f * delta, 0.0f});

			//transform.set_position({ 10.0f, 20.01f, 30.0f });
			transform.set_position({ 84.0751f, 69.8489f, 71.7952f });

			dbg->info("New Position: {}", transform.get_position());
		}

		if (keyboard.get_key(SDL_SCANCODE_R))
		{
			transform.set_position({ 10.7f, 46.0f, -13.0f });
			auto transform = get_named_transform("Player");

			transform.rotateY(math::radians(10.0f * delta));
		}
	}

	void ModelTest::update(app::Milliseconds time)
	{
		auto& mouse = input.get_mouse();

		if (mouse.locked())
		{
			input.poll(world.get_event_handler());
		}

		world.update(time);
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
						world.render(*graphics.canvas, viewport, false); // , true

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

			case SDLK_q:
			{
				auto t = world.get_transform(world.get_root());

				t.move({0.0f, 0.01f * world.get_delta_time(), 0.0f});

				break;
			}

			case SDLK_e:
			{
				auto child_obj = world.get_player(1); // world.get_camera();

				const auto& rel = world.get_registry().get<engine::Relationship>(child_obj);

				auto rel_parent = rel.get_parent();
				auto root = world.get_root();

				if ((rel_parent == engine::null) || (rel_parent == root))
				{
					//auto parent_obj = world.get_by_name(engine::DEFAULT_PLAYER_NAME);

					auto parent_obj = world.get_by_name("Rotating Platform"); // "Spinning Cube"

					world.set_parent(child_obj, parent_obj);
				}
				else
				{
					world.set_parent(child_obj, engine::null); // root
				}

				break;
			}

			case SDLK_f:
			{
				auto target_entity = world.get_camera(); // world.get_by_name("Moving Sphere"); // "Spinning Cube" // world.get_root()

				if (target_entity != engine::null)
				{
					auto t = transform(target_entity);

					std::cout << "\nCamera:\n";
					std::cout << "Position: " << t.get_position() << '\n';
					std::cout << "Rotation: " << math::degrees(t.get_rotation()) << '\n';
					std::cout << "Scale: " << t.get_scale() << '\n';
				}

				//engine::SimpleFollowComponent::update(world);

				break;
			}

			case SDLK_g:
			{
				auto target_entity = world.get_player(1);

				if (target_entity != engine::null)
				{
					auto t = transform(target_entity);

					std::cout << "\nPlayer:\n";
					std::cout << "Position: " << t.get_position() << '\n';
					std::cout << "Rotation: " << math::degrees(t.get_rotation()) << '\n';
					std::cout << "Scale: " << t.get_scale() << '\n';
				}

				//engine::SimpleFollowComponent::update(world);

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
	
	graphics::Viewport ModelTest::update_viewport(engine::Entity camera)
	{
		graphics::Viewport viewport = {};

		int w_width, w_height;

		window->get_size(w_width, w_height);

		viewport.set_size(static_cast<float>(w_width), static_cast<float>(w_height));

		if ((w_width != 0) && (w_height != 0))
		{
			graphics.context->set_viewport(viewport);

			auto& camera_params = world.get_registry().get<engine::CameraParameters>(camera);

			camera_params.update_aspect_ratio(viewport.get_width(), viewport.get_height());
		}

		return viewport;
	}
}