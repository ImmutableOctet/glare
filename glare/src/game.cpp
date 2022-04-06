#include "game.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cassert>

#include <app/events.hpp>
#include <app/input/keycodes.hpp>
#include <engine/engine.hpp>

#include <util/variant.hpp>

#include <engine/debug.hpp>
#include <engine/world/meta/meta.hpp>
#include <engine/world/debug/debug.hpp>

#include <graphics/native/opengl.hpp>
#include <graphics/world_render_state.hpp>

#include <engine/events/events.hpp>
#include <engine/type_component.hpp>
#include <engine/world/spin_component.hpp>
#include <engine/world/follow_component.hpp>
#include <engine/world/light.hpp>

#include <engine/free_look.hpp>

#include <sdl2/SDL_video.h>
#include <sdl2/SDL_events.h>

#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <format>

namespace glare
{
	Glare::init_shaders::init_shaders(Graphics& graphics)
	{
		std::string_view preprocessor =
			"\n#define LAYER_DEPTH_ENABLED 1\n"
			//"\n#define LAYER_POSITION_ENABLED 1\n"
		;

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

		animated_geometry_pass = memory::allocate<graphics::Shader>
		(
			graphics.context,

			"assets/shaders/geometry_pass.vert",
			"assets/shaders/geometry_pass.frag",

			std::string {},

			(std::string(preprocessor) + "\n#define ANIMATION_ENABLED 1\n")
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

		// TODO: Add proper construction for animated version of 'geometry_pass'.
		default_animated_shader = animated_geometry_pass;
	}

	Glare::Glare(bool auto_execute)
		: GraphicsApplication("Project Glare", 1600, 900, (app::WindowFlags::OpenGL | app::WindowFlags::Resizable), TARGET_UPDATE_RATE, false, true), // true
		gbuffer(graphics.context, window->get_size()),
		cfg(), // cfg(std::make_shared<engine::Config>()),
		shaders(graphics),
		resource_manager(graphics.context, shaders.default_shader, shaders.default_animated_shader), // shaders.forward
		world(cfg, resource_manager, TARGET_UPDATE_RATE),
		dbg_listener(world)
	{
		using namespace graphics;

		//GLint value = 0;
		//glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &value); // GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT

		/*
		int screen_width, screen_height;

		window->get_size(screen_width, screen_height);
		*/

		world.register_event<app::input::KeyboardState, &Glare::on_user_keyboard_input>(*this);

		// Normally, we lock when starting.
		//input.get_mouse().lock();

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
				(static_cast<float>(rand() % 100) / 255.0f) + 0.5f, // between 0.5 and 1.0 // 1.0f
				(static_cast<float>(rand() % 100) / 255.0f) + 0.5f, // between 0.5 and 1.0 // 1.0f
				(static_cast<float>(rand() % 100) / 255.0f) + 0.5f // between 0.5 and 1.0 // 1.0f
			};

			engine::create_point_light
			(
				world,
				(position + offset),
				
				{
					.diffuse{color}
				},

				{},

				engine::null,
				true,
				shaders.light_debug
			);
		}
	}

	void Glare::make_models(engine::World& world, engine::Entity player)
	{
		return;

		auto& registry = world.get_registry();

		//auto sphere = load_model("assets/tests/model_test/Sonic/Sonic.b3d"); // model("assets/tests/model_test/sphere.obj");
		//auto sphere_t = transform(sphere);

		//sphere_t.move({ 14.0f, 10.0f, 30.0f });

		auto cube = load_model("assets/tests/model_test/cube.b3d");

		registry.emplace<engine::NameComponent>(cube, "Spinning Cube");
		registry.emplace<engine::SpinBehavior>(cube);

		//world.set_parent(cube, player);

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
		auto stage = world.load("assets/maps/test01");
		//auto stage = world.load("assets/maps/test01 - old");

		//auto stage = world.load("assets/maps/room");
		//auto stage = world.load("assets/maps/collision_test");
		
		//auto stage = world.load("assets/maps/story/2.ice-world/ice-connector");

		auto player = world.get_player(1);

		//make_lights(world);
		make_models(world, player);
	}

	engine::Transform Glare::get_named_transform(std::string_view name)
	{
		auto entity = world.get_by_name(name);
		auto transform = world.get_transform(entity);

		return transform;
	}

	void Glare::on_user_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		//std::string_view target_obj = engine::DEFAULT_PLAYER_NAME;

		//auto camera_t = transform(world.get_camera());
		//auto camera_pos = camera_t.get_position();

		//auto transform = world.get_transform(world.get_camera());
		//auto transform = get_named_transform(target_obj);
		//auto angle = glm::normalize(transform.get_position() - camera_pos);
		//float delta = world.delta();
	}

	void Glare::update(app::Milliseconds time)
	{
		auto& mouse = input.get_mouse();

		if (mouse.locked())
		{
			input.poll(world.get_event_handler());
		}

		world.update(time);

		auto camera = world.get_camera(); // world.get_by_name("Moving Sphere"); // "Spinning Cube" // world.get_root()

		if (camera != engine::null)
		{
			auto t = transform(camera);
			auto position = t.get_position();

			std::string window_title = std::format("FPS: {} | Camera position: {},{},{}", graphics.framerate, position.x, position.y, position.z);

			window->set_title(window_title);
		}
	}

	void Glare::render_debug_controls()
	{
		using namespace engine::meta;

		if (!imgui_enabled())
			return;

		meta_controls();

		auto player = world.get_player();
		assert(player != engine::null);

		auto player_model = world.get_child_by_name(player, "model", false);
		assert(player_model != engine::null);

		auto target = player_model;

		animation_control(world, target);

		hierarchy_control(world, world.get_root());
	}

	void Glare::meta_controls()
	{
		ImGui::Begin("Meta", nullptr, ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoTitleBar);

		if (ImGui::Button("Switch to Game"))
		{
			auto& mouse = input.get_mouse();
			mouse.toggle_lock();
		}

		auto& io = ImGui::GetIO();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();
	}

	void Glare::render()
	{
		using namespace graphics;

		//auto& gfx = *graphics.canvas;
		//auto& wnd = *window;

		render_debug_controls();

		//std::sinf(milliseconds())

		// Backbuffer clear; for gbuffer, see below.

		graphics.context->clear(0.1f, 0.33f, 0.25f, 1.0f, BufferType::Color|BufferType::Depth); // gfx
		//graphics.context->clear(0.0f, 0.0f, 0.0f, 1.0f, BufferType::Color | BufferType::Depth);
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

			//gfx.flip(wnd); return;

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

			render_scene(world, viewport, gbuffer, render_state);

			render_screen(viewport, gbuffer, gbuffer_display_mode);

			render_debug(viewport, gbuffer);
		}
		//*/
	}

	graphics::GBuffer& Glare::render_geometry(engine::World& world, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, graphics::WorldRenderState& render_state)
	{
		//graphics.context->clear_textures(false); // true // <-- May not be needed.

		graphics.context->use(gbuffer.framebuffer, [&, this]()
		{
			graphics.context->clear(0.0f, 0.0f, 0.0f, 1.0f, (graphics::BufferType::Color | graphics::BufferType::Depth)); // gfx // 1.0f, 0.0f, 0.0f
			//graphics.context->clear(1.0f, 1.0f, 1.0f, 1.0f, (graphics::BufferType::Color | graphics::BufferType::Depth)); // gfx // 1.0f, 0.0f, 0.0f

			/*
			graphics.context->use(*shaders.geometry_pass, [&, this]()
			{
				world.render
				(
					*graphics.canvas, viewport,
					false, true,

					&render_state
				);
			});

			graphics.context->use(*shaders.animated_geometry_pass, [&, this]()
			{
				world.render
				(
					*graphics.canvas, viewport,
					false, true,

					&render_state
				);
			});
			*/

			world.render
			(
				*graphics.canvas, viewport,
				false, false,

				&render_state
			);
		});

		return gbuffer;
	}

	graphics::GBuffer& Glare::render_scene(engine::World& world, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, const graphics::WorldRenderState& render_state)
	{
		auto& shader = *shaders.lighting_pass;

		graphics.context->use(shader, [&, this]()
		{
			graphics.context->clear_textures(true); // false

			//auto gPosition = graphics.context->use(*gbuffer.position, "g_position");
			
			std::optional<decltype(graphics.context->use(*gbuffer.position, "g_position"))> gPosition =
				(gbuffer.position.has_value()) ? std::optional { graphics.context->use(*gbuffer.position, "g_position") } : std::nullopt;

			//auto gPosition = graphics.context->use(gbuffer.position, "g_position");
			 
			//auto gDepth = graphics.context->use(gbuffer.depth, "g_depth");

			std::optional<decltype(graphics.context->use(*gbuffer.depth, "g_depth"))> gDepth =
				(gbuffer.depth.has_value()) ? std::optional { graphics.context->use(*gbuffer.depth, "g_depth") } : std::nullopt;

			auto gNormal = graphics.context->use(gbuffer.normal, "g_normal");
			auto gAlbedoSpec = graphics.context->use(gbuffer.albedo_specular, "g_albedo_specular");

			std::optional<decltype(graphics.context->use(*gbuffer.render_flags, "g_render_flags"))> g_render_flags =
				(gbuffer.render_flags.has_value()) ? std::optional{ graphics.context->use(*gbuffer.render_flags, "g_render_flags") } : std::nullopt;

			if (!gbuffer.position.has_value())
			{
				assert(render_state.matrices.has_value());
				assert(render_state.screen.has_value());

				const auto& matrices = *render_state.matrices;
				const auto& screen = *render_state.screen;

				auto fov_y = screen.fov_y;

				auto aspect = screen.aspect_ratio;

				shader["inv_view"] = glm::inverse(matrices.view);
				//shader["inv_projection"] = glm::inverse(matrices.projection);
				//shader["inv_projview"] = glm::inverse(matrices.projection * matrices.view);


				shader["projection"] = matrices.projection;

				auto hs_near_plane = math::vec2{ (std::tan(fov_y / 2.0f) * aspect), (std::tan(fov_y / 2.0f)) };

				shader["half_size_near_plane"] = hs_near_plane;

				//auto depth_range = screen.depth_range;
				//shader["depth_range"] = math::vec2(0.0, 1.0); // depth_range
			}
			
			// Ambient light lookup is handled in previous phase, upload happens here:
			//shader["ambient_light"] = world.properties.ambient_light;

			if (render_state.meta.ambient_light.has_value())
			{
				shader["ambient_light"] = *render_state.meta.ambient_light;
			}

			if (render_state.meta.view_position.has_value())
			{
				shader["view_position"] = *render_state.meta.view_position;
			}


			std::size_t point_shadow_n_layers = 0;
			std::size_t directional_shadow_n_layers = 0;

			// Shadow-maps:
			{
				// Point shadows:
				const auto& point_shadow_lp = render_state.point_shadows.light_positions;

				if (point_shadow_lp.has_value())
				{
					const auto& light_pos_v = *point_shadow_lp;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Vector*>(light_pos_v, [&](const graphics::Vector* vec)
					{
						assert(vec);
					
						shader["point_shadow_light_position"] = *vec;

						point_shadow_n_layers = 1;
					}))
					{}
					else if (util::peek_value<graphics::VectorArray*>(light_pos_v, [&](const graphics::VectorArray* vec)
					{
						assert(vec);

						shader["point_shadow_light_position"] = *vec;

						point_shadow_n_layers = vec->size();
					}))
					{}
				}

				const auto& point_shadow_fp = render_state.point_shadows.far_planes;

				if (point_shadow_fp.has_value())
				{
					const auto& far_v = *point_shadow_fp;

					// TODO: Implement as visit:
					if (util::peek_value<float*>(far_v, [&](const float* far_plane)
					{
						assert(far_plane);

						shader["point_shadow_far_plane"] = *far_plane;

						//point_shadow_n_layers = std::min(point_shadow_n_layers, static_cast<std::size_t>(1));
					}))
					{}
					else if (util::peek_value<graphics::FloatArray*>(far_v, [&](const graphics::FloatArray* far_plane)
					{
						assert(far_plane);

						shader["point_shadow_far_plane"] = *far_plane;

						//point_shadow_n_layers = std::min(point_shadow_n_layers, far_plane->size());
					}))
					{}
				}

				// Directional shadows:
				const auto& directional_shadow_lp = render_state.directional_shadows.light_positions;

				if (directional_shadow_lp.has_value())
				{
					const auto& light_pos_v = *directional_shadow_lp;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Vector*>(light_pos_v, [&](const graphics::Vector* vec)
					{
						assert(vec);
					
						shader["directional_shadow_light_position"] = *vec;

						directional_shadow_n_layers = 1;
					}))
					{}
					else if (util::peek_value<graphics::VectorArray*>(light_pos_v, [&](const graphics::VectorArray* vec)
					{
						assert(vec);

						shader["directional_shadow_light_position"] = *vec;

						directional_shadow_n_layers = vec->size();
					}))
					{}
				}

				const auto& directional_shadow_mat = render_state.directional_shadows.light_matrices;

				if (directional_shadow_mat.has_value())
				{
					const auto& mat_v = *directional_shadow_mat;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Matrix*>(mat_v, [&](const graphics::Matrix* matrix)
					{
						assert(matrix);

						shader["directional_shadow_light_space_matrix"] = *matrix;

						//directional_shadow_n_layers = std::min(directional_shadow_n_layers, static_cast<std::size_t>(1));
					}))
					{}
					else if (util::peek_value<graphics::MatrixArray*>(mat_v, [&](const graphics::MatrixArray* matrices)
					{
						assert(matrices);

						shader["directional_shadow_light_space_matrix"] = *matrices;

						//directional_shadow_n_layers = std::min(directional_shadow_n_layers, matrices->size());
					}))
					{}
				}
			}

			shader["directional_shadows_count"] = directional_shadow_n_layers;
			shader["point_shadows_count"] = point_shadow_n_layers;

			if (render_state.dynamic_textures.has_value())
			{
				//static const std::string texture_name = "depth_map"; // constexpr
				const auto& dynamic_textures_v = *render_state.dynamic_textures;

				bool shadows_enabled = true; //!(draw_mode & DrawMode::IgnoreShadows);

				// TODO: Implement as visit:
				if (util::peek_value<std::tuple<std::string_view, const graphics::Texture*>>(dynamic_textures_v, [&](const std::tuple<std::string_view, const graphics::Texture*>& tdata)
				{
					const auto& texture_name = std::get<0>(tdata);
					const auto* texture = std::get<1>(tdata);

					if (!shadows_enabled)
					{
						if ((texture_name.find("shadow") != std::string::npos))
						{
							// Don't bind this texture.
							return; // From lambda.
						}
					}

					assert(texture);
					graphics.canvas->bind_texture(*texture, texture_name); // bind_texture(tdata);
				}))
				{}
				else if (util::peek_value<const graphics::NamedTextureArrayRaw*>(dynamic_textures_v, [&](const graphics::NamedTextureArrayRaw* tdata)
				{
					//assert(tdata);
					if (tdata)
					{
						graphics.canvas->bind_textures(*tdata, [shadows_enabled](const std::string& texture_name, const graphics::TextureArrayRaw& textures)
						{
							if (!shadows_enabled) // tolower(...)
							{
								if ((texture_name.find("shadow") != std::string::npos))
								{
									// Don't bind this texture.
									return false;
								}
							}

							// Bind this texture.
							return true;
						}); // false
					}
				}))
				{}
			}

			render_lights(world, render_state, shader);

			//graphics.context->toggle(graphics::ContextFlags::FaceCulling, false);

			//glFrontFace(GL_CCW);

			graphics.context->use(gbuffer.screen_quad, [&, this]()
			{
				graphics.context->draw();
			});

			//glFrontFace(GL_CW);

			//graphics.context->toggle(graphics::ContextFlags::FaceCulling, true);

			//auto e = glGetError();

			if (!gbuffer.depth.has_value())
			{
				graphics.context->copy_framebuffer(gbuffer.framebuffer, viewport, viewport, graphics::BufferType::Depth);
			}
		});

		return gbuffer;
	}

	graphics::GBuffer& Glare::render_lights(engine::World& world, const graphics::WorldRenderState& render_state, graphics::Shader& shader)
	{
		auto& registry = world.get_registry();

		unsigned int directional_light_idx = 0;
		unsigned int spot_light_idx        = 0;
		unsigned int point_light_idx       = 0;

		registry.view<engine::LightComponent, engine::TransformComponent, engine::Relationship>().each([&](auto entity, const auto& light, auto& transform, const auto& relationship) // const auto&
		{
			switch (light.type)
			{
				case engine::LightType::Directional:
					if (render_directional_light(world, render_state, shader, entity, light, transform, relationship, directional_light_idx) != engine::null)
					{
						directional_light_idx++;
					}

					break;
				case engine::LightType::Spotlight:
					if (render_spot_light(world, render_state, shader, entity, light, transform, relationship, spot_light_idx) != engine::null)
					{
						spot_light_idx++;
					}

					break;
				case engine::LightType::Point:
					if (render_point_light(world, render_state, shader, entity, light, transform, relationship, point_light_idx) != engine::null)
					{
						point_light_idx++;
					}

					break;
			}
		});

		shader["directional_lights_count"] = directional_light_idx;
		shader["spot_lights_count"]        = spot_light_idx;
		shader["point_lights_count"]       = point_light_idx;

		return gbuffer;
	}

	engine::Entity Glare::render_directional_light
	(
		engine::World& world,
		const graphics::WorldRenderState& render_state,
		graphics::Shader& shader,
		
		engine::Entity entity,
		const engine::LightComponent& light,
		engine::TransformComponent& transform,
		const engine::Relationship& relationship,
		
		unsigned int directional_light_idx
	)
	{
		auto& registry = world.get_registry();

		auto light_transform = engine::Transform(registry, entity, relationship, transform);
		//"directional_lights[0].color"
		auto uniform_prefix = ("directional_lights[" + std::to_string(directional_light_idx) + "].");

		auto attr = [&](const std::string& attr_name, auto&& value) // std::string_view
		{
			shader[(uniform_prefix + attr_name)] = value;
		};

		auto light_position = light_transform.get_position();
		auto light_direction = glm::normalize(-light_position);
		//auto light_direction = light_transform.get_direction_vector();

		//std::cout << light_position.x << ',' << light_position.y << ',' << light_position.z << '\n';

		attr("position", light_position); // <-- TODO: Remove (Not needed for directional lights)

		attr("direction", light_direction);

		attr("ambient",  light.properties.ambient);
		attr("diffuse",  light.properties.diffuse);
		attr("specular", light.properties.specular);

		auto* dir = registry.try_get<engine::DirectionalLightComponent>(entity);
		
		assert(dir);

		attr("use_position", dir->use_position);

		return entity;
	}

	engine::Entity Glare::render_spot_light
	(
		engine::World& world,
		const graphics::WorldRenderState& render_state,
		graphics::Shader& shader,
		
		engine::Entity entity,
		const engine::LightComponent& light,
		engine::TransformComponent& transform,
		const engine::Relationship& relationship,
		
		unsigned int spot_light_idx
	)
	{
		auto& registry = world.get_registry();

		auto light_transform = engine::Transform(registry, entity, relationship, transform);

		auto uniform_prefix = ("spot_lights[" + std::to_string(spot_light_idx) + "].");

		auto attr = [&](const std::string& attr_name, auto&& value) // std::string_view
		{
			shader[(uniform_prefix + attr_name)] = value;
		};

		auto light_position = light_transform.get_position();
		auto light_direction = light_transform.get_direction_vector();

		//std::cout << light_position.x << ',' << light_position.y << ',' << light_position.z << '\n';

		attr("position", light_position);
		attr("direction", light_direction);

		attr("ambient",  light.properties.ambient);
		attr("diffuse",  light.properties.diffuse);
		attr("specular", light.properties.specular);

		auto* spot = registry.try_get<engine::SpotLightComponent>(entity);
		
		assert(spot);

		attr("cutoff", spot->cutoff);
		attr("outer_cutoff", spot->outer_cutoff);

		attr("constant", spot->constant);
		attr("linear", spot->linear);
		attr("quadratic", spot->quadratic);

		return entity;
	}

	engine::Entity Glare::render_point_light
	(
		engine::World& world,
		const graphics::WorldRenderState& render_state,
		graphics::Shader& shader,
		
		engine::Entity entity,
		const engine::LightComponent& light,
		engine::TransformComponent& transform,
		const engine::Relationship& relationship,
		
		unsigned int point_light_idx
	)
	{
		auto& registry = world.get_registry();

		auto light_transform = engine::Transform(registry, entity, relationship, transform);

		auto uniform_prefix = ("point_lights[" + std::to_string(point_light_idx) + "].");

		auto attr = [&](const std::string& attr_name, auto&& value) // std::string_view
		{
			shader[(uniform_prefix + attr_name)] = value;
		};

		auto light_position = light_transform.get_position();

		//std::cout << light_position.x << ',' << light_position.y << ',' << light_position.z << '\n';

		attr("position", light_position);
		
		//attr("ambient", light.properties.ambient);
		attr("diffuse", light.properties.diffuse);
		//attr("specular", light.properties.specular);

		auto* point_light = registry.try_get<engine::PointLightComponent>(entity);

		assert(point_light);

		// update attenuation parameters and calculate radius
		//const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)

		attr("linear", point_light->linear); // light.linear
		attr("quadratic", point_light->quadratic); // light.linear

		///*
		float radius = point_light->get_radius(light.properties);

		//std::cout << "Radius: " << radius << '\n';
		attr("radius", radius);
		//*/

		return entity;
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

					/*
					case GBufferDisplayMode::RenderFlags:
						if (gbuffer.render_flags.has_value())
						{
							return *gbuffer.render_flags;
						}

						break;
					*/
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
			//print("World: {}", world);
			print_children(world, world.get_root());

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

				auto position = t.get_position();
				auto rotation = math::degrees(t.get_rotation());
				auto scale = t.get_scale();

				std::cout << "\nCamera:\n";

				//std::cout << "Position: " << t.get_position() << '\n';
				//std::cout << "Rotation: " << math::degrees(t.get_rotation()) << '\n';
				//std::cout << "Scale: " << t.get_scale() << '\n';
				std::cout << "\"position\": { " << "\"x\": " << position.x << ", " << "\"y\": " << position.y << ", " << "\"z\": " << position.z << " }," << '\n';
				std::cout << "\"rotation\": { " << "\"x\": " << rotation.x << ", " << "\"y\": " << rotation.y << ", " << "\"z\": " << rotation.z << " }," << '\n';
				std::cout << "\"scale\": { " << "\"x\": " << scale.x << ", " << "\"y\": " << scale.y << ", " << "\"z\": " << scale.z << " }" << '\n';
			}

			//engine::SimpleFollowComponent::update(world);

			break;
		}

		case SDLK_g:
		{
			auto target_entity = world.get_by_name("Rotating Platform"); //world.get_player(1);

			if (target_entity != engine::null)
			{
				auto t = transform(target_entity);

				/*
				std::cout << "\nPlayer:\n";
				std::cout << "Position: " << t.get_position() << '\n';
				std::cout << "Rotation: " << math::degrees(t.get_rotation()) << '\n';
				std::cout << "Scale: " << t.get_scale() << '\n';
				*/

				auto& registry = world.get_registry();
				auto& m = registry.get<engine::ModelComponent>(target_entity);

				m.receives_shadow = !m.receives_shadow;
				//m.receives_light = !m.receives_light;
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
			auto& point_shadows = dynamic_texture_maps["point_shadow_cubemap"];

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