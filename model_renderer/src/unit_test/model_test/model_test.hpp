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

			ref<graphics::Model> loaded_model;

			engine::World world;
			engine::ResourceManager resource_manager;

			engine::Entity& camera = world.cameras[0];
			engine::Entity model_entity;

			ModelTest(bool auto_execute=true);

			void update() override;
			void render() override;

			void on_keyup(const keyboard_event_t& event) override;
			void on_keydown(const keyboard_event_t& event) override;
	};
}