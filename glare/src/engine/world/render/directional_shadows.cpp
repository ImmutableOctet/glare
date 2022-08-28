#include "directional_shadows.hpp"

#include "render_scene.hpp"
#include "world_render_state.hpp"

#include <graphics/canvas.hpp>
#include <graphics/context.hpp>

#include <engine/world/world.hpp>
#include <engine/world/light.hpp>

#include <string>

namespace engine
{
	DirectionalShadowPhase::DirectionalShadowPhase(const ref<graphics::Shader>& depth_shader)
		: depth_shader(depth_shader) {}

	DirectionalShadowPhase::DirectionalShadowPhase(const ref<graphics::Context>& ctx, std::string_view shader_preprocessor)
		: DirectionalShadowPhase
		(
			memory::allocate<graphics::Shader>
			(
				ctx,

				"assets/shaders/shadow_mapping/directional_shadows_depth.vert",
				"assets/shaders/shadow_mapping/directional_shadows_depth.frag",
				std::string {},

				shader_preprocessor
			)
		) {}

	void DirectionalShadowPhase::clear()
	{
		positions.clear();
		matrices.clear();
	}

	const RenderParameters& DirectionalShadowPhase::operator()(const RenderParameters& parameters)
	{
		auto& render_state = parameters.render_state;

		// If point-light shadows aren't enabled, exit immediately.
		if (!render_state.directional_shadows.enabled)
		{
			return parameters;
		}

		if (!render_state.dynamic_textures)
		{
			return parameters;
		}

		auto& dynamic_textures = *render_state.dynamic_textures;
		auto& shadow_maps = dynamic_textures["directional_shadow_map"];

		auto& world = parameters.scene.world;
		auto& registry = world.get_registry();

		auto& ctx = parameters.scene.canvas.get_context();
		auto& shader = *this->depth_shader;

		// Clear internal state and registered shadow-maps.
		clear(); shadow_maps.clear();

		bool light_found = false;

		ctx.use(shader, [&, this]()
		{
			registry.view<LightComponent, DirectionLightShadows>().each([&](auto entity, LightComponent& light_component, DirectionLightShadows& shadows)
			{
				auto prev_viewport = ctx.get_viewport();
				auto viewport = shadows.shadow_map.get_viewport();
				auto& framebuffer = *(shadows.shadow_map.get_framebuffer());
				const auto& depth_texture = *(shadows.shadow_map.get_depth_map());

				auto light_tform = world.get_transform(entity);
				auto light_position = light_tform.get_position();

				//auto light_space_matrix = glm::ortho(-2048.0f, 2048.0f, -2048.0f, 2048.0f, 1.0f, 4000.0f) * glm::lookAt(light_position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

				const auto& light_space_matrix = shadows.transforms;
				
				/*
				auto camera_transform = get_transform(camera);

				const auto& light_space_matrix
					= camera_transform.get_camera_matrix();
					//= camera_transform.get_inverse_matrix();
				*/

				// Upload data about this light to the bound shader:
				shader["light_space_matrix"] = light_space_matrix;

				ctx.set_viewport(viewport);

				ctx.use(framebuffer, [&, this]()
				{
					ctx.clear(graphics::BufferType::Depth);

					//ctx.use(depth_texture, [&, this]
					{
						/*
						auto far_plane = shadows.shadow_perspective.far_plane;

						shader["far_plane"] = far_plane;

						shader["light_position"] = light_position;
						*/

						draw_models
						(
							(graphics::Canvas::DrawMode::Shadow), // graphics::Canvas::DrawMode::Opaque
							parameters.scene,
							{}, {},
							true
							// No need to pass the render state. -- TODO: Make sure this is accurate.
						);
					}//);
				});

				ctx.set_viewport(prev_viewport);

				light_found = true;

				// Store data about the directional-shadow-enabled light:
				shadow_maps.push_back(&depth_texture);

				this->positions.push_back(light_position);
				this->matrices.push_back(light_space_matrix);
			});
		});

		render_state.directional_shadows =
		{
			&this->positions,
			&this->matrices,

			light_found
		};

		//return light_found;
		return parameters;
	}
}