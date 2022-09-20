#pragma once

#include <string>
//#include <string_view>
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
class btTriangleMesh;

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

	struct CollisionGeometry;

	class Model
	{
		public:
			using VertexType = StandardVertex;
			using AVertexType = StandardAnimationVertex;

			/*
				TODO: Look into a copy-on-write interface for `Materials` tied to `MeshDescriptors`.
				
				e.g. You have a material (that could be shared) tied to part of your model,
				and the material is updated by some external system for an animated texture, time-value, etc.

				If you were to modify the material, it'd affect all models/meshes using it, but if you access with copy-on-write,
				you could modify a newly generated copy that's specific to your mesh/model.
			*/
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
			VertexWinding vertex_winding = VertexWinding::CounterClockwise;
		protected:
			bool animated = false;

			std::string name;
		public:
			friend void swap(Model& x, Model& y);

			Model() = default;

			// TODO: Handle copies.
			Model(const Model&) = delete;

			Model(Meshes&& meshes, VertexWinding vertex_winding=VertexWinding::CounterClockwise, bool animated=false) noexcept;
			Model(Model&& model) noexcept;

			// TODO: Verify non-const access.
			inline Meshes& get_meshes() { return meshes; };
			inline const std::string& get_name() const { return name; }

			inline bool has_meshes() const { return !meshes.empty(); }
			inline bool has_name() const { return !name.empty(); }

			inline bool is_animated() const { return animated; }

			inline explicit operator bool() const { return has_meshes(); }

			inline Model& operator=(Model model)
			{
				swap(*this, model);

				return *this;
			}

			inline VertexWinding get_winding_order() const { return vertex_winding; }
	};
}