#include "deferred_shading.hpp"

#include "render_scene.hpp"
#include "world_render_state.hpp"

//#include <graphics/types.hpp>
#include <graphics/canvas.hpp>
#include <graphics/context.hpp>
#include <graphics/gbuffer.hpp>

#include <engine/relationship.hpp>
#include <engine/transform.hpp>

#include <engine/world/world.hpp>
#include <engine/world/light.hpp>

#include <math/math.hpp>
#include <util/variant.hpp>

namespace engine
{
	DeferredShadingPhase::DeferredShadingPhase(const ref<graphics::Shader>& deferred_shading)
		: deferred_shading(deferred_shading) {}

	DeferredShadingPhase::DeferredShadingPhase(const ref<graphics::Context>& ctx, std::string_view shader_preprocessor)
		: DeferredShadingPhase
		(
			memory::allocate<graphics::Shader>
			(
				ctx,

				"assets/shaders/deferred_shading.vert",
				"assets/shaders/deferred_shading.frag",
				std::string {},

				shader_preprocessor
			)
		) {}

	const RenderParameters& DeferredShadingPhase::operator()(const RenderParameters& parameters)
	{
		auto& world        = parameters.scene.world;
		auto& canvas       = parameters.scene.canvas;
		auto& ctx          = parameters.scene.canvas.get_context();
		auto& gbuffer      = parameters.gbuffer;
		auto& viewport     = parameters.viewport;
		auto& render_state = parameters.render_state;

		auto& registry = world.get_registry();
		auto& shader = *this->deferred_shading;

		ctx.use(shader, [&, this]()
		{
			ctx.clear_textures(true); // false
			
			std::optional<decltype(ctx.use(*gbuffer.position, "g_position"))> gPosition =
				(gbuffer.position.has_value()) ? std::optional { ctx.use(*gbuffer.position, "g_position") } : std::nullopt;
			
			std::optional<decltype(ctx.use(*gbuffer.depth, "g_depth"))> gDepth =
				(gbuffer.depth.has_value()) ? std::optional { ctx.use(*gbuffer.depth, "g_depth") } : std::nullopt;

			auto gNormal     = ctx.use(gbuffer.normal, "g_normal");
			auto gAlbedoSpec = ctx.use(gbuffer.albedo_specular, "g_albedo_specular");

			std::optional<decltype(ctx.use(*gbuffer.render_flags, "g_render_flags"))> g_render_flags =
				(gbuffer.render_flags.has_value()) ? std::optional{ ctx.use(*gbuffer.render_flags, "g_render_flags") } : std::nullopt;

			if (!gbuffer.position.has_value())
			{
				assert(render_state.matrices.has_value());
				assert(render_state.screen.has_value());

				const auto& matrices = *render_state.matrices;
				const auto& screen = *render_state.screen;

				auto fov_y = screen.fov_y;

				auto aspect = screen.aspect_ratio;

				shader["inv_view"] = glm::inverse(matrices.view);
				//shader["inv_projection"] = glm::inverse(matrices.projection);
				//shader["inv_projview"] = glm::inverse(matrices.projection * matrices.view);

				shader["projection"] = matrices.projection;

				auto hs_near_plane = math::vec2{ (std::tan(fov_y / 2.0f) * aspect), (std::tan(fov_y / 2.0f)) };

				shader["half_size_near_plane"] = hs_near_plane;

				//auto depth_range = screen.depth_range;
				//shader["depth_range"] = math::vec2(0.0, 1.0); // depth_range
			}
			
			// Ambient light lookup is handled in previous phase, upload happens here:
			//shader["ambient_light"] = world.properties.ambient_light;

			if (render_state.meta.ambient_light.has_value())
			{
				shader["ambient_light"] = *render_state.meta.ambient_light;
			}

			if (render_state.meta.view_position.has_value())
			{
				shader["view_position"] = *render_state.meta.view_position;
			}


			// TODO: Split this phase into a subroutine, rather than taking up space in the main section.
			
			// Shadow-maps:
			std::size_t point_shadow_n_layers = 0;
			std::size_t directional_shadow_n_layers = 0;

			{
				// Point shadows:
				const auto& point_shadow_lp = render_state.point_shadows.light_positions;

				if (point_shadow_lp.has_value())
				{
					const auto& light_pos_v = *point_shadow_lp;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Vector*>(light_pos_v, [&](const graphics::Vector* vec)
					{
						assert(vec);
					
						shader["point_shadow_light_position"] = *vec;

						point_shadow_n_layers = 1;
					}))
					{}
					else if (util::peek_value<graphics::VectorArray*>(light_pos_v, [&](const graphics::VectorArray* vec)
					{
						assert(vec);

						shader["point_shadow_light_position"] = *vec;

						point_shadow_n_layers = vec->size();
					}))
					{}
				}

				const auto& point_shadow_fp = render_state.point_shadows.far_planes;

				if (point_shadow_fp.has_value())
				{
					const auto& far_v = *point_shadow_fp;

					// TODO: Implement as visit:
					if (util::peek_value<float*>(far_v, [&](const float* far_plane)
					{
						assert(far_plane);

						shader["point_shadow_far_plane"] = *far_plane;

						//point_shadow_n_layers = std::min(point_shadow_n_layers, static_cast<std::size_t>(1));
					}))
					{}
					else if (util::peek_value<graphics::FloatArray*>(far_v, [&](const graphics::FloatArray* far_plane)
					{
						assert(far_plane);

						shader["point_shadow_far_plane"] = *far_plane;

						//point_shadow_n_layers = std::min(point_shadow_n_layers, far_plane->size());
					}))
					{}
				}

				// Directional shadows:
				const auto& directional_shadow_lp = render_state.directional_shadows.light_positions;

				if (directional_shadow_lp.has_value())
				{
					const auto& light_pos_v = *directional_shadow_lp;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Vector*>(light_pos_v, [&](const graphics::Vector* vec)
					{
						assert(vec);
					
						shader["directional_shadow_light_position"] = *vec;

						directional_shadow_n_layers = 1;
					}))
					{}
					else if (util::peek_value<graphics::VectorArray*>(light_pos_v, [&](const graphics::VectorArray* vec)
					{
						assert(vec);

						shader["directional_shadow_light_position"] = *vec;

						directional_shadow_n_layers = vec->size();
					}))
					{}
				}

				const auto& directional_shadow_mat = render_state.directional_shadows.light_matrices;

				if (directional_shadow_mat.has_value())
				{
					const auto& mat_v = *directional_shadow_mat;

					// TODO: Implement as visit:
					if (util::peek_value<graphics::Matrix*>(mat_v, [&](const graphics::Matrix* matrix)
					{
						assert(matrix);

						shader["directional_shadow_light_space_matrix"] = *matrix;

						//directional_shadow_n_layers = std::min(directional_shadow_n_layers, static_cast<std::size_t>(1));
					}))
					{}
					else if (util::peek_value<graphics::MatrixArray*>(mat_v, [&](const graphics::MatrixArray* matrices)
					{
						assert(matrices);

						shader["directional_shadow_light_space_matrix"] = *matrices;

						//directional_shadow_n_layers = std::min(directional_shadow_n_layers, matrices->size());
					}))
					{}
				}
			}

			shader["directional_shadows_count"] = directional_shadow_n_layers;
			shader["point_shadows_count"] = point_shadow_n_layers;

			if (render_state.dynamic_textures)
			{
				// TODO: Look into this more.
				bool shadows_enabled = true; //!(draw_mode & DrawMode::IgnoreShadows);

				canvas.bind_textures(*render_state.dynamic_textures, [shadows_enabled](const std::string& texture_name, const graphics::TextureArrayRaw& textures)
				{
					if (!shadows_enabled) // tolower(...)
					{
						if ((texture_name.find("shadow") != std::string::npos))
						{
							// Don't bind this texture.
							return false;
						}
					}

					// Bind this texture.
					return true;
				}); // false
			}

			render_lights(world, render_state, shader);

			ctx.use(gbuffer.screen_quad, [&, this]()
			{
				ctx.draw();
			});

			if (!gbuffer.depth.has_value())
			{
				ctx.copy_framebuffer(gbuffer.framebuffer, viewport, viewport, graphics::BufferType::Depth);
			}
		});
		
		return parameters;
	}

	void DeferredShadingPhase::render_lights(World& world, const RenderState& render_state, graphics::Shader& shader)
	{
		auto& registry = world.get_registry();

		unsigned int directional_light_idx = 0;
		unsigned int spot_light_idx        = 0;
		unsigned int point_light_idx       = 0;

		// Enumerate lights, branching to each lighting subroutine based on the `LightComponent::type` field, incrementing light counters appropriately.
		registry.view<LightComponent, TransformComponent, Relationship>().each([&](auto entity, const auto& light, auto& transform, const auto& relationship) // const auto&
		{
			switch (light.type)
			{
				case LightType::Directional:
					if (render_directional_light(world, render_state, shader, entity, light, transform, relationship, directional_light_idx) != null)
					{
						directional_light_idx++;
					}

					break;
				case LightType::Spotlight:
					if (render_spot_light(world, render_state, shader, entity, light, transform, relationship, spot_light_idx) != null)
					{
						spot_light_idx++;
					}

					break;
				case LightType::Point:
					if (render_point_light(world, render_state, shader, entity, light, transform, relationship, point_light_idx) != null)
					{
						point_light_idx++;
					}

					break;
			}
		});

		// Indicate to the shader how many of each light-type is present.
		shader["directional_lights_count"] = directional_light_idx;
		shader["spot_lights_count"]        = spot_light_idx;
		shader["point_lights_count"]       = point_light_idx;
	}

	Entity DeferredShadingPhase::render_directional_light
	(
		World& world,
		const RenderState& render_state,
		graphics::Shader& shader,
		
		Entity entity,
		const LightComponent& light,
		TransformComponent& transform,
		const Relationship& relationship,
		
		unsigned int directional_light_idx
	)
	{
		auto& registry = world.get_registry();

		// A prefix string (array element) for accessing this light's data on the shader's end. (see `attr` below)
		auto uniform_prefix = ("directional_lights[" + std::to_string(directional_light_idx) + "].");

		// Helper lambda for assignment to the fully-qualified attribute name of this light.
		auto attr = [&](const std::string& attr_name, auto&& value) // std::string_view
		{
			shader[(uniform_prefix + attr_name)] = value;
		};

		// Retrieve the direction of the light based on its position, relative to the origin of the scene (position {0,0,0}):
		auto light_transform = Transform(registry, entity, relationship, transform);
		auto light_position = light_transform.get_position();
		auto light_direction = glm::normalize(-light_position); // light_transform.get_direction_vector();

		// Update attributes on the shader's side:
		attr("position",  light_position); // <-- TODO: Remove (Not needed for directional lights)
		attr("direction", light_direction);

		attr("ambient",   light.properties.ambient);
		attr("diffuse",   light.properties.diffuse);
		attr("specular",  light.properties.specular);

		// Retrieve the directional-light specific component to determine the `DirectionalLightComponent::use_position` flag:
		auto* dir = registry.try_get<DirectionalLightComponent>(entity);
		
		assert(dir);

		attr("use_position", dir->use_position);

		return entity;
	}

	Entity DeferredShadingPhase::render_spot_light
	(
		World& world,
		const RenderState& render_state,
		graphics::Shader& shader,
		
		Entity entity,
		const LightComponent& light,
		TransformComponent& transform,
		const Relationship& relationship,
		
		unsigned int spot_light_idx
	)
	{
		auto& registry = world.get_registry();

		// A prefix string (array element) for accessing this light's data on the shader's end. (see `attr` below)
		auto uniform_prefix = ("spot_lights[" + std::to_string(spot_light_idx) + "].");

		// Helper lambda for assignment to the fully-qualified attribute name of this light.
		auto attr = [&](const std::string& attr_name, auto&& value) // std::string_view
		{
			shader[(uniform_prefix + attr_name)] = value;
		};

		// Retrieve the position and direction vector of this spotlight:
		auto light_transform = Transform(registry, entity, relationship, transform);
		auto light_position = light_transform.get_position();
		auto light_direction = light_transform.get_direction_vector();

		// Update attributes on the shader's side:
		attr("position", light_position);
		attr("direction", light_direction);

		attr("ambient",  light.properties.ambient);
		attr("diffuse",  light.properties.diffuse);
		attr("specular", light.properties.specular);

		// Retrieve additional attributes specific to spotlights:
		auto* spot = registry.try_get<SpotLightComponent>(entity);
		
		assert(spot);

		attr("cutoff",       spot->cutoff);
		attr("outer_cutoff", spot->outer_cutoff);
		attr("constant",     spot->constant);
		attr("linear",       spot->linear);
		attr("quadratic",    spot->quadratic);

		return entity;
	}

	Entity DeferredShadingPhase::render_point_light
	(
		World& world,
		const RenderState& render_state,
		graphics::Shader& shader,
		
		Entity entity,
		const LightComponent& light,
		TransformComponent& transform,
		const Relationship& relationship,
		
		unsigned int point_light_idx
	)
	{
		auto& registry = world.get_registry();

		auto uniform_prefix = ("point_lights[" + std::to_string(point_light_idx) + "].");

		auto attr = [&](const std::string& attr_name, auto&& value) // std::string_view
		{
			shader[(uniform_prefix + attr_name)] = value;
		};

		// Retrieve the position of this point light:
		auto light_transform = Transform(registry, entity, relationship, transform);
		auto light_position = light_transform.get_position();

		// Update attributes on the shader's side:
		attr("position", light_position);
		
		attr("diffuse", light.properties.diffuse);
		//attr("ambient", light.properties.ambient);
		//attr("specular", light.properties.specular);

		// Retrieve additional attributes specific to point lights:
		auto* point_light = registry.try_get<PointLightComponent>(entity);

		assert(point_light);

		attr("linear",    point_light->linear); // light.linear
		attr("quadratic", point_light->quadratic); // light.linear
		attr("radius",    point_light->get_radius(light.properties));

		return entity;
	}
}