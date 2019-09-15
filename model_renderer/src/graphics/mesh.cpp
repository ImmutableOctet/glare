#include <algorithm>

#include "mesh.hpp"
#include "context.hpp"

namespace graphics
{
	Mesh Mesh::GenerateTexturedQuad(pass_ref<Context> ctx)
	{
		return Generate<TexturedVertex>
		(
			ctx,

			{
				// Vertices (Position + UV)
				{
					  // Positions:           // UV Coordinates:
					{{{-1.0f,  1.0f, 0.0f}},  {0.0f, 1.0f}},
					{{{-1.0f, -1.0f, 0.0f}},  {0.0f, 0.0f}},
					{{{ 1.0f,  1.0f, 0.0f}},  {1.0f, 1.0f}},
					{{{ 1.0f, -1.0f, 0.0f}},  {1.0f, 0.0f}}
				},
				
				// Raw mesh; no need for indices.
				{}
			}, Primitive::TriangleStrip
		);
	}

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

	void Mesh::on_bind(Context& context)
	{
		// Nothing so far.
	}
}