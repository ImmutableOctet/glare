#pragma once

#include <engine/types.hpp>
#include "types.hpp"

namespace graphics
{
	class Canvas;
	class Shader;
	class GBuffer;
}

namespace engine
{
	struct WorldRenderState;
	struct RenderScene;
	struct RenderParameters;

	// Abstract base-class to render pipelines (combinations of `RenderPhase` objects).
	class RenderPipeline
	{
		public:
			// Enable virtual destruction.
			virtual ~RenderPipeline() = default;

			virtual const RenderParameters& render(const RenderParameters& parameters) = 0;
	};
}