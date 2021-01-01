#pragma once

#include <vector>

#include <core.hpp>
#include <math/math.hpp>

#include <engine/engine.hpp>
#include <engine/world/debug/debug.hpp>

#include <graphics/mesh.hpp>

namespace glare::tests
{
	class ModelTest : public app::GraphicsApplication
	{
		public:
			enum class GBufferDisplayMode : int
			{
				None,
				Position,
				Normal,
				AlbedoSpecular,

				Modes,
			};

			struct
			{
				graphics::FrameBuffer framebuffer;

				graphics::Texture position;
				graphics::Texture normal;
				graphics::Texture albedo_specular;

				graphics::Mesh screen_quad;

				//GBufferDisplayMode display_mode = GBufferDisplayMode::AlbedoSpecular;
				GBufferDisplayMode display_mode = GBufferDisplayMode::None;

			} gBuffer;

			struct
			{
				ref<graphics::Shader> forward;
				ref<graphics::Shader> forward_test;
				ref<graphics::Shader> geometry;
				ref<graphics::Shader> lighting_pass;
				ref<graphics::Shader> framebuffer_dbg;
				ref<graphics::Shader> light_box;
			} shaders;

			graphics::Texture test_texture;

			static constexpr auto NR_LIGHTS = 32;

			//std::vector<ref<graphics::Shader>> shaders;

			engine::World world;
			engine::ResourceManager resource_manager;

			//engine::Entity model_entity;

			ModelTest(bool auto_execute=true);

			engine::Entity load_model(const std::string& path);
			engine::Transform model(const std::string& path);
			engine::Transform transform(engine::Entity entity);

			engine::Entity make_camera(engine::World& world);

			void make_lights(engine::World& world);
			void make_models(engine::World& world, engine::Entity camera);

			void setup_world(engine::World& world);

			engine::Transform get_named_transform(std::string_view name);

			void on_user_keyboard_input(const app::input::KeyboardState& keyboard);

			void update(const app::DeltaTime& delta_time) override;
			void render() override;

			void on_keyup(const keyboard_event_t& event) override;
			void on_keydown(const keyboard_event_t& event) override;

			graphics::PointRect update_viewport(engine::Entity camera);
	};
}