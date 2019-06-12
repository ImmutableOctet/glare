#include <algorithm>

#include "mesh.hpp"
#include "context.hpp"

namespace graphics
{
	Mesh::~Mesh()
	{
		get_context().release_mesh(std::move(composition));
	}

	void swap(Mesh& x, Mesh& y)
	{
		using std::swap;

		swap(x.context, y.context);
		swap(x.composition, y.composition);
		swap(x.primitive_type, y.primitive_type);
		swap(x.vertex_count, y.vertex_count);
		swap(x.vertex_offset, y.vertex_offset);
	}
}