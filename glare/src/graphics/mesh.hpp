#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include <type_traits>

#include "types.hpp"
#include "context.hpp"
#include "vertex.hpp"

// Assimp mesh object.
struct aiMesh;

namespace graphics
{
	template <typename VertexType>
	struct MeshData
	{
		using Index = MeshIndex;
		using Vertex = VertexType;

		//using SimpleVertex = graphics::SimpleVertex;

		static constexpr bool is_animated_vertex = std::is_same_v<StandardAnimationVertex, VertexType>;

		std::vector<VertexType> vertices; // std::shared_ptr<std::vector<VertexType>>
		std::shared_ptr<std::vector<Index>> indices;

		inline bool has_vertices() const { return (vertices.size() > 0); }
		inline bool has_indices() const
		{
			if (!indices)
			{
				return false;
			}

			return (indices->size() > 0);
		}

		inline bool is_complete() const { return (has_vertices() && has_indices()); }
		inline bool is_usable() const { return has_vertices(); }

		inline explicit operator bool() const { return is_usable(); }

		inline bool is_animated() const { return is_animated_vertex; }

		// Creates a copy of the 'vertices' vector, containing only simple vertex attributes. (e.g. position)
		inline std::vector<SimpleVertex> as_simple_vertices() const
		{
			std::vector<SimpleVertex> vertices_out;

			vertices_out.reserve(vertices.size());

			for (const auto& v : vertices)
			{
				vertices_out.push_back(v);
			}

			return vertices_out;
		}
	};

	using SimpleMeshData = MeshData<SimpleVertex>;

	// TODO: Review separation of 'MeshComposition' and 'Mesh'.
	// Context-controlled 'MeshComposition' objects would allow proper
	// re-use of buffer objects, and help unify the rendering model.
	class Mesh
	{
		public:
			template <typename resource_t, typename bind_fn>
			friend class BindOperation;

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
			std::size_t index_count = 0;

			bool animated = false;

			inline Mesh(weak_ref<Context> ctx, MeshComposition composition, Primitive primitive_type, std::size_t vertex_count, Index vertex_offset=0, std::size_t index_count=0, bool animated=false)
				: context(ctx), composition(composition), primitive_type(primitive_type), vertex_count(vertex_count), vertex_offset(vertex_offset), index_count(index_count), animated(animated) {}

			virtual void on_bind(Context& context);
		public:
			Mesh(const Mesh&) = delete;
			//Mesh& operator=(const Mesh&) = delete;

			inline ref<Context> get_context_ref() { return context.lock(); }
			
			inline Context* get_context()
			{
				return get_context_ref().get();
			}

			inline const MeshComposition& get_composition() const { return composition; }
			inline Primitive get_primitive() const { return primitive_type; }

			inline std::size_t size() const { return vertex_count; }
			inline std::size_t indices() const { return index_count; }
			inline Index offset() const { return vertex_offset; }

			inline bool is_animated() const { return animated; }

			template <typename VertexType>
			inline static Mesh Generate(pass_ref<Context> ctx, const Data<VertexType>& data, Primitive primitive_type=Primitive::Triangle, std::optional<bool> animated={})
			{
				return Mesh
				(
					ctx,

					ctx->generate_mesh
					(
						memory::memory_view(data.vertices.data(), data.vertices.size()),
						sizeof(VertexType),
						VertexType::format(),
						((data.indices) ? memory::array_view<MeshIndex>(data.indices->data(), data.indices->size()) : nullptr)
					),
					
					primitive_type,
					data.vertices.size(), 0,
					((data.indices) ? data.indices->size() : 0),
					animated.value_or(std::is_same_v<StandardAnimationVertex, VertexType>)
				);
			}

			static Mesh GenerateTexturedQuad(pass_ref<Context> ctx, VertexWinding winding=VertexWinding::CounterClockwise); // Clockwise

			friend void swap(Mesh& x, Mesh& y) noexcept;

			inline Mesh& operator=(Mesh mesh) noexcept
			{
				swap(*this, mesh);

				return *this;
			}

			Mesh() = default;

			inline Mesh(Mesh&& mesh) noexcept : Mesh() { swap(*this, mesh); }
			
			~Mesh();
	};

	std::vector<SimpleVertex> copy_simple_vertices(const aiMesh& mesh);
}