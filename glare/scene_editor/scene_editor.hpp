#pragma once

#include <game/game.hpp>

//#include <engine/editor/editor.hpp>

namespace engine
{
	class Editor;
}

namespace glare
{
	using namespace game;

	class SceneEditor : public Game
	{
		public:
			SceneEditor();

			void on_update(float delta) override;
			void on_render(RenderState& render_state) override;

			void render_debug_controls();
			void meta_controls();

			void toggle_freelook();

			void on_keyup(const keyboard_event_t& event) override;

		protected:
			engine::Editor* get_editor();
	};
}