#include "graphics_application.hpp"

#include <imgui/imgui_impl_sdl.h>

namespace app
{
	Graphics::Graphics(app::Window& window, WindowFlags flags, bool vsync, bool extensions)
		: extensions(extensions)
	{
		if (!(flags & WindowFlags::OpenGL))
		{
			throw std::runtime_error("OpenGL is currently the only supported graphics-backend.");
		}

		auto graphics_flags = graphics::ContextFlags::Default;

		if (vsync)
		{
			graphics_flags |= graphics::ContextFlags::VSync;
		}
		else
		{
			graphics_flags &= ~graphics::ContextFlags::VSync;
		}

		context = memory::allocate<graphics::Context>(window, graphics::Backend::OpenGL, graphics_flags, extensions);

		// Create the default canvas.
		canvas = memory::allocate<graphics::Canvas>(context);
	}

	void GraphicsApplication::begin_render()
	{
		graphics.canvas->begin();
	}

	void GraphicsApplication::end_render()
	{
		graphics.canvas->end();

		graphics.canvas->flip(*window);
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
			{
				return true;
			}
		}

		return false;
	}
}