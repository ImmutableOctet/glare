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

#include <game/game.hpp>

namespace graphics
{
	struct WorldRenderState;
}

namespace glare
{
	using namespace game;

	class Glare : public Game
	{
		public:
			static constexpr auto NR_LIGHTS = 32; // 128; // 16;

			Glare();

			engine::Transform get_named_transform(std::string_view name);

			//void on_stage_loaded(const engine::OnStageLoaded& stage_info);
			void on_user_keyboard_input(const app::input::KeyboardState& keyboard);
			void on_user_mouse_input(const app::input::MouseState& mouse);

			void on_update(float delta) override;
			void on_render(RenderState& render_state) override;

			void render_debug_controls();
			void meta_controls();

			void on_keyup(const keyboard_event_t& event) override;
			void on_keydown(const keyboard_event_t& event) override;
	};
}