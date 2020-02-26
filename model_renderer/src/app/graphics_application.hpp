#pragma once

#include <string>
#include <stdexcept>

#include <graphics/graphics.hpp>

#include "application.hpp"

namespace app
{
	class GraphicsApplication : public Application
	{
		public:
			struct Graphics
			{
				Graphics(app::Window& window, WindowFlags flags)
				{
					if (!(flags & WindowFlags::OpenGL))
					{
						throw std::runtime_error("OpenGL is currently the only supported graphics-backend.");
					}

					context = memory::allocate<graphics::Context>(window, graphics::Backend::OpenGL, graphics::ContextFlags::Default);

					// Create the default canvas.
					canvas = memory::allocate<graphics::Canvas>(context);
				}

				ref<graphics::Context> context;
				ref<graphics::Canvas> canvas;
			};

			Graphics graphics;

			GraphicsApplication(const std::string& title, int width, int height, WindowFlags flags=app::WindowFlags::OpenGL|app::WindowFlags::Resizable)
				: Application(), graphics(make_window(width, height, title, flags), flags) {}
	};
}