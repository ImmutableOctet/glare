#include "game.hpp"

#include <app/input/profile_metadata.hpp>
#include <app/input/input_handler.hpp>
#include <app/input/mouse.hpp>
#include <app/input/keyboard.hpp>
#include <app/input/gamepad.hpp>

#include <engine/world/physics/physics.hpp>
#include <engine/world/motion/motion.hpp>
#include <engine/world/animation/animation.hpp>
#include <engine/world/zones/zones.hpp>

#include <engine/world/render/world_render_state.hpp>
#include <engine/world/render/deferred_render_pipeline.hpp>
#include <engine/world/render/render_scene.hpp>

#include <engine/input/input_system.hpp>

// Debugging related:
#include <engine/world/render/bullet_debug_render_phase.hpp>

// TODO: Automate behavior registration in some way.
// (May also be applicable to systems)
// Behaviors:
#include <engine/world/behaviors/free_look_behavior.hpp>
#include <engine/world/behaviors/debug_move_behavior.hpp>

#include <utility>

namespace game
{
	Game::Game
	(
		std::string_view title, int width, int height,
		UpdateRate update_rate, bool vsync, bool lock_mouse,
		app::WindowFlags window_flags,
		bool imgui_enabled,

		unique_ref<engine::RenderPipeline>&& rendering_pipeline
	) :
		GraphicsApplication(title, width, height, window_flags, update_rate, vsync, imgui_enabled),
		
		cfg(),
		
		screen(graphics.context, window->get_size()),
		
		resource_manager(graphics.context),
		world(cfg, resource_manager, update_rate),
		systems(world),

		// May split this out in body of constructor. -- This would
		// allow for a different renderer (forward, etc.)
		renderer
		(
			engine::RenderScene { *graphics.canvas, world, resource_manager },
			std::move(rendering_pipeline)
		)
	{
		if (lock_mouse)
		{
			input.get_mouse().lock();
		}

		init_default_systems((!renderer));
	}

	void Game::set_title(std::string_view title)
	{
		if (!window)
		{
			return;
		}

		window->set_title(title);
	}

	void Game::update(app::Milliseconds time)
	{
		auto& mouse = input.get_mouse();

		// Currently based on mouse's lock-status.
		// (May change this in the future)
		if (mouse.locked())
		{
			auto& event_handler = world.get_active_event_handler();
			
			input.poll(&event_handler);
		}

		world.update(time);

		on_update(world.get_delta_time());
	}

	void Game::render()
	{
		using namespace graphics;

		auto camera = world.get_camera();

		if (camera == engine::null)
		{
			return;
		}

		auto [viewport, window_size] = screen.update_viewport(*window, world, camera);

		// Backbuffer clear; for gbuffer, see `Screen::render`, etc.
		graphics.context->clear(effects.clear_color.r, effects.clear_color.g, effects.clear_color.b, effects.clear_color.a, BufferType::Color|BufferType::Depth);

		if (!renderer)
		{
			return;
		}

		RenderState render_state;
		initialize_render_state(render_state);

		renderer.render(camera, screen, viewport, render_state);

		// Execute rendering events.
		world.render(graphics);

		auto& gbuffer = screen.render();

		on_render(render_state);
	}

	void Game::on_window_resize(app::Window& window, int width, int height)
	{
		if (this->window.get() != &window)
		{
			return;
		}

		world.update_camera_parameters(width, height);

		screen.resize(width, height);
		on_resize(width, height);
	}

	RenderState& Game::initialize_render_state(RenderState& render_state)
	{
		//render_state = {};

		effects.bind(render_state, cfg);

		if (cfg.graphics.shadow.enabled)
		{
			render_state.directional_shadows.enabled = true;
			render_state.point_shadows.enabled = true;
		}

		return render_state;
	}

	void Game::load_input_profiles(engine::InputSystem& input_system, app::input::InputHandler& input_handler)
	{
		auto& keyboard = input_handler.get_keyboard();

		keyboard.load_profile
		(
			{
				// Path:
				.path = std::filesystem::path("config/input/keyboard.json"),

				// Input:
				.buttons = input_handler.get_buttons(),
				.analogs = input_handler.get_analogs(),

				// Output:
				.player_mappings_out = input_handler.get_player_device_map()
			}
		);

		auto& mouse = input_handler.get_mouse();
		
		mouse.load_profile
		(
			{
				// Path:
				.path = std::filesystem::path("config/input/mouse.json"),

				// Input:
				.buttons = input_handler.get_buttons(),
				.analogs = input_handler.get_analogs(),

				// Output:
				.player_mappings_out = input_handler.get_player_device_map()
			}
		);

		auto& gamepads = input_handler.get_gamepads();

		gamepads.load_profiles
		(
			{
				// Path:
				.path = std::filesystem::path("config/input/gamepads.json"),

				// Input:
				.buttons = input_handler.get_buttons(),
				.analogs = input_handler.get_analogs(),

				// Output:
				.player_mappings_out = input_handler.get_player_device_map()
			}
		);
	}

	entt::dispatcher* Game::get_event_handler()
	{
		return &(world.get_active_event_handler());
	}

	void Game::init_default_systems(bool init_renderer)
	{
		// World systems:
		auto& physics = world_system<engine::PhysicsSystem>();
		world_system<engine::MotionSystem>(physics);

		world_system<engine::AnimationSystem>();
		world_system<engine::ZoneSystem>();

		// Behaviors:
		behavior<engine::FreeLookBehavior>();
		behavior<engine::DebugMoveBehavior>();

		//behavior<engine::TargetBehavior>();
		//behavior<engine::BillboardBehavior>();
		/*
		// TODO: Look into which behaviors we want to enable by default.
		// TODO: Look into auto-detecting construction of certain component types and initializing behaviors/systems automatically.
		system<engine::SpinBehavior>(world);
		system<engine::TargetComponent>(world);
		system<engine::SimpleFollowComponent>(world);
		system<engine::BillboardBehavior>(world);
		system<engine::RaveComponent>(world);
		*/

		if (init_renderer)
		{
			renderer.set_pipeline
			(
				engine::make_render_pipeline
				(
					engine::PointLightShadowPhase  { graphics.context, effects.get_preprocessor() },
					engine::DirectionalShadowPhase { graphics.context, effects.get_preprocessor() },
				
					util::inspect_and_store
					(
						engine::GeometryRenderPhase { graphics.context, effects.get_preprocessor() },

						[this](auto&& geometry_phase)
						{
							this->resource_manager.set_default_shader(geometry_phase.get_shader());
							this->resource_manager.set_default_animated_shader(geometry_phase.get_animated_shader());
						}
					),

					engine::DeferredShadingPhase { graphics.context, effects.get_preprocessor() },

					util::inspect_and_store
					(
						engine::BulletDebugRenderPhase { graphics.context, effects.get_preprocessor() },
						[this, &physics](auto& bullet_dbg)
						{
							physics.register_debug_drawer(bullet_dbg.get_debug_drawer());

							//bullet_dbg.disable();
						}
					)
				)
			);
		}
	}
}