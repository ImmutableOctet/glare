#include <util/log.hpp>

//#include "src/graphics/graphics.hpp"

// Unit Tests:
//#include "src/tests/shader_test/shader_test.hpp"
//#include "src/tests/deferred_test/deferred_test.hpp"

//#include "src/tests/signal_test/signal_test.hpp"
//#include "src/tests/timer_test/timer_test.hpp"

#include "src/glare.hpp"

namespace game
{
	int test(int argc, char** argv)
	{
		auto application =
			
			//test_vertex_element_types();
			//ShaderTest();
			//DeferredTest();

			glare::Glare();

		//TimerTest();

		//SignalTest();
		//DispatcherTest();

		application.execute();

		return 0;
	}

	int exec(int argc, char** argv)
	{
		return test(argc, argv);
	}
}

int main(int argc, char** argv)
{
	util::log::init();

	return game::exec(argc, argv);
}