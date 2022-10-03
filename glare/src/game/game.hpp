#pragma once

#include <string_view>

#include <app/graphics_application.hpp>
#include <app/input/types.hpp>

#include "graphics.hpp"

#include <util/memory.hpp>

#include <graphics/types.hpp>

#include <engine/config.hpp>
#include <engine/input_events.hpp>
#include <engine/resource_manager/resource_manager.hpp>

#include <engine/world/world.hpp>
#include <engine/world/world_system.hpp>
#include <engine/system_manager.hpp>

#include <engine/world/render/world_renderer.hpp>

#include <util/shorthand.hpp>
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

	using WorldSystemManager = SystemManager<World>;

	class InputSystem;
}

namespace game
{
	using RenderState = engine::WorldRenderState;
	using RenderBuffer = graphics::GBuffer;

	class Game : public app::GraphicsApplication
	{
		protected:
			engine::Config cfg;

			Screen screen;
			Effects effects;

			engine::ResourceManager resource_manager;
			
			engine::World world;
			engine::WorldSystemManager systems;
			engine::WorldRenderer renderer;
		public:
			Game
			(
				std::string_view title,
				
				int width=1600, int height=900,
				UpdateRate update_rate=DEFAULT_FRAMERATE,

				bool vsync=true, bool lock_mouse=true,

				app::WindowFlags window_flags=(app::WindowFlags::OpenGL|app::WindowFlags::Resizable),
				bool imgui_enabled=true,

				unique_ref<engine::RenderPipeline>&& rendering_pipeline=nullptr
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

			void update(app::Milliseconds time) override;
			void render() override;

			entt::dispatcher* get_event_handler() override;
		protected:
			void init_default_systems(bool init_renderer=true);

			// Implement Game-specific logic using these:
			virtual void on_update(float delta) abstract;
			virtual void on_render(RenderState& render_state) {}
			virtual void on_resize(int width, int height) {}

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
				auto& input_system = system<engine::InputSystem>(world, input);

				generate_buttons(input_system, input);

				load_input_profiles(input_system, input);

				return input_system;
			}
	};
}