#include "src/core.hpp"
//#include "src/graphics/graphics.hpp"

#include <iostream>
#include <vector>
#include <string>

// Temporary:
#include <sdl2/SDL.h>

class Game : public app::Application
{
	public:
		struct Libraries
		{
			Libraries()
			{
				using namespace util;

				ASSERT(lib::init_sdl());
				ASSERT(lib::establish_gl());
			}

			~Libraries() {}
		};

		struct Graphics
		{
			Graphics(app::Window& window)
			{
				context = std::make_shared<graphics::Context>(window, graphics::backend::OpenGL);

				// Create the default canvas.
				canvas = std::make_shared<graphics::Canvas>(context);
			}

			ref<graphics::Context> context;
			ref<graphics::Canvas> canvas;
		};

		Libraries init_libraries;
		app::Window window;
		Graphics graphics;

		Game(const std::string& title="", bool auto_run=true, int width=1280, int height=720)
			: init_libraries(), window(width, height, title), graphics(window)
		{
			if (auto_run)
			{
				start();
				run();
			}
		}

		void update() override
		{
			/*
			//std::cout << "Hello world." << std::endl;

			SDL_Event event;

			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
				case SDL_KEYDOWN:
					break;
				case SDL_QUIT:
					stop();

					return;
				}
			}
			*/
		}

		void render() override
		{
			auto& gfx = *graphics.canvas;

			gfx.clear(1, 1, 0, 1);
			gfx.flip(window);
		}
};

int main(int argc, char** argv)
{
	std::cout << "Hello world\n";

	{
		Game game("Game Engine");
	}

	std::cin.get();

	return 0;
}