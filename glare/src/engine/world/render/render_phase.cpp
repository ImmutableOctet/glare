#include "render_phase.hpp"
#include "world_render_state.hpp"
#include "render_scene.hpp"
#include "render_parameters.hpp"

#include <graphics/canvas.hpp>
#include <graphics/shader.hpp>
#include <graphics/gbuffer_render_flags.hpp>

#include <engine/world/world.hpp>
#include <engine/world/camera/components/camera_component.hpp>
#include <engine/world/animation/components/skeletal_pose_component.hpp>

#include <engine/config.hpp>
#include <engine/transform.hpp>
#include <engine/components/model_component.hpp>
#include <engine/components/relationship_component.hpp>

#include <engine/resource_manager/resource_manager.hpp>

namespace engine
{
	void RenderPhase::draw_model
	(
		graphics::CanvasDrawMode draw_mode,
		graphics::Canvas& canvas,
		graphics::Shader& active_shader,

		World& world,

		Entity entity,
		ModelComponent& model_component,
		TransformComponent& transform,
		const RelationshipComponent& relationship,

		WorldRenderState* render_state,

		bool bind_dynamic_textures,
		bool _auto_clear_textures
	)
	{
		auto& shader = active_shader;

		if (!model_component.visible)
		{
			return;
		}

		if ((draw_mode & graphics::CanvasDrawMode::Shadow) && (!model_component.casts_shadow))
		{
			return;
		}

		/*
		if (model_component.transparent())
		{
			return;
		}
		*/

		if (model_component.model == nullptr)
		{
			return;
		}

		auto& registry = world.get_registry();

		auto model_transform = Transform(registry, entity, relationship, transform);

		// TODO: Check if forcing a refresh is 100% necessary.
		auto model_matrix = model_transform.get_matrix(false); // true

		shader["model"] = model_matrix;

		auto& model = *model_component.model;
		auto color  = model_component.color;

		auto model_draw_mode = draw_mode;

		auto render_flags = graphics::GBufferRenderFlags::Default;

		if (!model_component.receives_shadow)
		{
			//render_flags &= ~graphics::GBufferRenderFlags::ShadowMap;
			render_flags = static_cast<graphics::GBufferRenderFlags>(0);
		}

		if (!model_component.receives_light)
		{
			render_flags &= ~graphics::GBufferRenderFlags::Lighting;
		}

		auto render_flags_raw = static_cast<std::uint8_t>(render_flags); // std::uint32_t

		shader["render_flags"] = render_flags_raw; // 0; // render_flags_raw;

		// TODO: Implement 'receives_shadow' flag using stencil buffer.
		/*
		if (model_component.receives_shadow)
		{
			// TODO: Refactor to N-textures.
			//shader["point_shadows_enabled"] = ((render_state) ? static_cast<bool>(render_state->point_shadows) : true);
			//shader["directional_shadows_enabled"] = ((render_state) ? static_cast<bool>(render_state->directional_shadows) : true);

			shader["point_shadows_count"] = point_shadow_n_layers;
			shader["directional_shadows_count"] = directional_shadow_n_layers;
		}
		else
		{
			model_draw_mode |= (graphics::CanvasDrawMode::IgnoreShadows);

			// TODO: Refactor to N-textures as 0.
			//shader["point_shadows_enabled"] = false;
			//shader["directional_shadows_enabled"] = false;

			shader["point_shadows_count"] = 0;
			shader["directional_shadows_count"] = 0;
		}
		*/

		if (const auto pose = registry.try_get<SkeletalPoseComponent>(entity))
		{
			const bool is_animated = (model_component.model->is_animated());
			
			shader["animated"] = is_animated;
			shader["bone_matrices"] = pose->get_pose();
		}
		else
		{
			shader["animated"] = false;
		}

		canvas.draw
		(
			model, color, model_draw_mode,
			_auto_clear_textures,
			(((render_state) && (bind_dynamic_textures)) ? render_state->dynamic_textures : nullptr)
		);
	}

	void RenderPhase::draw_models
	(
		graphics::CanvasDrawMode draw_mode,
		
		const RenderScene& scene,

		// graphics::Shader& shader,
		
		const math::Matrix* projection_matrix,
		const math::Matrix* view_matrix,
		
		bool use_active_shader,
		
		WorldRenderState* render_state,

		bool combine_matrices,
		bool bind_dynamic_textures
	)
	{
		auto& world = scene.world;
		auto& canvas = scene.canvas;
		auto& registry = world.get_registry();

		auto draw = [&, this](auto& shader)
		{
			if ((combine_matrices) && ((projection_matrix) && (view_matrix))) // ((projection_matrix != nullptr) && (view_matrix != nullptr))
			{
				shader["view_projection"] = ((*projection_matrix) * (*view_matrix));
			}
			else
			{
				if (projection_matrix)
				{
					shader["projection"] = *projection_matrix;
				}

				if (view_matrix)
				{
					shader["view"] = *view_matrix;
				}
			}

			if (render_state)
			{
				if (render_state->meta.view_position.has_value())
				{
					shader["view_position"] = *render_state->meta.view_position;
				}

				shader["height_map_min_layers"] = render_state->parallax.min_layers;
				shader["height_map_max_layers"] = render_state->parallax.max_layers;
			}

			registry.view<ModelComponent, TransformComponent, RelationshipComponent>().each
			(
				[this, draw_mode, &canvas, &shader, &world, &render_state, bind_dynamic_textures]
				(
					Entity entity,
					ModelComponent& model_component,
					TransformComponent& transform,
					const RelationshipComponent& relationship
				)
				{
					draw_model
					(
						draw_mode, canvas, shader,
						world,
						entity, model_component, transform, relationship,
						render_state, bind_dynamic_textures
					);
				}
			);
		};

		if ((use_active_shader) || (draw_mode & graphics::CanvasDrawMode::IgnoreShaders))
		{
			draw(canvas.get_shader());
		}
		else
		{
			auto& shaders = scene.resources.loaded_shaders;

			for (auto& shader_entry : shaders)
			{
				auto& shader_ptr = shader_entry.second;

				if (!shader_ptr)
				{
					continue;
				}

				auto& ctx = canvas.get_context();
				auto& shader = *shader_ptr;

				ctx.use(shader, [&, this]() { draw(shader); });
			}
		}
	}

