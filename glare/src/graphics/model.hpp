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

namespace Assimp
{
	class Importer;
}

namespace std
{
	namespace filesystem
	{
		class path;
	}
}

namespace filesystem = std::filesystem;

// Assimp:
struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;

//class aiMatrix4x4;

using _aiMatrix4x4 = void*;

// Bullet:
class btTriangleIndexVertexArray;

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

			struct MeshDescriptor
			{
				Material material;
				std::vector<Mesh> meshes;

				MeshDescriptor(Material&& material, std::vector<Mesh>&& meshes) noexcept
					: material(std::move(material)), meshes(std::move(meshes)) {}

				MeshDescriptor(Material&& material) noexcept
					: material(std::move(material)) {}

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
		public:
			static std::tuple<Model, std::optional<CollisionGeometry>> Load(pass_ref<Context> context, const std::string& path, pass_ref<Shader> default_shader, bool load_collision=false);

			friend void swap(Model& x, Model& y);

			Model() = default;

			// TODO: Handle copies.
			Model(const Model&) = delete;

			Model(Meshes&& meshes, VertexWinding vertex_winding=VertexWinding::Clockwise) noexcept;
			Model(Model&& model) noexcept;

			// TODO: Verify non-const access.
			inline Meshes& get_meshes() { return meshes; };
			inline bool has_meshes() const { return !meshes.empty(); }
			inline explicit operator bool() const { return has_meshes(); }

			inline Model& operator=(Model model)
			{
				swap(*this, model);

				return *this;
			}

			inline VertexWinding get_winding_order() const { return vertex_winding; }
		protected:
			ref<Texture> process_texture(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const filesystem::path& texture_path);
			Material process_material(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiMaterial* native_material, pass_ref<Shader> default_shader, bool load_textures=true, bool load_values=true);
			void process_node(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node, pass_ref<Shader> default_shader, VertexWinding vert_direction, CollisionGeometry::Container* opt_collision_out=nullptr, const _aiMatrix4x4* _scene_orientation=nullptr);
			MeshData<VertexType> process_mesh(Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node, const aiMesh* mesh, VertexWinding vert_direction, const _aiMatrix4x4* _scene_orientation=nullptr);
	};
}