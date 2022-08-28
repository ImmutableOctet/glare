#pragma once

#include <string_view>

#include <app/graphics_application.hpp>

#include "graphics.hpp"

#include <graphics/types.hpp>

#include <engine/config.hpp>
#include <engine/resource_manager/resource_manager.hpp>

#include <engine/world/world.hpp>
#include <engine/world/physics.hpp>

#include <engine/world/render/world_renderer.hpp>

#include <util/shorthand.hpp>

namespace graphics
{
	class GBuffer;
}

namespace engine
{
	struct WorldRenderState;
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
			engine::PhysicsSystem physics;

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

			inline engine::World& get_world() { return world; }

			// Proxy for underlying `Window` object's `set_title` member-function.
			// `title` must be zero-terminated.
			void set_title(std::string_view title);
			
			void render() override;
			void update(app::Milliseconds time) override;

			// Implement Game-specific logic using these:
			virtual void on_update(float delta) abstract;
			virtual void on_render(RenderState& render_state) {}
			virtual void on_resize(int width, int height) {}
		protected:
			virtual void on_window_resize(app::Window& window, int width, int height) override;

			RenderState& initialize_render_state(RenderState& render_state);
	};
}