#pragma once

#include <core.hpp>
#include <math/math.hpp>

namespace unit_test
{
	class ShaderTest : public app::GraphicsApplication
	{
		public:
			ref<graphics::Shader> test_shader;
			
			graphics::Texture texture1;
			graphics::Texture texture2;

			struct
			{
				Uniform<math::mat4> projection = {"projection"};
				Uniform<math::mat4> view = {"view"};
				Uniform<math::mat4> model = {"model"};
			} uniforms;

			//graphics::ShaderVar<math::vec3> color;

			graphics::Mesh cube;
			graphics::Mesh quad;

			ShaderTest(bool auto_execute=true);

			void update() override;
			void render() override;
	};
}