#pragma once

#include "render_phase.hpp"
#include "types.hpp"

#include <util/memory.hpp>
//#include <graphics/types.hpp>

#include <engine/types.hpp>

namespace engine
{
	struct Relationship;
	struct LightComponent;
	struct TransformComponent;
	
	// Handles deferred shading/lighting phase, using previous phases' gbuffer data and render-state context as input.
	class DeferredShadingPhase : public RenderPhase
	{
		protected:
			ref<graphics::Shader> deferred_shading;

		public:
			DeferredShadingPhase(const ref<graphics::Shader>& deferred_shading);
			DeferredShadingPhase(const ref<graphics::Context>& ctx, std::string_view shader_preprocessor);

			const RenderParameters& operator()(const RenderParameters& parameters);

		protected:
			// Lighting-related subroutines:
			void render_lights(World& world, const RenderState& render_state, graphics::Shader& shader);

			Entity render_directional_light
			(
				World& world,
				const RenderState& render_state,
				graphics::Shader& shader,

				Entity entity,
				const LightComponent& light,
				TransformComponent& transform,
				const Relationship& relationship,

				unsigned int directional_light_idx
			);

			Entity render_spot_light
			(
				World& world,
				const RenderState& render_state,
				graphics::Shader& shader,

				Entity entity,
				const LightComponent& light,
				TransformComponent& transform,
				const Relationship& relationship,

				unsigned int directional_light_idx
			);

			Entity render_point_light
			(
				World& world,
				const RenderState& render_state,
				graphics::Shader& shader,
				
				Entity entity,
				const LightComponent& light,
				TransformComponent& transform,
				const Relationship& relationship,
				
				unsigned int point_light_idx
			);
	};
}