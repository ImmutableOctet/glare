#pragma once

/*
	TODO:
		* Add scissor functionality for rendering portions of the screen.
*/

#include <memory>
#include <unordered_map>
#include <vector>

#include <util/memory.hpp>
#include <math/math.hpp>

namespace app
{
	class Window;
}

namespace graphics
{
	class Context;
	class Model;

	class Canvas
	{
		public:
			Canvas();
			Canvas(memory::pass_ref<Context> ctx);

			~Canvas();

			bool attach(memory::pass_ref<Context> ctx);
			void detach();

			void flip(app::Window& wnd);
			void clear(float red, float green, float blue, float alpha);

			void draw(Model& model, const math::Matrix& model_matrix);
		private:
			std::shared_ptr<Context> context;

			//std::unordered_map<const weak_ref<Material>, std::vector<const ref<Mesh>>> draw_operations;
	};
}