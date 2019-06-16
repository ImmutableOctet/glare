#pragma once

#include <core.hpp>
#include <math/math.hpp>

namespace unit_test
{
	class ShaderTest : public app::GraphicsApplication
	{
		public:
			template <typename T>
			using Var = graphics::ShaderVar<T>;

			ref<graphics::Shader> test_shader;
			
			graphics::Texture texture1;
			graphics::Texture texture2;

			Var<int> texture1_sampler;
			Var<int> texture2_sampler;

			Var<math::mat4> projection;
			Var<math::mat4> view;
			Var<math::mat4> model;

			engine::Camera camera;

			//graphics::ShaderVar<math::vec3> color;

			graphics::Mesh cube;

			ShaderTest(bool auto_execute=true);

			void update() override;
			void render() override;
	};
}