#include "point_light_shadows.hpp"

#include "render_scene.hpp"
#include "world_render_state.hpp"

#include <util/memory.hpp>

#include <graphics/canvas.hpp>
#include <graphics/context.hpp>
#include <graphics/shader.hpp>

#include <engine/world/world.hpp>

#include <engine/world/components/light_component.hpp>
#include <engine/world/components/point_light_shadow_component.hpp>

namespace engine
{
	PointLightShadowPhase::PointLightShadowPhase(const ref<graphics::Shader>& point_shadow_depth)
		: point_shadow_depth(point_shadow_depth) {}

	PointLightShadowPhase::PointLightShadowPhase(const ref<graphics::Context>& ctx, std::string_view shader_preprocessor)
		: PointLightShadowPhase
		(
			memory::allocate<graphics::Shader>
			(
				ctx,

				"engine/shaders/shadow_mapping/point_shadows_depth.vert",
				"engine/shaders/shadow_mapping/point_shadows_depth.frag",
				"engine/shaders/shadow_mapping/point_shadows_depth.geom",

				shader_preprocessor
			)
		) {}

	void PointLightShadowPhase::clear()
	{
		light_positions.clear();
		far_planes.clear();
	}

	const RenderParameters& PointLightShadowPhase::operator()(const RenderParameters& parameters)
	{
		auto& render_state = parameters.render_state;

		// If point-light shadows aren't enabled, exit immediately.
		if (!render_state.point_shadows.enabled)
		{
			return parameters;
		}

		if (!render_state.dynamic_textures)
		{
			return parameters;
		}

		auto& dynamic_textures = *render_state.dynamic_textures;

		auto& shadow_maps = dynamic_textures["point_shadow_cubemap"];

		auto& world = parameters.scene.world;
		auto& registry = world.get_registry();

		auto& ctx = parameters.scene.canvas.get_context();
		auto& shader = *this->point_shadow_depth;

		// Clear internal state and registered shadow-maps.
		clear(); shadow_maps.clear();

		bool light_found = false;

		ctx.use(shader, [&, this]()
		{
			registry.view<LightComponent, PointLightShadowComponent>().each([&](auto entity, LightComponent& light_component, PointLightShadowComponent& shadows)
			{
				auto prev_viewport = ctx.get_viewport();
				auto viewport = shadows.shadow_map.get_viewport();
				auto& framebuffer = *shadows.shadow_map.get_framebuffer();

				auto light_tform = world.get_transform(entity);
				auto light_position = light_tform.get_position();

				ctx.set_viewport(viewport);

				ctx.use(framebuffer, [&, this]()
				{
					ctx.clear(graphics::BufferType::Depth);

					const auto& tforms = shadows.transforms;

					// Upload data about this light to the bound shader:
					for (std::size_t i = 0; i < tforms.size(); ++i) // auto
					{
						shader["cube_matrices[" + std::to_string(i) + "]"] = tforms.transforms[i];
					}

					auto far_plane = shadows.shadow_perspective.far_plane;

					shader["far_plane"] = far_plane;
					shader["light_position"] = light_position;

					draw_models
					(
						(graphics::Canvas::DrawMode::Shadow), // graphics::Canvas::DrawMode::Opaque
						parameters.scene,
						{}, {},
						true
						// No need to pass the render state. -- TODO: Make sure this is accurate.
					);

					light_found = true;

					// Store data about the point-shadow-enabled light:
					shadow_maps.push_back(shadows.shadow_map.get_depth_map().get());

					this->light_positions.push_back(light_position);
					this->far_planes.push_back(far_plane);
				});

				ctx.set_viewport(prev_viewport);
			});
		});

		// Bind the current render-state for point-light shadows:
		render_state.point_shadows =
		{
			&this->light_positions,
			&this->far_planes,

			light_found
		};

		return parameters;
	}
}