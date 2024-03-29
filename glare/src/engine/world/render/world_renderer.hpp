#pragma once

#include <graphics/types.hpp>
#include <engine/types.hpp>

#include "types.hpp"
#include "render_scene.hpp"
#include "render_phase.hpp"
#include "render_parameters.hpp"
#include "render_pipeline.hpp"

namespace game
{
	class Game;
}

namespace engine
{
	class World;
	class ResourceManager;
	class RenderPhase;

	struct Resources;
	struct WorldRenderState;

	class WorldRenderer;

	class WorldRenderer
	{
		protected:
			//using Canvas = graphics::Canvas;

			//friend class game::Game;
			//friend class RenderPhase;

			// Description of the scene we're rendering.
			RenderScene scene;

			// The rendering pipeline (collection of rendering phases) used to process `scene`.
			std::unique_ptr<RenderPipeline> pipeline;
		public:
			// Rendering pipelines can be created using `engine::make_render_pipeline`.
			WorldRenderer(const RenderScene& scene, std::unique_ptr<RenderPipeline>&& pipeline);

			RenderBuffer& render
			(
				Entity camera,
				RenderBuffer& gbuffer,
				const RenderViewport& viewport,
				RenderState& render_state
			);

			inline void set_pipeline(std::unique_ptr<RenderPipeline>&& pipeline)
			{
				this->pipeline = std::move(pipeline);
			}

			inline RenderPipeline* get_pipeline() { return pipeline.get(); }
			inline const RenderPipeline* get_pipeline() const { return pipeline.get(); }

			inline bool has_pipeline() const { return static_cast<bool>(pipeline); }
			inline explicit operator bool() const { return has_pipeline(); } // { return (pipeline.get()); }
	};
}