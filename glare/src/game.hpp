#pragma once

#include <vector>
#include <tuple>

#include <core.hpp>
#include <math/math.hpp>

#include <engine/engine.hpp>
#include <engine/world/debug/debug.hpp>

#include <graphics/mesh.hpp>
#include <graphics/texture.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics/gbuffer.hpp>

#include <util/log.hpp>

namespace graphics
{
	struct WorldRenderState;
}

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
				ShadowMap,
				AlbedoSpecular,
				Depth,

				Modes,
			};

			GBufferDisplayMode gbuffer_display_mode
				//= GBufferDisplayMode::AlbedoSpecular;
				= GBufferDisplayMode::None;
				//= GBufferDisplayMode::ShadowMap;

			graphics::GBuffer gbuffer;

			struct init_shaders
			{
				init_shaders(Graphics& graphics);

				ref<graphics::Shader> forward;
				ref<graphics::Shader> forward_test;
				ref<graphics::Shader> geometry_pass;
				ref<graphics::Shader> lighting_pass;
				ref<graphics::Shader> framebuffer_dbg;
				ref<graphics::Shader> light_debug;
				ref<graphics::Shader> point_shadow_depth;
				ref<graphics::Shader> directional_shadow_depth;
				//ref<graphics::Shader> shadow_test;

				ref<graphics::Shader> default_shader;
			} shaders;

			static constexpr auto NR_LIGHTS = 32; // 128; // 16;
			static constexpr Application::UpdateRate TARGET_UPDATE_RATE = 60;

			graphics::Texture test_texture;

			graphics::NamedTextureArrayRaw dynamic_texture_maps;

			struct
			{
				graphics::VectorArray positions; // std::vector<math::Vector>
				graphics::FloatArray  far_planes; // std::vector<float>
			} point_light_shadows;

			struct
			{
				graphics::VectorArray positions; // std::vector<math::Vector>
				graphics::MatrixArray matrices;
			} directional_light_shadows;

			//std::vector<ref<graphics::Shader>> shaders;

			engine::ResourceManager resource_manager;
			engine::World world;

			//engine::Entity model_entity;

			util::Logger dbg = util::log::get_console();

			//ref<engine::Config> cfg;
			engine::Config cfg;

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

			std::tuple<bool, bool> render_shadows(bool point_lights=true, bool directional_lights=true);
			graphics::GBuffer& render_geometry(engine::World& world, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, graphics::WorldRenderState& render_state);
			graphics::GBuffer& render_lighting(engine::World& world, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, const graphics::WorldRenderState& render_state);
			graphics::GBuffer& render_screen(const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, GBufferDisplayMode display_mode);
			graphics::GBuffer& render_debug(const graphics::Viewport& viewport, graphics::GBuffer& gbuffer);

			void on_keyup(const keyboard_event_t& event) override;
			void on_keydown(const keyboard_event_t& event) override;
			void on_window_resize(app::Window& window, int width, int height) override;

			std::tuple<graphics::Viewport, math::vec2i> update_viewport(); // graphics::Viewport
			std::tuple<graphics::Viewport, math::vec2i> update_viewport(engine::Entity camera); // graphics::Viewport
	};
}