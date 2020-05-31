#pragma once

#include <string>
#include <vector>
#include <utility>
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
			struct MeshDescriptor
			{
				Material material;
				std::vector<Mesh> meshes;

				MeshDescriptor(Material&& material, std::vector<Mesh>&& meshes)
					: material(std::move(material)), meshes(std::move(meshes)) {}

				MeshDescriptor() = default;

				MeshDescriptor(MeshDescriptor&&) = default;
				MeshDescriptor(const MeshDescriptor&) = delete;
			};

			using Meshes = std::vector<MeshDescriptor>;

			friend Canvas;
			friend Context;
		private:
			// Retrieves a string containing the name of 'type' to be used to link with shaders.
			std::string get_texture_type_variable(TextureType type);

			Meshes meshes;
		public:
			static Model Load(pass_ref<Context> context, ResourceManager& resource_manager, const std::string& path, pass_ref<Shader> default_shader);

			friend void swap(Model& x, Model& y);

			Model() = default;

			// TODO: Handle copies.
			Model(const Model&) = delete;

			Model(Meshes&& meshes);
			Model(Model&& model);

			inline Model& operator=(Model model)
			{
				swap(*this, model);

				return *this;
			}

			// TODO: Verify non-const access.
			inline Meshes& get_meshes() { return meshes; };
		protected:
			ref<Texture> process_texture(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const filesystem::path& texture_path);
			Material process_material(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiMaterial* native_material, pass_ref<Shader> default_shader, bool load_textures=true, bool load_values=true);
			void process_node(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node, pass_ref<Shader> default_shader);
			Mesh process_mesh(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiMesh* mesh);
	};
}