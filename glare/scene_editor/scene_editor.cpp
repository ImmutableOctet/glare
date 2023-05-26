#include "scene_editor.hpp"

#include <app/events.hpp>
#include <app/input/keycodes.hpp>

#include <util/log.hpp>

#include <engine/debug/debug.hpp>

#include <engine/world/meta_controls/meta_controls.hpp>

#include <graphics/context.hpp>
#include <graphics/native/opengl.hpp>
#include <engine/world/render/world_render_state.hpp>
#include <engine/world/physics/collision_shape_description.hpp>

#include <engine/editor/editor.hpp>

#include <engine/events.hpp>

#include <engine/meta/serial.hpp>

#include <engine/input/input.hpp>
#include <engine/input/buttons.hpp>

#include <sdl2/SDL_video.h>
#include <sdl2/SDL_events.h>

#include <imgui.h>

#include <util/format.hpp>

// Debugging related:
#include <engine/world/physics/collision_group.hpp>
#include <util/magic_enum.hpp>

namespace glare
{
	SceneEditor::SceneEditor():
		Game("Glare Scene Editor", 1600, 900, 60, false, false) // true
	{
		using namespace graphics;

		auto& editor = system<engine::Editor>(world, cfg, resource_manager);

		engine::load(cfg, std::filesystem::path("config/config.json"), true);

		init_input_system
		(
			[](auto& input_system, auto& input_handler)
			{
				// TODO: Migrate these to the `glare` namespace:
				engine::generate_button_map(input_handler.get_buttons());
				engine::generate_analog_map(input_handler.get_analogs());
			}
		);
		
		world_system<engine::DebugListener>();

		//input.get_mouse().lock();
		input.set_lock_status(true);

		const auto path = std::filesystem::path { "assets/maps/test01" };

		world.load(path, "map.json", engine::null, &systems);
	}

	engine::Editor* SceneEditor::get_editor()
	{
		return systems.get_system<engine::Editor>();
	}

	void SceneEditor::on_update(float delta)
	{
		engine::position_in_titlebar(*this, world.get_camera(), util::format("FPS: {} | ", graphics.framerate));
	}

	void SceneEditor::on_render(RenderState& render_state)
	{
		render_debug_controls();
	}

	void SceneEditor::render_debug_controls()
	{
		if (!imgui_enabled())
			return;

		meta_controls();

		engine::hierarchy_control(world, world.get_root());
	}

	// Subroutine of `render_debug_controls`.
	void SceneEditor::meta_controls()
	{
		ImGui::Begin("Meta", nullptr, ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoTitleBar);

		if (ImGui::Button("Switch to Game"))
		{
			toggle_freelook();
		}

		auto& io = ImGui::GetIO();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::End();
	}

	void SceneEditor::on_keyup(const keyboard_event_t& event)
	{
		switch (event.keysym.sym)
		{
			case SDLK_ESCAPE:
				stop();

				break;
			case SDLK_TAB:
				toggle_freelook();

				break;
			
			case SDLK_F12:
				graphics.context->set_flags(graphics::ContextFlags::Wireframe, !graphics.context->get_flag(graphics::ContextFlags::Wireframe));

				break;
		}
	}

	void SceneEditor::toggle_freelook()
	{
		//toggle_input_lock();

		input.get_mouse().toggle_lock();
	}
}

int main(int argc, char** argv)
{
	util::log::init();

	auto application = glare::SceneEditor();
	application.execute();

	return 0;
}