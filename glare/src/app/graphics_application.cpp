#include "graphics_application.hpp"

#include <imgui/imgui_impl_sdl.h>

namespace app
{
	void GraphicsApplication::begin_render()
	{
		auto& gfx = *graphics.canvas;
		//auto& wnd = *window;

		gfx.begin();
	}

	void GraphicsApplication::end_render()
	{
		auto& gfx = *graphics.canvas;
		auto& wnd = *window;

		gfx.end();

		gfx.flip(wnd);
	}

	bool GraphicsApplication::process_event(const SDL_Event& e)
	{
		if (imgui_enabled())
		{
			const auto& mouse = input.get_mouse();
			
			if (mouse.locked())
			{
				return false;
			}

			if (ImGui_ImplSDL2_ProcessEvent(&e))
				return true;
		}

		return false;
	}
}