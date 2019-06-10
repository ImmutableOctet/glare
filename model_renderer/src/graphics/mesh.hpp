#pragma once

#include <vector>
#include <cstdint>

#include "types.hpp"
#include "context.hpp"
#include "vertex.hpp"

namespace graphics
{
	template <typename VertexType>
	struct MeshData
	{
		using Index = MeshIndex;
		//using Vertex = VertexType;

		std::vector<VertexType> vertices;
		std::vector<Index> indices;

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
		protected:
			// Fields:
			MeshComposition composition;
			weak_ref<Context> context;
		public:
			inline ref<Context> get_context_ref() { return context.lock(); }
			inline Context& get_context() { return *get_context_ref(); }
			inline const MeshComposition& get_composition() const { return composition; }

			Mesh(pass_ref<Context> ctx, const Data& data)
				: context(ctx),
				composition
				(
					ctx->generate_mesh
					(
						memory::memory_view(data.vertices.data(), data.vertices.size()),
						sizeof(VertexType),
						VertexType::format()
					)
				)
			{
			}

			~Mesh()
			{
				get_context().release_mesh(std::move(composition));
			}
	};
}