#pragma once

#include <core.hpp>
#include <math/math.hpp>

namespace unit_test
{
	class DeferredTest : public app::GraphicsApplication
	{
		public:
			struct
			{
				graphics::Shader geometry_pass;
				graphics::Shader lighting_pass;
				graphics::Shader light_box;
			} shaders;

			graphics::Mesh screen_quad;
			graphics::FrameBuffer gBuffer;

			struct
			{
				Uniform<math::mat4> projection = {"projection"};
				Uniform<math::mat4> view = {"view"};
				Uniform<math::mat4> model = {"model"};
			} uniforms;

			ref<graphics::Model> model;

			DeferredTest(bool auto_execute=true);

			void update() override;
			void render() override;
	};
}