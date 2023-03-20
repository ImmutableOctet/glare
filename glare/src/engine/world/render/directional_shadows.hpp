#pragma once

#include "render_phase.hpp"
#include "types.hpp"

#include <graphics/array_types.hpp>

#include <memory>

namespace engine
{
	class DirectionalShadowPhase : public RenderPhase
	{
		protected:
			std::shared_ptr<graphics::Shader> depth_shader;

			graphics::VectorArray positions; // std::vector<math::Vector>
			graphics::MatrixArray matrices;

		public:
			DirectionalShadowPhase(const std::shared_ptr<graphics::Shader>& depth_shader);
			DirectionalShadowPhase(const std::shared_ptr<graphics::Context>& ctx, std::string_view shader_preprocessor);

			void clear();

			const RenderParameters& operator()(const RenderParameters& parameters);
	};

	//using BasicRenderPhase = GeometryRenderPhase;
}