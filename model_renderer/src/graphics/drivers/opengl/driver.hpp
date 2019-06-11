#pragma once

#include <graphics/drivers/driver.hpp>

namespace graphics
{
	// The layout of an OpenGL high-level mesh composition.
	struct GLComposition
	{
		// Vertex Array Object. (Mesh State)
		ContextHandle VAO;

		// Vertex Buffer Object. (Vertices)
		ContextHandle VBO;

		// Element Buffer Object. (Indices)
		ContextHandle EBO;
	};
}