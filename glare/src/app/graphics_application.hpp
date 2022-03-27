#pragma once

#include <string_view>
#include <stdexcept>

// Debugger related.
#include <iostream>

#include <graphics/graphics.hpp>

#include "application.hpp"

namespace app
{
	class GraphicsApplication : public Application
	{
		protected:
			FrameCounter _prev_render_counter = 0;

			void begin_render() override;
			void end_render() override;

			bool process_event(const SDL_Event& e) override;

		private:
			bool _imgui_enabled = true;

		public:
			static constexpr UpdateRate DEFAULT_FRAMERATE = 60;

			struct Graphics
			{
				public:
					Graphics(app::Window& window, WindowFlags flags, bool vsync=true, bool extensions=true)
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

					ref<graphics::Context> context;
					ref<graphics::Canvas> canvas;

					FrameCounter framerate = 0;

					bool extensions = false;
			};

			Graphics graphics;

			GraphicsApplication(std::string_view title, int width, int height, WindowFlags flags=app::WindowFlags::OpenGL|app::WindowFlags::Resizable, UpdateRate update_rate=DEFAULT_FRAMERATE, bool vsync=true, bool imgui_enabled=true)
				: Application(update_rate), graphics(make_window(width, height, title, flags), flags, vsync, imgui_enabled), _imgui_enabled(imgui_enabled)
			{
				auto time = milliseconds();

				*this << Timer::make_continuous(Timer::DURATION_SECOND, time, [this](Timer& timer, Duration time_elapsed) mutable
				{
					auto framerate = (render_counter - _prev_render_counter);

					_prev_render_counter = render_counter;

					graphics.framerate = framerate;

					//std::cout << "Framerate: " << framerate << '\n';

					return Timer::DURATION_SECOND;
				});
			}

			inline bool imgui_enabled() const { return _imgui_enabled; }
	};
}