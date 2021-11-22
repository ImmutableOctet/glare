#pragma once

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <tuple>
#include <unordered_map>

#include <util/memory.hpp>

#include "types.hpp"
#include "mesh.hpp"
#include "material.hpp"

// Bullet:
class btTriangleIndexVertexArray;

namespace engine
{
	class ModelLoader;
}

namespace graphics
{
	class Context;
	class Canvas;
	class Texture;
	//class Material;
	//class Mesh;

	class Model
	{
		public:
			using VertexType = StandardVertex;
			using AVertexType = StandardAnimationVertex;

			struct MeshDescriptor
			{
				//Material material;
				ref<Material> material;
				std::vector<Mesh> meshes;

				/*
				MeshDescriptor(Material&& material, std::vector<Mesh>&& meshes) noexcept
					: material(std::move(material)), meshes(std::move(meshes)) {}

				MeshDescriptor(Material&& material) noexcept
					: material(std::move(material)) {}
				*/

				MeshDescriptor(pass_ref<Material> material, std::vector<Mesh>&& meshes) noexcept
					: material(material), meshes(std::move(meshes)) {}

				MeshDescriptor(pass_ref<Material> material) noexcept
					: material(material) {}

				MeshDescriptor() noexcept = default;

				MeshDescriptor(const MeshDescriptor&) noexcept = delete;
				MeshDescriptor(MeshDescriptor&&) = default;

				inline bool has_meshes() const { return (meshes.size() > 0); }
				inline explicit operator bool() const { return has_meshes(); }

				MeshDescriptor& operator=(MeshDescriptor&&) noexcept(false) = default;
				//MeshDescriptor& operator=(const MeshDescriptor&) = delete;
			};

			using Meshes = std::vector<MeshDescriptor>;

			struct CollisionGeometry // CollisionData
			{
				using Descriptor = btTriangleIndexVertexArray;
				using Container = std::vector<SimpleMeshData>;

				CollisionGeometry(Container&& mesh_data);

				std::unique_ptr<Descriptor> mesh_interface;
				Container mesh_data;
			};

			friend Canvas;
			friend Context;

			friend engine::ModelLoader;

			// Retrieves a string containing the name of 'class' to be used to link with shaders.
			static const char* get_texture_class_variable_raw(TextureClass type);

			inline static std::string get_texture_class_variable(TextureClass type)
			{
				const auto* raw = get_texture_class_variable_raw(type);

				if (!raw)
				{
					return {};
				}

				return raw;
			}

		private:
			Meshes meshes;
			VertexWinding vertex_winding = VertexWinding::Clockwise;

			//bool animated = false;
		public:
			friend void swap(Model& x, Model& y);

			Model() = default;

			// TODO: Handle copies.
			Model(const Model&) = delete;

			Model(Meshes&& meshes, VertexWinding vertex_winding=VertexWinding::Clockwise) noexcept;
			Model(Model&& model) noexcept;

			// TODO: Verify non-const access.
			inline Meshes& get_meshes() { return meshes; };
			inline bool has_meshes() const { return !meshes.empty(); }

			inline bool is_animated() const { return false; }

			inline explicit operator bool() const { return has_meshes(); }

			inline Model& operator=(Model model)
			{
				swap(*this, model);

				return *this;
			}

			inline VertexWinding get_winding_order() const { return vertex_winding; }
	};
}