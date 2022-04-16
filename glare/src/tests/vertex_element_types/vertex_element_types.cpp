#pragma once

#include <graphics/vertex.hpp>
#include <debug.hpp>

namespace glare::tests
{
	void test_vertex_element_types()
	{
		using namespace graphics;

		assert(Vertex::GetElementType<int>() == ElementType::Int);
		assert(Vertex::GetElementType<float>() == ElementType::Float);
		assert(Vertex::GetElementType<double>() == ElementType::Double);
	}
}