#include <core.hpp>
//#include "src/graphics/graphics.hpp"

#include <iostream>

// Unit Tests:
//#include "src/unit_test/shader_test/shader_test.hpp"
//#include "src/unit_test/deferred_test/deferred_test.hpp"

#include "src/unit_test/model_test/model_test.hpp"
//#include "src/unit_test/signal_test/signal_test.hpp"
#include "src/unit_test/timer_test/timer_test.hpp"

int main(int argc, char** argv)
{
	using namespace unit_test;

	//test_vertex_element_types();
	//ShaderTest();
	//DeferredTest();

	ModelTest();

	//TimerTest();

	//SignalTest();
	//DispatcherTest();

	return 0;
}