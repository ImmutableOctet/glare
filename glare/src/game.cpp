#include "game.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <cmath>
#include <cstdlib>
#include <optional>

#include <app/events.hpp>
#include <app/input/keycodes.hpp>
#include <engine/engine.hpp>
#include <graphics/native/opengl.hpp>

#include <engine/world/spin_component.hpp>
#include <engine/world/follow_component.hpp>
#include <engine/world/light.hpp>

#include <engine/free_look.hpp>

#include <sdl2/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glare
{
	Glare::init_shaders::init_shaders(Graphics& graphics)
	{
		auto preprocessor = std::string_view{ "\n#define LAYER_DEPTH_ENABLED 1\n" }; // //#define LAYER_POSITION_ENABLED 1\n

		lighting_pass = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/deferred_shading.vert",
			"assets/shaders/deferred_shading.frag",
			std::string {},
			preprocessor
		);

		// Not an initializer list for readability/testing purposes:
		forward = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/forward.vert",
			"assets/shaders/forward.frag"
		);

		forward_test = memory::allocate<graphics::Shader>
		(
			graphics.context,

			//"assets/tests/model_test/shaders/forward.vs",
			"assets/shaders/deferred_shading.vert",
			"assets/shaders/forward_test.frag"
		);

		geometry_pass = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/geometry_pass.vert",
			"assets/shaders/geometry_pass.frag",

			std::string {},

			preprocessor
		);

		framebuffer_dbg = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/8.1.fbo_debug.vert",
			"assets/shaders/8.1.fbo_debug.frag"
		);

		//light_debug = res.get_shader("assets/shaders/light_box.vert", "assets/shaders/light_box.frag");

		/*
		light_debug = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/light_box.vert",
			"assets/shaders/light_box.frag"
		);
		*/

		light_debug = forward;

		point_shadow_depth = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/shadow_mapping/point_shadows_depth.vert",
			"assets/shaders/shadow_mapping/point_shadows_depth.frag",
			"assets/shaders/shadow_mapping/point_shadows_depth.geom"
		);

		directional_shadow_depth = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/shadow_mapping/directional_shadows_depth.vert",
			"assets/shaders/shadow_mapping/directional_shadows_depth.frag"
		);

		/*
		shadow_test = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/shadow_mapping/3.2.2.point_shadows.vert",
			"assets/shaders/shadow_mapping/3.2.2.point_shadows.frag"
		);
		*/

		default_shader = geometry_pass; // shadow_test
	}

	Glare::Glare(bool auto_execute)
		: GraphicsApplication("Project Glare", 1600, 900, (app::WindowFlags::OpenGL | app::WindowFlags::Resizable), TARGET_UPDATE_RATE, false), // true
		gbuffer(graphics.context, window->get_size()),
		cfg(), // cfg(std::make_shared<engine::Config>()),
		shaders(graphics),
		resource_manager(graphics.context, shaders.default_shader), // shaders.forward
		world(cfg, resource_manager, TARGET_UPDATE_RATE)
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

		input.get_mouse().lock();

		setup_world(world);

		if (auto_execute)
		{
			execute();
		}
	}

	engine::Entity Glare::load_model(const std::string& path)
	{
		return engine::load_model(world, path);
	}

	engine::Transform Glare::model(const std::string& path)
	{
		auto entity = load_model(path);

		return transform(entity);
	}

	engine::Transform Glare::transform(engine::Entity entity)
	{
		return world.get_transform(entity);
	}

	void Glare::make_lights(engine::World& world)
	{
		using namespace std;

		math::Vector offset = { 0.0f, 0.0f, 0.0f }; // { 0.0f, 64.0f, 0.0f };

		srand(13); // 42

		for (unsigned int i = 0; i < NR_LIGHTS; i++)
		{
			math::Vector position =
			{
				(static_cast<float>(rand() % 600) - 300.0f),
				(static_cast<float>(rand() % 60) + 40.0f),
				(static_cast<float>(rand() % 600) - 300.0f)
			};

			// also calculate random color
			graphics::ColorRGB color =
			{
				(static_cast<float>(rand() % 100) / 100.0f) + 0.5f, // between 0.5 and 1.0 // 1.0f
				(static_cast<float>(rand() % 100) / 100.0f) + 0.5f, // between 0.5 and 1.0 // 1.0f
				(static_cast<float>(rand() % 100) / 100.0f) + 0.5f // between 0.5 and 1.0 // 1.0f
			};

			engine::create_light(world, (position + offset), color, engine::LightType::Point, engine::null, true, shaders.light_debug);
		}
	}

	void Glare::make_models(engine::World& world, engine::Entity player)
	{
		//return;

		auto& registry = world.get_registry();

		//auto sphere = load_model("assets/tests/model_test/Sonic/Sonic.b3d"); // model("assets/tests/model_test/sphere.obj");
		//auto sphere_t = transform(sphere);

		//sphere_t.move({ 14.0f, 10.0f, 30.0f });

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

	void Glare::setup_world(engine::World& world)
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
		//auto stage = world.load("assets/maps/collision_test");

		auto player = world.get_player(1);

		make_lights(world);
		make_models(world, player);

		world.register_event<app::input::KeyboardState, &Glare::on_user_keyboard_input>(*this);
	}

	engine::Transform Glare::get_named_transform(std::string_view name)
	{
		auto entity = world.get_by_name(name);
		auto transform = world.get_transform(entity);

		return transform;
	}

	void Glare::on_user_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		std::string_view target_obj = engine::DEFAULT_PLAYER_NAME;

		auto camera_t = transform(world.get_camera());
		auto camera_pos = camera_t.get_position();

		//auto transform = world.get_transform(world.get_camera());
		auto transform = get_named_transform(target_obj);

		auto angle = glm::normalize(transform.get_position() - camera_pos);

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
			transform.move(delta * 0.5f * angle, false);
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
			//transform.set_position({ 10.7f, 46.0f, -13.0f });
			auto transform = get_named_transform("Player");

			transform.rotateY(math::radians(10.0f * delta));
		}
	}

	void Glare::update(app::Milliseconds time)
	{
		auto& mouse = input.get_mouse();

		if (mouse.locked())
		{
			input.poll(world.get_event_handler());
		}

		world.update(time);
	}

	void Glare::render()
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
		auto view_position = camera_transform.get_position();

		///*
		{
			// Execute shadow-pass first.
			auto [point_shadows_enabled, directional_shadows_enabled] = render_shadows();

			auto [viewport, window_size] = update_viewport(camera);

			// Geometry pass:
			auto render_state = WorldRenderState
			{
				.dynamic_textures { &dynamic_texture_maps },

				.point_shadows
				{
					&point_light_shadows.positions,
					&point_light_shadows.far_planes,

					point_shadows_enabled
				},

				.directional_shadows
				{
					&directional_light_shadows.positions,
					&directional_light_shadows.matrices,

					directional_shadows_enabled
				},

				.meta
				{
					view_position
				}
			};

			// Geometry pass.

			//glDepthRange(0.1, 4000.0);

			auto& gbuffer = render_geometry(world, viewport, this->gbuffer, render_state);

			//graphics.context->clear(0.0f, 0.0f, 0.0f, 1.0f, BufferType::Color | BufferType::Depth); // gfx
			//graphics.context->clear(1.0f, 1.0f, 1.0f, 1.0f, BufferType::Color | BufferType::Depth); // gfx

			// Lighting pass.
			render_lighting(world, viewport, gbuffer, render_state);

			render_screen(viewport, gbuffer, gbuffer_display_mode);

			render_debug(viewport, gbuffer);
		}
		//*/

		gfx.flip(wnd);
	}

	graphics::GBuffer& Glare::render_geometry(engine::World& world, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, graphics::WorldRenderState& render_state)
	{
		auto& shader = *shaders.geometry_pass;

		//graphics.context->clear_textures(false); // true // <-- May not be needed.

		graphics.context->use(gbuffer.framebuffer, [&, this]()
		{
			graphics.context->clear(0.0f, 0.0f, 0.0f, 1.0f, (graphics::BufferType::Color | graphics::BufferType::Depth)); // gfx // 1.0f, 0.0f, 0.0f
			//graphics.context->clear(1.0f, 1.0f, 1.0f, 1.0f, (graphics::BufferType::Color | graphics::BufferType::Depth)); // gfx // 1.0f, 0.0f, 0.0f

			graphics.context->use(shader, [&, this]()
			{
				world.render
				(
					*graphics.canvas, viewport,
					false, true,

					&render_state
				);
			});
		});

		return gbuffer;
	}

	graphics::GBuffer& Glare::render_lighting(engine::World& world, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, const graphics::WorldRenderState& render_state)
	{
		auto& shader = *shaders.lighting_pass;

		graphics.context->use(shader, [&, this]()
		{
			graphics.context->clear_textures(true); // false

			shader["ambient_light"] = world.properties.ambient_light;

			//auto gPosition = graphics.context->use(*gbuffer.position, "g_position");
			
			std::optional<decltype(graphics.context->use(*gbuffer.position, "g_position"))> gPosition =
				(gbuffer.position.has_value()) ? std::optional { graphics.context->use(*gbuffer.position, "g_position") } : std::nullopt;

			//auto gPosition = graphics.context->use(gbuffer.position, "g_position");
			 
			//auto gDepth = graphics.context->use(gbuffer.depth, "g_depth");

			std::optional<decltype(graphics.context->use(*gbuffer.depth, "g_depth"))> gDepth =
				(gbuffer.depth.has_value()) ? std::optional { graphics.context->use(*gbuffer.depth, "g_depth") } : std::nullopt;

			auto gNormal = graphics.context->use(gbuffer.normal, "g_normal");
			auto gAlbedoSpec = graphics.context->use(gbuffer.albedo_specular, "g_albedo_specular");

			if (!gbuffer.position.has_value())
			{
				ASSERT(render_state.matrices.has_value());
				ASSERT(render_state.screen.has_value());

				const auto& matrices = *render_state.matrices;
				const auto& screen = *render_state.screen;

				auto fov_y = screen.fov_y;
				//auto fov_x = std::atanf((16.0f/9.0f) * std::tan(fov_y));

				auto aspect = screen.aspect_ratio;

				//aspect = 1600.0f / 900.0f;
				//aspect = 900.0f / 1600.0f;

				// Included for now:
				shader["inv_view"] = glm::inverse(matrices.view);
				shader["inv_projection"] = glm::inverse(matrices.projection);

				shader["inv_projview"] = glm::inverse(matrices.projection * matrices.view);


				shader["projection"] = matrices.projection;

				auto hs_near_plane = math::vec2{ (std::tan(fov_y / 2.0f) * aspect), (std::tan(fov_y / 2.0f)) };
				//auto hs_near_plane = math::vec2{ (std::tan(fov_x / 2.0f) * aspect), (std::tan(fov_y / 2.0f)) * 2.0f };

				auto depth_range = screen.depth_range;

				shader["half_size_near_plane"] = hs_near_plane;

				shader["depth_range"] = math::vec2(0.0, 1.0); // depth_range
			}

			if (render_state.meta.ambient_light.has_value())
			{
				shader["ambient_light"] = *render_state.meta.ambient_light;
			}

			shader["view_position"] = *render_state.meta.view_position;

			auto& registry = world.get_registry();

			// update attenuation parameters and calculate radius
			const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const float linear = 0.1f; // 0.000005f; // 0.7f; // 0.005f;
			const float quadratic = 1.0f / 2000.0f; // 1.8f; // 0.0005f;

			unsigned int light_idx = 0;

			registry.view<engine::LightComponent, engine::TransformComponent, engine::Relationship>().each([&](auto entity, const auto& light, auto& transform, const auto& relationship) // const auto&
			{
				auto light_transform = engine::Transform(registry, entity, relationship, transform);

				auto uniform_prefix = ("point_lights[" + std::to_string(light_idx) + "].");

				auto attr = [&](const std::string& attr_name, auto&& value) // std::string_view
				{
					shader[(uniform_prefix + attr_name)] = value;
				};

				const auto& light_color = light.color;

				auto light_position = light_transform.get_position();

				//std::cout << light_position.x << ',' << light_position.y << ',' << light_position.z << '\n';

				attr("position", light_position);
				attr("color", light_color);
				attr("linear", linear); // light.linear
				attr("quadratic", quadratic); // light.linear

				///*
				// then calculate radius of light volume/sphere
				const float maxBrightness = std::fmaxf(std::fmaxf(light_color.r, light_color.g), light_color.b);
				float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);

				//std::cout << "Radius: " << radius << '\n';
				attr("radius", radius);
				//*/

				light_idx++;
			});

			shader["point_lights_count"] = (light_idx-1);

			//graphics.context->toggle(graphics::ContextFlags::FaceCulling, false);

			//glFrontFace(GL_CCW);

			graphics.context->use(gbuffer.screen_quad, [&, this]()
			{
				graphics.context->draw();
			});

			//glFrontFace(GL_CW);

			//graphics.context->toggle(graphics::ContextFlags::FaceCulling, true);

			if (!gbuffer.depth.has_value())
			{
				graphics.context->copy_framebuffer(gbuffer.framebuffer, viewport, viewport, graphics::BufferType::Depth);
			}
		});

		return gbuffer;
	}

	graphics::GBuffer& Glare::render_screen(const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, GBufferDisplayMode display_mode)
	{
		if (display_mode == GBufferDisplayMode::None)
		{
			return gbuffer;
		}

		graphics.context->toggle(graphics::ContextFlags::DepthTest, false);

		auto& shader = *shaders.framebuffer_dbg;

		graphics.context->clear_textures(false); // true

		graphics.context->use(shader, [&, this]()
		{
			auto get_texture = [&, this]() -> const graphics::Texture&
			{
				switch (display_mode)
				{
					case GBufferDisplayMode::Normal:
						return gbuffer.normal;
					case GBufferDisplayMode::AlbedoSpecular:
						return gbuffer.albedo_specular;

					case GBufferDisplayMode::Depth:
						if (gbuffer.depth)
						{
							return *gbuffer.depth;
						}

						break;

					///*
					case GBufferDisplayMode::ShadowMap:
					{
						auto light = world.get_by_name("shadow_test");

						if (light != engine::null)
						{
							auto& registry = world.get_registry();
							auto* shadows = registry.try_get<engine::DirectionLightShadows>(light);

							if (shadows)
							{
								const auto& depth = *shadows->shadow_map.get_depth_map();

								return depth;
							}
						}

						break;
					}
					//*/
				}

				if (gbuffer.position)
				{
					return *(gbuffer.position);
				}

				return gbuffer.albedo_specular;
			};

			auto fb_texture = graphics.context->use(get_texture()); // test_texture // gbuffer.normal

			//graphics.context->use(gbuffer.position, [&, this]() // normal // albedo_specular
			//{
			graphics.context->use(gbuffer.screen_quad, [&, this]()
			{
				graphics.context->draw();
			});
			//});
		});

		graphics.context->toggle(graphics::ContextFlags::DepthTest, true);

		return gbuffer;
	}

	void Glare::on_keyup(const keyboard_event_t& event)
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

			t.move({ 0.0f, 0.01f * world.get_delta_time(), 0.0f });

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

				auto& registry = world.get_registry();
				auto& m = registry.get<engine::ModelComponent>(target_entity);

				m.receives_shadow = false;
			}

			//engine::SimpleFollowComponent::update(world);

			break;
		}

		case SDLK_h:
		{
			auto& registry = world.get_registry();

			auto light = world.get_by_name("shadow_test");
			//auto& shadows = registry.get<engine::PointLightShadows>(light);

			auto lt = world.get_transform(light);
			auto ct = world.get_transform(world.get_camera());

			lt.set_position(ct.get_local_position());
			lt.set_rotation(ct.get_rotation());

			break;
		}

		case SDLK_c:
			gbuffer_display_mode = static_cast<GBufferDisplayMode>((static_cast<int>(gbuffer_display_mode) + 1) % static_cast<int>(GBufferDisplayMode::Modes));

			break;
		}
	}

	void Glare::on_keydown(const keyboard_event_t& event)
	{
		/*
		switch (event.keysym.sym)
		{
		}
		*/
	}

	void Glare::on_window_resize(app::Window& window, int width, int height)
	{
		if (this->window.get() == &window)
		{
			world.update_camera_parameters(width, height);

			gbuffer.resize(width, height);
		}
	}

	std::tuple<graphics::Viewport, math::vec2i> Glare::update_viewport() // graphics::Viewport
	{
		graphics::Viewport viewport = {};

		int w_width, w_height;

		window->get_size(w_width, w_height);

		viewport.set_size(static_cast<graphics::PointRect::Type>(w_width), static_cast<graphics::PointRect::Type>(w_height));

		if ((w_width != 0) && (w_height != 0))
		{
			graphics.context->set_viewport(viewport);
		}

		return { viewport, { w_width, w_height } };
	}

	std::tuple<graphics::Viewport, math::vec2i> Glare::update_viewport(engine::Entity camera)
	{
		auto [viewport, w_size] = update_viewport();

		if ((w_size.x != 0) && (w_size.y != 0))
		{
			auto& camera_params = world.get_registry().get<engine::CameraParameters>(camera);

			camera_params.update_aspect_ratio(viewport.get_width(), viewport.get_height());
		}

		return { viewport, w_size };
	}

	std::tuple<bool, bool> Glare::render_shadows(bool point_lights, bool directional_lights)
	{
		// Point-light shadows:
		if (point_lights)
		{
			auto& point_shadows = dynamic_texture_maps["shadow_cubemap"];

			point_shadows.clear();

			point_light_shadows.positions.clear();
			point_light_shadows.far_planes.clear();

			point_lights = world.render_point_shadows
			(
				*graphics.canvas,
				*shaders.point_shadow_depth,

				&point_shadows,

				&point_light_shadows.positions,
				&point_light_shadows.far_planes
			);
		}

		// Shadow pass (directional lights):
		if (directional_lights)
		{
			auto& directional_shadows = dynamic_texture_maps["directional_shadow_map"];

			directional_shadows.clear();

			directional_light_shadows.positions.clear();
			directional_light_shadows.matrices.clear();

			directional_lights = world.render_directional_shadows
			(
				*graphics.canvas,
				*shaders.directional_shadow_depth,
					
				&directional_shadows,

				&directional_light_shadows.positions,
				&directional_light_shadows.matrices
			);
		}

		return { point_lights, directional_lights };
	}

	graphics::GBuffer& Glare::render_debug(const graphics::Viewport& viewport, graphics::GBuffer& gbuffer)
	{
		// Render debug meshes for lights.
		graphics.context->use(*shaders.light_debug, [&, this]()
		{
			world.render(*graphics.canvas, viewport, false, true);
		});

		return gbuffer;
	}
}