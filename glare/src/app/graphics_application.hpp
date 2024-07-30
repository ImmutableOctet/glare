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
			static constexpr UpdateRate MAX_FRAMERATE_UNCAPPED = static_cast<UpdateRate>(-1ull);

			Graphics(app::Window& window, WindowFlags flags, bool vsync=true, bool extensions=true);

			std::shared_ptr<::graphics::Context> context;
			std::shared_ptr<::graphics::Canvas> canvas;

			UpdateRate framerate     = {};
			UpdateRate max_framerate = MAX_FRAMERATE_UNCAPPED;

			bool extensions : 1 = false;
	};

	class GraphicsApplication : public Application
	{
		protected:
			void begin_render(TimePoint time) override;
			void end_render(TimePoint time) override;

			bool process_event(const SDL_Event& e) override;

			bool can_update(TimePoint time) const override;
			bool can_render(TimePoint time) const override;

			Duration wait(TimePoint step_begin_time, bool exhaust_time_remainder=false) override;

			void on_step_counter_snapshot(TimeStepCounts counts_per_snapshot) override;

		private:
			bool _imgui_enabled : 1 = true;

		protected:
			Graphics graphics;

		public:
			using Graphics = app::Graphics;

			static constexpr UpdateRate DEFAULT_FRAMERATE = 60;

			GraphicsApplication
			(
				std::string_view title,
				int width, int height,
				WindowFlags flags=(WindowFlags::OpenGL|WindowFlags::Resizable),
				UpdateRate update_rate=DEFAULT_FRAMERATE,
				bool vsync=true, bool imgui_enabled=true
			) :
				Application(update_rate),
				
				graphics
				(
					make_window(width, height, title, flags),
					flags, vsync, imgui_enabled
				),

				_imgui_enabled(imgui_enabled)
			{}

			inline bool imgui_enabled() const { return _imgui_enabled; }

			bool vsync_enabled() const;
	};
}