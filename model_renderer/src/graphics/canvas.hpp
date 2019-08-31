#pragma once

/*
	TODO:
		* Add scissor functionality for rendering portions of the screen.
*/

#include <memory>
#include <unordered_map>
#include <vector>

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

			void draw();
		private:
			std::shared_ptr<Context> context;

			//std::unordered_map<const weak_ref<Material>, std::vector<const ref<Mesh>>> draw_operations;
	};
}