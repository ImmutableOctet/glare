#pragma once

#include "render_phase.hpp"
#include "types.hpp"

#include <util/memory.hpp>
#include <graphics/types.hpp>

namespace graphics
{
	class Context;
}

namespace engine
{
	class PointLightShadowPhase : public RenderPhase
	{
		protected:
			ref<graphics::Shader> point_shadow_depth;

			graphics::TextureArrayRaw shadow_maps; // std::vector<const Texture*>

			graphics::VectorArray light_positions; // std::vector<math::Vector>
			graphics::FloatArray  far_planes; // std::vector<float>

		public:
			PointLightShadowPhase(const ref<graphics::Shader>& point_shadow_depth);
			PointLightShadowPhase(const ref<graphics::Context>& ctx, std::string_view shader_preprocessor);

			void clear();

			const RenderParameters& operator()(const RenderParameters& parameters);
	};

	//using BasicRenderPhase = GeometryRenderPhase;
}