#pragma once

#include <memory>

namespace app
{
	class Window;
}

namespace graphics
{
	class Context;

	class Canvas
	{
		public:
			Canvas();
			Canvas(std::shared_ptr<Context> ctx);

			~Canvas();

			bool attach(std::shared_ptr<Context> ctx);
			void detach();

			void flip(app::Window& wnd);
			void clear(float red, float green, float blue, float alpha);
		private:
			std::shared_ptr<Context> ctx; // Attached Context.
	};
}