	// Renders the scene using the last bound camera.
	// This essentially redirects to the main `render` implementation.
	bool RenderPhase::render
	(
		const RenderScene& scene,
		const graphics::Viewport& viewport,
		bool multi_pass,
		bool use_active_shader,
		WorldRenderState* render_state,
		graphics::CanvasDrawMode additional_draw_modes,
		bool _combine_view_proj_matrices,
		bool _bind_dynamic_textures
	)
	{
		return render
		(
			scene,
			viewport,
			scene.world.get_camera(),
			multi_pass,
			use_active_shader,
			render_state,
			additional_draw_modes,
			_combine_view_proj_matrices,
			_bind_dynamic_textures
		);
	}

	bool RenderPhase::render
	(
		const RenderScene& scene,
		const graphics::Viewport& viewport,
		Entity camera,
		bool multi_pass,
		bool use_active_shader,
		WorldRenderState* render_state,
		graphics::CanvasDrawMode additional_draw_modes,

		bool _combine_view_proj_matrices,
		bool _bind_dynamic_textures
	)
	{
		// Deferred referning is current unsupported.
		//assert(forward_rendering);

		if (camera == null)
		{
			return false;
		}

		auto& world = scene.world;
		auto& registry = world.get_registry();

		auto& camera_params = registry.get<engine::CameraComponent>(camera);

		// TODO: Move this aspect-ratio update to an event triggered on window-resize.
		///camera_params.aspect_ratio = window->horizontal_aspect_ratio();

		math::Matrix projection, camera_matrix;

		auto camera_transform = get_camera_matrices(world, viewport, camera, camera_params, projection, camera_matrix);

		if (render_state)
		{
			const auto& properties = world.get_properties();

			if (!render_state->meta.view_position.has_value())
			{
				auto camera_position = camera_transform.get_position();

				render_state->meta.view_position = camera_position;
			}

			if (!render_state->meta.ambient_light.has_value())
			{
				render_state->meta.ambient_light = properties.ambient_light;
			}
		}

		if (multi_pass)
		{
			draw_models((graphics::CanvasDrawMode::Opaque | additional_draw_modes), scene, &projection, &camera_matrix, use_active_shader, render_state, _combine_view_proj_matrices, _bind_dynamic_textures);

			// Projection & Camera matrices not passed-in due to already having been set in prior opaque pass.
			draw_models((graphics::CanvasDrawMode::Transparent | additional_draw_modes), scene, nullptr, nullptr, use_active_shader, render_state, _combine_view_proj_matrices, _bind_dynamic_textures);
		}
		else
		{
			draw_models((graphics::Canvas::DrawMode::All | additional_draw_modes), scene, &projection, &camera_matrix, use_active_shader, render_state, _combine_view_proj_matrices, _bind_dynamic_textures);
		}

		// Capture render state for next phase(s) in the deferred pipeline:
		if (render_state)
		{
			const auto& cfg = world.get_config();

			render_state->matrices =
			{
				.view       { camera_matrix },
				.projection { projection }
			};

			render_state->screen =
			{
				.fov_y        = camera_params.fov,
				.aspect_ratio = camera_params.aspect_ratio,
				.depth_range  = { camera_params.near_plane, camera_params.far_plane }
			};

			render_state->parallax =
			{
				.min_layers = cfg.graphics.parallax.min_layers,
				.max_layers = cfg.graphics.parallax.max_layers,
			};
		}

		return true;
	}

	Transform RenderPhase::get_camera_matrices(World& world, const graphics::Viewport& viewport, Entity camera, const engine::CameraComponent& camera_params, math::Matrix& projection, math::Matrix& view)
	{
		//auto& camera_transform = registry.get<engine::TransformComponent>(camera);
		//auto inverse_world = camera_transform.inverse_world;

		auto camera_transform = world.get_transform(camera);

		if (camera_params.free_rotation)
		{
			view = camera_transform.get_inverse_matrix();
		}
		else
		{
			view = camera_transform.get_camera_matrix();
		}

		projection = camera_params.get_projection(viewport);

		return camera_transform;
	}

	Transform RenderPhase::get_camera_matrices(World& world, const graphics::Viewport& viewport, Entity camera, math::Matrix& projection, math::Matrix& view)
	{
		auto& registry = world.get_registry();

		auto& camera_params = registry.get<engine::CameraComponent>(camera);

		return get_camera_matrices(world, viewport, camera, camera_params, projection, view);
	}
}