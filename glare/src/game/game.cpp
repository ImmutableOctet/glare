#include "game.hpp"

#include <app/input/profile_metadata.hpp>
#include <app/input/input_handler.hpp>
#include <app/input/mouse.hpp>
#include <app/input/keyboard.hpp>
#include <app/input/gamepad.hpp>

#include <engine/editor/editor.hpp>

#include <engine/entity/entity_system.hpp>

#include <engine/world/physics/physics.hpp>
#include <engine/world/motion/motion.hpp>
#include <engine/world/animation/animation_system.hpp>
#include <engine/world/camera/camera_system.hpp>
#include <engine/world/delta/delta_system.hpp>

#include <engine/world/render/world_render_state.hpp>
#include <engine/world/render/deferred_render_pipeline.hpp>
#include <engine/world/render/render_scene.hpp>

#include <engine/input/input_system.hpp>

#include <engine/meta/reflect_all.hpp>

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
		std::string_view title,
		
		int width, int height,
		
		UpdateRate update_rate,
		DeltaSystemMode delta_mode,
		
		bool vsync, bool input_lock_status,
		
		app::WindowFlags window_flags,

		bool imgui_enabled,

		std::unique_ptr<engine::RenderPipeline>&& rendering_pipeline
	) :
		GraphicsApplication(title, width, height, window_flags, update_rate, vsync, imgui_enabled),
		
		cfg(),
		
		screen(graphics.context, window->get_size()),
		
		resource_manager(graphics.context),
		world(registry, systems, cfg, resource_manager, update_rate),
		systems(world),

		// May split this out in body of constructor. -- This would
		// allow for a different renderer (forward, etc.)
		renderer
		(
			engine::RenderScene { *graphics.canvas, world, resource_manager },
			std::move(rendering_pipeline)
		)
	{
		// Generate reflection data for the `engine` module.
		engine::reflect_all();

		set_input_lock(input_lock_status);

		init_default_systems(delta_mode, (!renderer));
	}

	void Game::set_title(std::string_view title)
	{
		if (!window)
		{
			return;
		}

		window->set_title(title);
	}

	void Game::lock_input()
	{
		input.get_mouse().lock();
		input.set_lock_status(true);
	}

	void Game::unlock_input()
	{
		input.get_mouse().unlock();
		input.set_lock_status(false);
	}

	void Game::set_input_lock(bool value)
	{
		if (value)
		{
			lock_input();
		}
		else
		{
			unlock_input();
		}
	}

	void Game::toggle_input_lock()
	{
		set_input_lock(!is_input_locked());
	}

	bool Game::is_input_locked() const
	{
		return input.get_lock_status();
	}

	void Game::update(app::Milliseconds time)
	{
		auto& mouse = input.get_mouse();

		auto& event_handler = world.get_active_event_handler();

		input.poll(&event_handler);

		world.update(time);

		on_update(world.get_delta_time());
	}

	void Game::fixed_update(app::Milliseconds time)
	{
		world.fixed_update(time);

		on_fixed_update();
	}

	void Game::render(app::Milliseconds time)
	{
		using namespace graphics;

		auto camera_system = systems.get_system<engine::CameraSystem>();

		if (!camera_system)
		{
			return;
		}

		const auto camera = camera_system->get_active_camera(); // world.get_camera();

		if (camera == engine::null)
		{
			return;
		}

		auto [viewport, window_size] = screen.update_viewport(*window, world, camera);

		camera_system->update_active_camera_viewport(viewport, { std::get<0>(window_size), std::get<1>(window_size) });

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

	void Game::on_fixed_update() {}
	void Game::on_render(RenderState& render_state) {}
	void Game::on_resize(int width, int height) {}

	void Game::on_window_resize(app::Window& window, int width, int height)
	{
		if (this->window.get() != &window)
		{
			return;
		}

		/*
		if (auto camera_system = systems.get_system<engine::CameraSystem>())
		{
			// TODO: Rework this interface.
			camera_system->update_camera_parameters(width, height);
		}
		*/

		screen.resize(width, height);
		on_resize(width, height);
	}

	void Game::on_execute()
	{
		util::log::register_debug_data(*this);
	}

	void Game::on_shutdown()
	{
		util::log::unregister_debug_data(*this);
	}

	RenderState& Game::initialize_render_state(RenderState& render_state)
	{
		//render_state = {};

		effects.bind(render_state, cfg);

		if (cfg.graphics.shadows.enabled)
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

	std::uint64_t Game::get_log_frame_number() const
	{
		return static_cast<std::uint64_t>(get_update_counter());
	}

	std::uint64_t Game::get_log_time_ms() const
	{
		return static_cast<std::uint64_t>(milliseconds());
	}

	void Game::init_default_systems(DeltaSystemMode delta_mode, bool init_renderer)
	{
		//auto& resource_manager = world.get_resource_manager();

		auto& entity_system = world_system<engine::EntitySystem>(systems);

		world_system<engine::DeltaSystem>(entity_system, get_update_rate(), delta_mode);

		world_system<engine::CameraSystem>();

		auto& physics = world_system<engine::PhysicsSystem>();

		world_system<engine::MotionSystem>(physics);

		world_system<engine::AnimationSystem>();

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