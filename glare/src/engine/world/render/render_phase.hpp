#pragma once

#include "render_parameters.hpp"

#include <engine/types.hpp>
#include <engine/transform.hpp>

#include <graphics/types.hpp>
#include <graphics/canvas_draw_mode.hpp>

namespace game
{
	class Game;
}

namespace engine
{
	class World;
	class ResourceManager;
	class WorldRenderer;

	struct Resources;
	struct WorldRenderState;
	struct RenderScene;
	struct RenderParameters;

	struct ModelComponent;
	struct TransformComponent;
	struct RelationshipComponent;
	struct CameraComponent;

	class RenderPhase;

	// Base-class for rendering phases within a `RenderPipeline`.
	class RenderPhase
	{
		public:
			friend class WorldRenderer;

			using Canvas = graphics::Canvas;
		public:
			//virtual ~RenderPhase() = default;

			void draw_model
			(
				graphics::CanvasDrawMode draw_mode,
				graphics::Canvas& canvas,
				graphics::Shader& active_shader,

				World& world, // const World& world,

				Entity entity,
				ModelComponent& model_component,
				TransformComponent& transform,
				const RelationshipComponent& relationship,

				WorldRenderState* render_state=nullptr,

				bool bind_dynamic_textures=true,
				bool _auto_clear_textures=false // true
			);

			// Renders models with the given draw-mode.
			void draw_models
			(
				graphics::CanvasDrawMode draw_mode,
				
				const RenderScene& scene,

				// graphics::Shader& shader,
				
				const math::Matrix* projection_matrix=nullptr,
				const math::Matrix* view_matrix=nullptr,
				
				bool use_active_shader=false,
				
				WorldRenderState* render_state=nullptr,

				bool combine_matrices=false,
				bool bind_dynamic_textures=false
			);

			// Renders the scene using the last bound camera. If no camera has been bound/assinged, then this routine will return 'false'.
			// Returns 'false' if an essential rendering component is missing.
			bool render
			(
				const RenderScene& scene,
				const graphics::Viewport& viewport,
				bool multi_pass=false,
				bool use_active_shader=false,
				WorldRenderState* render_state=nullptr,
				graphics::CanvasDrawMode additional_draw_modes=graphics::CanvasDrawMode::None, // (graphics::CanvasDrawMode::IgnoreShaders)
				bool _combine_view_proj_matrices=false,
				bool _bind_dynamic_textures=false
			);

			// Renders the scene using the camera specified.
			// Returns 'false' if an essential rendering component is missing. (e.g. 'camera')
			bool render
			(
				const RenderScene& scene,
				const graphics::Viewport& viewport,
				Entity camera,
				bool multi_pass=false,
				bool use_active_shader=false,
				WorldRenderState* render_state=nullptr,
				graphics::CanvasDrawMode additional_draw_modes=graphics::CanvasDrawMode::None,
				bool _combine_view_proj_matrices=false,
				bool _bind_dynamic_textures=false
			);

			Transform get_camera_matrices
			(
				World& world,
				const graphics::Viewport& viewport,
				Entity camera, const engine::CameraComponent& camera_params,

				math::Matrix& projection, math::Matrix& view
			);

			Transform get_camera_matrices
			(
				World& world,
				const graphics::Viewport& viewport,
				Entity camera,

				math::Matrix& projection, math::Matrix& view
			);

			// Not required due to util::pipeline allowing for any return type to be passed forward, etc.
			//virtual const RenderParameters& operator()(const RenderParameters& parameters) = 0;
	};
}