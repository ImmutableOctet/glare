#pragma once

#include <graphics/vertex.hpp>
#include <debug.hpp>

namespace unit_test
{
	void test_vertex_element_types()
	{
		using namespace graphics;

		ASSERT(Vertex::GetElementType<int>() == ElementType::Int);
		ASSERT(Vertex::GetElementType<float>() == ElementType::Float);
		ASSERT(Vertex::GetElementType<double>() == ElementType::Double);
	}
}