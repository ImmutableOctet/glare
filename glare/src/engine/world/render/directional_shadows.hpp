#pragma once

#include "render_phase.hpp"
#include "types.hpp"

#include <util/memory.hpp>
#include <graphics/types.hpp>

namespace engine
{
	class DirectionalShadowPhase : public RenderPhase
	{
		protected:
			ref<graphics::Shader> depth_shader;

			graphics::TextureArrayRaw shadow_maps;

			graphics::VectorArray positions; // std::vector<math::Vector>
			graphics::MatrixArray matrices;

		public:
			DirectionalShadowPhase(const ref<graphics::Shader>& depth_shader);
			DirectionalShadowPhase(const ref<graphics::Context>& ctx, std::string_view shader_preprocessor);

			void clear();

			const RenderParameters& operator()(const RenderParameters& parameters);
	};

	//using BasicRenderPhase = GeometryRenderPhase;
}