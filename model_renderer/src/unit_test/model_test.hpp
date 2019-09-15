#pragma once

#include <core.hpp>
#include <math/math.hpp>

namespace unit_test
{
	class ModelTest : public app::GraphicsApplication
	{
		public:
			ref<graphics::Shader> test_shader;

			math::mat4 projection;
			math::mat4 view;
			math::mat4 model;

			ref<graphics::Model> loaded_model;

			ModelTest(bool auto_execute=true);

			void update() override;
			void render() override;
	};
}