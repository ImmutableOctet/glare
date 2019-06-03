#pragma once

#include <vector>
#include <cstdint>

#include "vertex.hpp"

namespace graphics
{
	using MeshIndex = std::int32_t; // GLint;

	template <typename VertexType>
	struct MeshData
	{
		using Index = MeshIndex;
		//using Vertex = VertexType;

		std::vector<VertexType> vertices;
		std::vector<MeshIndex> indices;

		inline bool has_vertices() const { return (vertices.count() > 0); }
		inline bool has_indices() const { return (indices.count() > 0); }

		inline bool is_complete() const { return (has_vertices() && has_indices()); }
		inline bool is_usable() const { return has_vertices(); }

		inline operator bool() const { return is_usable(); }
	};

	template <typename VertexType>
	class Mesh
	{
		public:
			using Index = MeshIndex;
			//using Vertex = VertexType;

			using Data = MeshData<VertexType>;

			inline Mesh(const Data& data)
			{

			}
	};
}