#pragma once

#include <vector>

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
			std::vector<ref<graphics::Shader>> shaders;

			ref<graphics::Model> loaded_model;

			engine::World world;
			engine::ResourceManager resource_manager;

			engine::Entity model_entity;

			ModelTest(bool auto_execute=true);

			void setup_world(engine::World& world);

			void update(const app::DeltaTime& delta_time) override;
			void render() override;

			void on_keyup(const keyboard_event_t& event) override;
			void on_keydown(const keyboard_event_t& event) override;
	};
}