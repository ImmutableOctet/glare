#include "signal_test.hpp"

#include <entt/entt.hpp>
#include <iostream>
#include <string>

#include <app/input/input.hpp>

namespace glare::tests
{
	int f()
	{
		return 10;
	}

	int g()
	{
		return 20;
	}

	void SignalTest()
	{
		// Signals ('sigh' objects) store function pointers.
		entt::sigh<int()> signal;

		{
			// 'sink' objects register functions for use with 'signal'.
			entt::sink sink { signal };

			sink.connect<&f>();
		}

		{
			// 'sink' objects register functions for use with 'signal'.
			entt::sink sink{ signal };

			sink.connect<&g>();
		}

		signal.collect([](int value)
		{
			std::cout << value << '\n';
		});

		std::cin.get();
	}

	void DispatcherTest()
	{
		using MouseState = app::input::MouseState;

		struct listener_t
		{
			entt::dispatcher dispatcher;

			void poll()
			{
				dispatcher.enqueue<MouseState>({4, 1});
			}

			void update()
			{
				dispatcher.update();
			}

			void on_mouse_input(const MouseState& m)
			{
				std::cout << "Mouse: " << m.x << ", " << m.y << '\n';
			}

			void on_mouse_input2(const MouseState& m)
			{
				std::cout << "Mouse 2: " << m.x << ", " << m.y << '\n';
			}

			listener_t()
			{
				dispatcher.sink<MouseState>().connect<&listener_t::on_mouse_input>(*this);
				dispatcher.sink<MouseState>().connect<&listener_t::on_mouse_input2>(*this);
			}
		};

		listener_t listener;

		listener.poll();
		listener.poll();
		listener.poll();

		listener.update();

		std::cin.get();
	}
}