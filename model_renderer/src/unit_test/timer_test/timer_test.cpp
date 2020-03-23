#include "timer_test.hpp"

#include <app/timer.hpp>
#include <iostream>

void TimerTest()
{
	using namespace app;

	auto timer = Timer(1000, 0, [](Timer&, Duration)
		{
			std::cout << "Hello world.\n";
		});

	timer.update(1000);

	std::cin.get();
}