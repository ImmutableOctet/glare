#pragma once

#include "types.hpp"
#include "application.hpp"

#include <graphics/graphics.hpp>

#include <string_view>
#include <stdexcept>

namespace app
{
	struct Graphics
	{
		public:
			Graphics(app::Window& window, WindowFlags flags, bool vsync=true, bool extensions=true);

			std::shared_ptr<::graphics::Context> context;
			std::shared_ptr<::graphics::Canvas> canvas;

			FrameCounter framerate = 0;

			bool extensions = false;
	};

	class GraphicsApplication : public Application
	{
		protected:
			void begin_render(Milliseconds time) override;
			void end_render(Milliseconds time) override;

			bool process_event(const SDL_Event& e) override;

		private:
			bool _imgui_enabled : 1 = true;

			FrameCounter _prev_render_counter = {};
			app::Milliseconds framerate_timer = {};

			FrameCounter update_framerate(Milliseconds time);

		public:
			using Graphics = app::Graphics;

			static constexpr UpdateRate DEFAULT_FRAMERATE = 60;

			Graphics graphics;

			GraphicsApplication
			(
				std::string_view title, int width, int height,
				WindowFlags flags=app::WindowFlags::OpenGL|app::WindowFlags::Resizable,
				UpdateRate update_rate=DEFAULT_FRAMERATE,
				bool vsync=true, bool imgui_enabled=true
			)
				: Application(update_rate), graphics(make_window(width, height, title, flags), flags, vsync, imgui_enabled), _imgui_enabled(imgui_enabled)
			{}

			inline bool imgui_enabled() const { return _imgui_enabled; }
	};
}