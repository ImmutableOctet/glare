#include <core.hpp>
#include <util/log.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

//#include "src/graphics/graphics.hpp"

#include <iostream>

// Unit Tests:
//#include "src/tests/shader_test/shader_test.hpp"
//#include "src/tests/deferred_test/deferred_test.hpp"

#include "src/tests/model_test/model_test.hpp"
//#include "src/tests/signal_test/signal_test.hpp"
#include "src/tests/timer_test/timer_test.hpp"

namespace glare
{
	int test(int argc, char** argv)
	{
		using namespace glare::tests;

		auto application =

			//test_vertex_element_types();
			//ShaderTest();
			//DeferredTest();

			ModelTest();

		//TimerTest();

		//SignalTest();
		//DispatcherTest();

		return 0;
	}

	int exec(int argc, char** argv)
	{
		auto console = spdlog::stdout_color_mt("console");
		auto err_logger = spdlog::stderr_color_mt("stderr");

		return test(argc, argv);
	}
}

int main(int argc, char** argv)
{
	return glare::exec(argc, argv);
}