#include "graphics_application.hpp"

#include <imgui_impl_sdl.h>

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

		context = std::make_unique<graphics::Context>(window, graphics::Backend::OpenGL, graphics_flags, extensions);

		// Create the default canvas.
		canvas = std::make_unique<graphics::Canvas>(context);
	}

	void GraphicsApplication::begin_render(Milliseconds time)
	{
		graphics.canvas->begin();
	}

	void GraphicsApplication::end_render(Milliseconds time)
	{
		graphics.canvas->end();

		graphics.canvas->flip(*window);

		update_framerate(time);
	}

	FrameCounter GraphicsApplication::update_framerate(Milliseconds time)
	{
		if (const auto framerate_time_elapsed = (time - framerate_timer); framerate_time_elapsed >= 1000) // ms
		{
			const auto framerate = (get_render_counter() - _prev_render_counter);

			_prev_render_counter = get_render_counter();

			graphics.framerate = framerate;

			//framerate_timer += 1000;
			framerate_timer = time;
		}

		return graphics.framerate;
	}

	bool GraphicsApplication::process_event(const SDL_Event& e)
	{
		if (imgui_enabled())
		{
			const auto& mouse = input.get_mouse();
			
			if (!mouse.locked())
			{
				if (ImGui_ImplSDL2_ProcessEvent(&e))
				{
					if (e.type != SDL_WINDOWEVENT)
					{
						return true;
					}
				}
			}
		}

		return Application::process_event(e);
	}
}