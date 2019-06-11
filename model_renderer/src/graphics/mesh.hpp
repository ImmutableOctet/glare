#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

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

	class Mesh
	{
		public:
			using Index = MeshIndex;
			//using Vertex = VertexType;

			template <typename VertexType>
			using Data = MeshData<VertexType>;
		protected:
			// Fields:
			weak_ref<Context> context;

			MeshComposition composition = {};
			Primitive primitive_type = Primitive::Unknown;

			std::size_t vertex_count = 0;
			Index vertex_offset = 0;

			inline Mesh(weak_ref<Context> ctx, MeshComposition composition, Primitive primitive_type, std::size_t vertex_count, Index vertex_offset=0)
				: context(ctx), composition(composition), primitive_type(primitive_type), vertex_count(vertex_count), vertex_offset(vertex_offset) {}

			Mesh(const Mesh&) = delete;
		public:
			inline ref<Context> get_context_ref() { return context.lock(); }
			inline Context& get_context() { return *get_context_ref(); }
			inline const MeshComposition& get_composition() const { return composition; }
			inline Primitive get_primitive() const { return primitive_type; }

			template <typename VertexType>
			static Mesh Generate(pass_ref<Context> ctx, const Data<VertexType>& data, Primitive primitive_type=Primitive::Triangle)
			{
				return Mesh
				(
					ctx,

					ctx->generate_mesh
					(
						memory::memory_view(data.vertices.data(), data.vertices.size()),
						sizeof(VertexType),
						VertexType::format()
					),
					
					primitive_type,
					data.vertices.size(), 0
				);
			}

			// TODO: Move this to a separate source file.
			inline friend void swap(Mesh& x, Mesh& y)
			{
				using std::swap;

				swap(x.context, y.context);
				swap(x.composition, y.composition);
				swap(x.primitive_type, y.primitive_type);
				swap(x.vertex_count, y.vertex_count);
				swap(x.vertex_offset, y.vertex_offset);
			}

			inline Mesh& operator=(Mesh mesh)
			{
				swap(*this, mesh);

				return *this;
			}

			Mesh() = default;
			inline Mesh(Mesh&& mesh) : Mesh() { swap(*this, mesh); }
			
			~Mesh()
			{
				get_context().release_mesh(std::move(composition));
			}

			inline std::size_t size() const { return vertex_count; }
			inline Index offset() const { return vertex_offset; }
	};
}