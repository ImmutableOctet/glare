#pragma once

#include <graphics/context.hpp>
#include <graphics/drivers/driver.hpp>

namespace graphics
{
	// The layout of an OpenGL high-level mesh composition.
	struct GLMeshComposition
	{
		// Vertex Array Object. (Mesh State)
		Context::Handle VAO;

		// Vertex Buffer Object. (Vertices)
		Context::Handle VBO;

		// Element Buffer Object. (Indices)
		Context::Handle EBO;
	};
}