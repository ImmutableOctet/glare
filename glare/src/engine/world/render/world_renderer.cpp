#include <utility>

#include "world_renderer.hpp"
#include "world_render_state.hpp"
#include "render_parameters.hpp"

#include <graphics/gbuffer.hpp>
#include <engine/world/world.hpp>

namespace engine
{
	WorldRenderer::WorldRenderer(const RenderScene& scene, std::unique_ptr<RenderPipeline>&& pipeline)
		: scene(scene), pipeline(std::move(pipeline)) {}

	RenderBuffer& WorldRenderer::render
	(
		Entity camera,
		RenderBuffer& gbuffer,
		const RenderViewport& viewport,
		RenderState& render_state
	)
	{
		if (camera == null)
		{
			return gbuffer;
		}

		auto& world = scene.world;

		auto camera_transform = world.get_transform(camera);
		auto view_position = camera_transform.get_position();

		render_state.meta.view_position = view_position;

		if (!pipeline)
		{
			return gbuffer;
		}

		pipeline->render({ scene, camera, gbuffer, viewport, render_state });

		/*
		// TODO: Replace/rework.
		{
			// Execute shadow-pass first.
			auto [point_shadows_enabled, directional_shadows_enabled] = render_shadows();

			// Update render state:
			render_state.directional_shadows.enabled = directional_shadows_enabled;

			// Geometry pass.
			auto& gbuffer = render_geometry(world, viewport, screen.gbuffer, render_state);

			//graphics.context->clear(0.0f, 0.0f, 0.0f, 1.0f, BufferType::Color | BufferType::Depth);

			render_scene(world, viewport, gbuffer, render_state);
		}
		*/

		return gbuffer;
	}
}