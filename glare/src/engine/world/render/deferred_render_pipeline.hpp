#pragma once

#include "render_pipeline_impl.hpp"
#include "render_phase.hpp"

#include "geometry_render_phase.hpp"
#include "point_light_shadows.hpp"
#include "directional_shadows.hpp"
#include "deferred_shading.hpp"

namespace engine
{
	/*
	template <typename... PreRenderPhases>
	using DeferredRenderPipeline = RenderPipelineImpl
	<
		PreRenderPhases..., GeometryRenderPhase, DeferredShadingPhase
	>;

	using ShadowMappedDeferredRenderPipeline = RenderPipelineImpl
	<
		PointLightShadowPhase,
		DirectionalShadowPhase,
		GeometryRenderPhase,
		DeferredShadingPhase
	>;
	*/
}