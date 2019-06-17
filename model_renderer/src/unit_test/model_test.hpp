#pragma once

#include <core.hpp>
#include <math/math.hpp>

namespace unit_test
{
	class ModelTest : public app::GraphicsApplication
	{
		public:
			template <typename T>
			using Var = graphics::ShaderVar<T>;

			ref<graphics::Shader> test_shader;

			Var<math::mat4> projection;
			Var<math::mat4> view;
			Var<math::mat4> model;

			ref<graphics::Model> loaded_model;

			ModelTest(bool auto_execute=true);

			void update() override;
			void render() override;
	};
}