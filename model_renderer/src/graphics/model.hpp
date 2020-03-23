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

namespace boost
{
	namespace filesystem
	{
		class path;
	}
}

namespace filesystem = boost::filesystem;

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
			using MeshDescriptor = std::pair<ref<Mesh>, ref<Material>>;
			using Meshes = std::vector<MeshDescriptor>;
			using Materials = std::vector<ref<Material>>;
			using Textures = std::unordered_map<TextureType, TextureArray>;

			friend Canvas;
			friend Context;
		private:
			// Retrieves a string containing the name of 'type' to be used to link with shaders.
			std::string get_texture_type_variable(TextureType type);

			Meshes meshes;
			Materials materials;
			Textures textures;
		public:
			static Model Load(pass_ref<Context> context, ResourceManager& resource_manager, const std::string& path);

			friend void swap(Model& x, Model& y);

			Model() = default;

			// TODO: Handle copies.
			Model(const Model&) = delete;

			Model(const Meshes& meshes);
			Model(Model&& model);

			inline Model& operator=(Model model)
			{
				swap(*this, model);

				return *this;
			}

			// TODO: Verify non-const access.
			inline Meshes& get_meshes() { return meshes; };
		protected:
			ref<Material> process_material(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiMaterial* native_material, bool load_textures=true, bool load_values=true);
			void process_node(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node);
			MeshDescriptor process_mesh(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiMesh* mesh);
	};
}