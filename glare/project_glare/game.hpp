#pragma once

#include <vector>
#include <tuple>

#include <math/math.hpp>

#include <graphics/mesh.hpp>
#include <graphics/texture.hpp>
#include <graphics/framebuffer.hpp>
#include <graphics/gbuffer.hpp>

#include <engine/transform.hpp>

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
			Glare();

			engine::Transform get_named_transform(std::string_view name);

			void on_user_keyboard_input(const app::input::KeyboardState& keyboard);

			void on_update(float delta) override;
			void on_render(RenderState& render_state) override;

			void render_debug_controls();
			void meta_controls();

			void on_keyup(const keyboard_event_t& event) override;
	};
}