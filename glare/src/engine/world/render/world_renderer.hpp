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
			unique_ref<RenderPipeline> pipeline;
		public:
			// Rendering pipelines can be created using `engine::make_render_pipeline`.
			WorldRenderer(const RenderScene& scene, unique_ref<RenderPipeline>&& pipeline);

			RenderBuffer& render
			(
				Entity camera,
				RenderBuffer& gbuffer,
				const RenderViewport& viewport,
				RenderState& render_state
			);

			inline RenderPipeline* get_pipeline() { return pipeline.get(); }
			inline const RenderPipeline* get_pipeline() const { return pipeline.get(); }

			inline operator bool() const { return static_cast<bool>(pipeline); } // { return (pipeline.get()); }
	};
}