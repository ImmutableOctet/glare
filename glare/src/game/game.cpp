#include "game.hpp"

#include <engine/world/render/world_render_state.hpp>
#include <engine/world/render/deferred_render_pipeline.hpp>
#include <engine/world/render/render_scene.hpp>

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

		physics(world),

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

		if (!renderer)
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

					engine::DeferredShadingPhase { graphics.context, effects.get_preprocessor() }
				)
			);
		}
	}

	void Game::set_title(std::string_view title)
	{
		if (!window)
		{
			return;
		}

		window->set_title(title);
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

	void Game::update(app::Milliseconds time)
	{
		auto& mouse = input.get_mouse();

		if (mouse.locked())
		{
			input.poll(world.get_event_handler());
		}

		world.update(time);

		on_update(world.get_delta_time());
	}
}