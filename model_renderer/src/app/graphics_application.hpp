#pragma once

#include <string>
#include <graphics/graphics.hpp>

#include "application.hpp"

namespace app
{
	class GraphicsApplication : public Application
	{
		public:
			struct Graphics
			{
				Graphics(app::Window& window)
				{
					context = memory::allocate<graphics::Context>(window, graphics::Backend::OpenGL);

					// Create the default canvas.
					canvas = memory::allocate<graphics::Canvas>(context);
				}

				ref<graphics::Context> context;
				ref<graphics::Canvas> canvas;
			};

			Graphics graphics;

			GraphicsApplication(const std::string& title, int width, int height)
				: Application(), graphics(make_window(width, height, title)) {}
	};
}