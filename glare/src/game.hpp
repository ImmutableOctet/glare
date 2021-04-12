#pragma once

#include <vector>

#include <core.hpp>
#include <math/math.hpp>

#include <engine/engine.hpp>
#include <engine/world/debug/debug.hpp>

#include <graphics/mesh.hpp>

#include <util/log.hpp>

namespace glare
{
	class Glare : public app::GraphicsApplication
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

				GBufferDisplayMode display_mode = GBufferDisplayMode::AlbedoSpecular;
				//GBufferDisplayMode display_mode = GBufferDisplayMode::None;

			} gBuffer;

			struct init_shaders
			{
				init_shaders(Graphics& graphics);

				ref<graphics::Shader> forward;
				ref<graphics::Shader> forward_test;
				ref<graphics::Shader> geometry;
				ref<graphics::Shader> lighting_pass;
				ref<graphics::Shader> framebuffer_dbg;
				ref<graphics::Shader> light_box;
			} shaders;

			static constexpr auto NR_LIGHTS = 32;
			static constexpr Application::UpdateRate TARGET_UPDATE_RATE = 60;

			graphics::Texture test_texture;

			//std::vector<ref<graphics::Shader>> shaders;

			engine::ResourceManager resource_manager;
			engine::World world;

			//engine::Entity model_entity;

			util::Logger dbg = util::log::get_console();

			Glare(bool auto_execute=true);

			engine::Entity load_model(const std::string& path);

			engine::Transform model(const std::string& path);
			engine::Transform transform(engine::Entity entity);

			engine::Entity make_camera(engine::World& world);

			void make_lights(engine::World& world);
			void make_models(engine::World& world, engine::Entity player);

			void setup_world(engine::World& world);

			engine::Transform get_named_transform(std::string_view name);

			void on_user_keyboard_input(const app::input::KeyboardState& keyboard);

			void update(app::Milliseconds time) override;
			void render() override;

			void on_keyup(const keyboard_event_t& event) override;
			void on_keydown(const keyboard_event_t& event) override;

			graphics::Viewport update_viewport(engine::Entity camera);
	};
}