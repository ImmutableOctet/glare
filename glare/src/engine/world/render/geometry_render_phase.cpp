#include "geometry_render_phase.hpp"
#include "render_scene.hpp"

#include <graphics/canvas.hpp>
#include <graphics/context.hpp>
#include <graphics/gbuffer.hpp>
#include <graphics/shader.hpp>

namespace engine
{
	GeometryRenderPhase::GeometryRenderPhase(const std::shared_ptr<graphics::Shader>& geometry_pass, const std::shared_ptr<graphics::Shader>& geometry_pass_animated)
		: geometry_pass(geometry_pass), geometry_pass_animated(geometry_pass_animated) {}

	GeometryRenderPhase::GeometryRenderPhase(const std::shared_ptr<graphics::Context>& ctx, std::string_view shader_preprocessor)
		: GeometryRenderPhase
		(
			std::make_shared<graphics::Shader>
			(
				ctx,

				"engine/shaders/geometry_pass.vert",
				"engine/shaders/geometry_pass.frag",
				std::string {},

				shader_preprocessor
			),

			std::make_shared<graphics::Shader>
			(
				ctx,

				"engine/shaders/geometry_pass.vert",
				"engine/shaders/geometry_pass.frag",
				std::string {},

				(std::string(shader_preprocessor) + "\n#define ANIMATION_ENABLED 1\n")
			)
		) {}

	const RenderParameters& GeometryRenderPhase::operator()(const RenderParameters& parameters)
	{
		auto& ctx = parameters.scene.canvas.get_context();
		auto& world = parameters.scene.world;

		//ctx.clear_textures(false); // true // <-- May not be needed.

		ctx.use(parameters.gbuffer.framebuffer, [&ctx, &parameters, this]()
		{
			ctx.clear(0.0f, 0.0f, 0.0f, 1.0f, (graphics::BufferType::Color | graphics::BufferType::Depth));

			// TODO: Confirm that we aren't missing anything by explicitly specifying animated vs. static here.

			// Regular meshes.
			ctx.use(*geometry_pass, [&, this]()
			{
				render(parameters.scene, parameters.viewport, parameters.camera, false, true, &parameters.render_state);
			});

			// Animated meshes.
			ctx.use(*geometry_pass_animated, [&, this]()
			{
				render(parameters.scene, parameters.viewport, parameters.camera, false, true, &parameters.render_state);
			});

			// Everything.
			///render(parameters.scene, parameters.viewport, parameters.camera, false, false, &parameters.render_state);
		});

		return parameters;
	}
}