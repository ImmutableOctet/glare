#include <core.hpp>
//#include "src/graphics/graphics.hpp"

#include <iostream>

// Unit Tests:
//#include "src/unit_test/shader_test.hpp"
#include "src/unit_test/model_test.hpp"

int main(int argc, char** argv)
{
	using namespace unit_test;

	//test_vertex_element_types();
	//ShaderTest();
	ModelTest();

	return 0;
}