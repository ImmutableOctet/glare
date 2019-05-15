#pragma once

#include <memory>

namespace graphics
{
	class context;
	class window;

	class canvas
	{
		public:
			canvas();
			canvas(std::shared_ptr<context> ctx);

			~canvas();

			bool attach(std::shared_ptr<context> ctx);
			void detach();

			void flip(window& wnd);
			void clear(float red, float green, float blue, float alpha);
		private:
			std::shared_ptr<context> ctx; // Attached context.
	};
}