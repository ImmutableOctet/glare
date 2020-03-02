#pragma once

#include <core.hpp>
#include <math/math.hpp>

#include <engine/engine.hpp>
#include <engine/world/debug/debug.hpp>

namespace unit_test
{
	class ModelTest : public app::GraphicsApplication
	{
		public:
			ref<graphics::Shader> test_shader;

			math::mat4 projection;
			math::mat4 view;
			math::mat4 model;

			struct
			{
				Uniform<math::mat4> projection = { "projection" };
				Uniform<math::mat4> view = { "view" };
				Uniform<math::mat4> model = { "model" };
			} uniforms;

			ref<graphics::Model> loaded_model;

			engine::World world;
			engine::Entity& camera = world.cameras[0];

			ModelTest(bool auto_execute=true);

			void update() override;
			void render() override;

			void on_keyup(const keyboard_event_t& event) override;
	};
}