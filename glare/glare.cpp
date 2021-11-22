#include <core.hpp>
#include <util/log.hpp>

#include <iostream>
//#include <regex>

//#include "src/graphics/graphics.hpp"

// Unit Tests:
//#include "src/tests/shader_test/shader_test.hpp"
//#include "src/tests/deferred_test/deferred_test.hpp"

#include "src/game.hpp"
//#include "src/tests/signal_test/signal_test.hpp"
//#include "src/tests/timer_test/timer_test.hpp"

namespace glare
{
	int test(int argc, char** argv)
	{
		auto application =
			
			//test_vertex_element_types();
			//ShaderTest();
			//DeferredTest();

			Glare();

		//TimerTest();

		//SignalTest();
		//DispatcherTest();

		return 0;
	}

	int exec(int argc, char** argv)
	{
		util::log::init();

		return test(argc, argv);
	}
}

int main(int argc, char** argv)
{
	return glare::exec(argc, argv);
}