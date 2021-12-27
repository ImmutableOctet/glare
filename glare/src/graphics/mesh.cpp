#include "mesh.hpp"
#include "context.hpp"

#include <math/math.hpp>

#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/vector3.h>
#include <assimp/mesh.h>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//#include <assimp/material.h>

namespace graphics
{
	Mesh Mesh::GenerateTexturedQuad(pass_ref<Context> ctx, VertexWinding winding)
	{
		constexpr auto primitive = Primitive::TriangleStrip;

		switch (winding)
		{
			case VertexWinding::Clockwise:
				return Generate<TexturedVertex>
				(
					ctx,

					{
						// Vertices (Position + UV)
						{
							// Positions:             // UV Coordinates:
							{{{-1.0f,  1.0f, 0.0f}},  {0.0f, 1.0f}}, // Top Left
							{{{ 1.0f,  1.0f, 0.0f}},  {1.0f, 1.0f}}, // Top Right
							{{{-1.0f, -1.0f, 0.0f}},  {0.0f, 0.0f}}, // Bottom Left
							{{{ 1.0f, -1.0f, 0.0f}},  {1.0f, 0.0f}}  // Bottom Right
						},
				
						// Raw mesh; no need for indices.
						{}
					}, primitive
				);
			default: // case VertexWinding::CounterClockwise:
				return Generate<TexturedVertex>
				(
					ctx,

					{
						// Vertices (Position + UV)
						{
							// Positions:             // UV Coordinates:
							{{{-1.0f,  1.0f, 0.0f}},  {0.0f, 1.0f}},
							{{{-1.0f, -1.0f, 0.0f}},  {0.0f, 0.0f}},
							{{{ 1.0f,  1.0f, 0.0f}},  {1.0f, 1.0f}},
							{{{ 1.0f, -1.0f, 0.0f}},  {1.0f, 0.0f}}
						},
				
						// Raw mesh; no need for indices.
						{}
					}, primitive
				);
		}
	}

	Mesh::~Mesh()
	{
		auto ctx = get_context();
		
		if (ctx)
		{
			ctx->release_mesh(std::move(composition));
		}
	}

	void swap(Mesh& x, Mesh& y) noexcept
	{
		using std::swap;

		swap(x.context, y.context);
		swap(x.composition, y.composition);
		swap(x.primitive_type, y.primitive_type);
		swap(x.vertex_count, y.vertex_count);
		swap(x.vertex_offset, y.vertex_offset);
		swap(x.index_count, y.index_count);
	}

	void Mesh::on_bind(Context& context)
	{
		// Nothing so far.
	}

	std::vector<SimpleVertex> copy_simple_vertices(const aiMesh& mesh)
	{
		const auto vertex_count = mesh.mNumVertices;

		std::vector<SimpleVertex> vertices_out;

		vertices_out.reserve(vertex_count);

		for (unsigned i = 0; i < vertex_count; i++)
		{
			SimpleVertex vertex;

			vertex.position = math::to_vector(mesh.mVertices[i]);

			vertices_out.push_back(vertex);
		}

		return vertices_out;
	}
}