#pragma once

#include "render_phase.hpp"
#include "types.hpp"

#include <tuple>
#include <functional>

#include <util/memory.hpp>
//#include <graphics/types.hpp>

namespace engine
{
	class GeometryRenderPhase : public RenderPhase
	{
		protected:
			std::shared_ptr<graphics::Shader> geometry_pass;
			std::shared_ptr<graphics::Shader> geometry_pass_animated;
		public:
			GeometryRenderPhase(const std::shared_ptr<graphics::Shader>& geometry_pass, const std::shared_ptr<graphics::Shader>& geometry_pass_animated);
			GeometryRenderPhase(const std::shared_ptr<graphics::Context>& ctx, std::string_view shader_preprocessor);

			const RenderParameters& operator()(const RenderParameters& parameters);

			inline std::shared_ptr<graphics::Shader> get_shader() const
			{
				return geometry_pass;
			}

			inline std::shared_ptr<graphics::Shader> get_animated_shader() const
			{
				return geometry_pass_animated;
			}
	};

	//using BasicRenderPhase = GeometryRenderPhase;
}