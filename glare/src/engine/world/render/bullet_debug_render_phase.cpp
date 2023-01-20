#include "bullet_debug_render_phase.hpp"

#include "render_scene.hpp"
#include "world_render_state.hpp"

#include <graphics/canvas.hpp>
#include <graphics/context.hpp>
#include <graphics/shader.hpp>

#include <engine/world/world.hpp>
#include <engine/world/components/camera_component.hpp>

#include <graphics/gbuffer.hpp>

namespace engine
{
	BulletDebugRenderPhase::BulletDebugRenderPhase(const std::shared_ptr<graphics::Shader>& debug_lines_shading)
		: debug_lines_shading(debug_lines_shading), debug_drawer(std::make_unique<graphics::BulletDebugDrawer>())
	{}

	BulletDebugRenderPhase::BulletDebugRenderPhase(const std::shared_ptr<graphics::Context>& ctx, std::string_view shader_preprocessor)
		: BulletDebugRenderPhase
		(
			memory::allocate<graphics::Shader>
			(
				ctx,

				"engine/shaders/debug_lines.vert",
				"engine/shaders/debug_lines.frag",
				std::string {},

				shader_preprocessor
				)
		) {}

	const RenderParameters& BulletDebugRenderPhase::operator()(const RenderParameters& parameters)
	{
		if (!is_enabled())
		{
			return parameters;
		}

		if (!debug_drawer->has_points())
		{
			return parameters;
		}

		auto& world = parameters.scene.world;
		auto& canvas = parameters.scene.canvas;
		auto& ctx = parameters.scene.canvas.get_context();
		auto& gbuffer = parameters.gbuffer;
		auto& viewport = parameters.viewport;
		auto& render_state = parameters.render_state;

		auto& registry = world.get_registry();
		auto& shader = *this->debug_lines_shading;
		
		auto camera = parameters.camera;

		if (camera == null)
		{
			return parameters;
		}

		math::Matrix projection, view;

		get_camera_matrices(world, viewport, camera, projection, view);

		ctx.use(shader, [&, this]()
		{
			auto& point_data = debug_drawer->get_gpu_state();

			ctx.set_flags(graphics::ContextFlags::DepthTest, false);

			shader["view"] = view;
			shader["projection"] = projection;

			ctx.use(point_data, [&]()
			{
				ctx.draw();
			});

			ctx.set_flags(graphics::ContextFlags::DepthTest, true);
		});

		return parameters;
	}

	void BulletDebugRenderPhase::enable()
	{
		debug_drawer->enable();
	}

	void BulletDebugRenderPhase::disable()
	{
		debug_drawer->disable();
	}

	bool BulletDebugRenderPhase::is_enabled() const
	{
		return debug_drawer->is_enabled();
	}
}