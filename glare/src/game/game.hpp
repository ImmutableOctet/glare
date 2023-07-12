#pragma once

#include <string_view>

#include <app/graphics_application.hpp>
#include <app/input/types.hpp>

#include "graphics.hpp"

#include <util/memory.hpp>

#include <graphics/types.hpp>

#include <engine/config.hpp>
#include <engine/input/raw_input_events.hpp>
#include <engine/resource_manager/resource_manager.hpp>

#include <engine/world/world.hpp>
#include <engine/world/world_system.hpp>

#include <engine/world/delta/delta_system_mode.hpp>

#include <engine/system_manager.hpp>

#include <engine/world/render/world_renderer.hpp>

#include <utility>

namespace graphics
{
	class GBuffer;
}

namespace engine
{
	struct WorldRenderState;

	//struct OnMouseState;
	//struct OnKeyboardState;

	class InputSystem;
}

namespace game
{
	using WorldSystemManager = engine::SystemManager<engine::World>;

	using RenderState = engine::WorldRenderState;
	using RenderBuffer = graphics::GBuffer;

	using DeltaSystemMode = engine::DeltaSystemMode;

	class Game : public app::GraphicsApplication
	{
		protected:
			engine::Registry registry;

			engine::Config cfg;

			Screen screen;
			Effects effects;

			engine::ResourceManager resource_manager;
			
			engine::World world;
			WorldSystemManager systems;
			engine::WorldRenderer renderer;
		public:
			Game
			(
				std::string_view title,
				
				int width=1600, int height=900,

				UpdateRate update_rate=DEFAULT_FRAMERATE,
				DeltaSystemMode delta_mode=DeltaSystemMode::FixedUpdate,

				bool vsync=true, bool input_lock_status=true,

				app::WindowFlags window_flags=(app::WindowFlags::OpenGL|app::WindowFlags::Resizable),

				bool imgui_enabled=true,

				std::unique_ptr<engine::RenderPipeline>&& rendering_pipeline=nullptr
			);

			// Alias for `emplace_system`; allocates a system internally and returns a reference to it.
			template <typename SystemType, typename...Args>
			inline SystemType& system(Args&&... args) { return systems.emplace_system<SystemType>(std::forward<Args>(args)...); }

			template <typename SystemType>
			inline SystemType& system() { return systems.emplace_system<SystemType>(); }

			template <typename WorldSystemType, typename...Args>
			inline WorldSystemType& world_system(Args&&... args) { return systems.emplace_system<WorldSystemType>(world, std::forward<Args>(args)...); }

			template <typename WorldSystemType>
			inline WorldSystemType& world_system() { return systems.emplace_system<WorldSystemType>(world); }

			template <typename BehaviorType>
			inline void behavior() { systems.register_behavior<BehaviorType>(); }

			inline engine::World& get_world() { return world; }

			// Proxy for underlying `Window` object's `set_title` member-function.
			// `title` must be zero-terminated.
			void set_title(std::string_view title);

			void lock_input();
			void unlock_input();
			void set_input_lock(bool value);
			void toggle_input_lock();
			bool is_input_locked() const;

			void update(app::Milliseconds time) override;
			void fixed_update(app::Milliseconds time) override;
			void render(app::Milliseconds time) override;

			entt::dispatcher* get_event_handler() override;

		protected:
			void init_default_systems(DeltaSystemMode delta_mode, bool init_renderer=true);

			// Implement Game-specific logic using these:
			virtual void on_update(float delta) = 0;
			virtual void on_fixed_update();
			virtual void on_render(RenderState& render_state);
			virtual void on_resize(int width, int height);

			virtual void on_build_input_system(engine::InputSystem& input_system, app::input::EngineButtonMap& buttons_out) {}

			virtual void on_window_resize(app::Window& window, int width, int height) override;

			/*
				This method is called during the `build_input_system` setup phase,
				and is intended to handle input profiles at the device-level.

				Default implementation loads from "config/input" directory, looking the following files:
				* "mouse.json"
				* "keyboard.json"
				* "gamepads.json"

				Feel free to override with custom logic, etc.
			*/
			virtual void load_input_profiles(engine::InputSystem& input_system, app::input::InputHandler& input_handler);

			RenderState& initialize_render_state(RenderState& render_state);

			// Initializes the engine's input system, calls `generate_buttons`, then calls `load_input_profiles`.
			// `engine::InputSystem` must be fully defined in order to use this template.
			template <typename generate_buttons_fn>
			inline engine::InputSystem& init_input_system(generate_buttons_fn&& generate_buttons)
			{
				auto& input_system = system<engine::InputSystem>(world, input, screen);

				generate_buttons(input_system, input);

				load_input_profiles(input_system, input);

				return input_system;
			}
	};
}