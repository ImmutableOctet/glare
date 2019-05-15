#include "src/core.hpp"
//#include "src/graphics/graphics.hpp"

#include <iostream>
#include <vector>
#include <string>

// Temporary:
#include <sdl2/SDL.h>
#include "src/graphics/native/opengl.hpp"

namespace graphics
{
	class mesh
	{

	};

	class material
	{

	};

	class model
	{
		private:
			std::vector<mesh> meshes;
			std::vector<material> materials;
	};
}

int main(int argc, char** argv)
{
	using namespace std;
	using namespace util;

	if (!lib::init_sdl())
		return 1;

	lib::establish_gl();

	auto wnd = graphics::window(1024, 768);
	auto ctx = std::make_shared<graphics::context>(wnd, graphics::backend::OpenGL);

	auto gfx = graphics::canvas(ctx);

	gfx.clear(1, 1, 0, 1);

	gfx.flip(wnd);

	cout << "Hello world\n";

	cin.get();

	return 0;
}