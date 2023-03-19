#pragma once

#include "render_phase.hpp"
#include "types.hpp"

#include <graphics/array_types.hpp>

#include <memory>

namespace graphics
{
	class Context;
}

namespace engine
{
	class PointLightShadowPhase : public RenderPhase
	{
		protected:
			std::shared_ptr<graphics::Shader> point_shadow_depth;

			graphics::VectorArray light_positions; // std::vector<math::Vector>
			graphics::FloatArray  far_planes; // std::vector<float>

		public:
			PointLightShadowPhase(const std::shared_ptr<graphics::Shader>& point_shadow_depth);
			PointLightShadowPhase(const std::shared_ptr<graphics::Context>& ctx, std::string_view shader_preprocessor);

			void clear();

			const RenderParameters& operator()(const RenderParameters& parameters);
	};

	//using BasicRenderPhase = GeometryRenderPhase;
}