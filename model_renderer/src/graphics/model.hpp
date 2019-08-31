#pragma once

#include <string>
#include <vector>
#include <utility>

#include <util/memory.hpp>

#include "mesh.hpp"
#include "material.hpp"

// Assimp:
struct aiScene;
struct aiNode;
struct aiMesh;

namespace graphics
{
	class Context;
	class Canvas;
	//class Material;
	//class Mesh;

	class Model
	{
		public:
			using MeshDescriptor = std::pair<ref<Mesh>, ref<Material>>;
			using Meshes = std::vector<MeshDescriptor>;
		private:
			Meshes meshes;
		protected:
			Model(const Meshes& meshes);
		public:
			static Model Load(pass_ref<Context> context, const std::string& path);

			friend void swap(Model& x, Model& y);

			Model() = default;

			// TODO: Handle copies.
			Model(const Model&) = delete;

			Model(Model&& model);

			inline Model& operator=(Model model)
			{
				swap(*this, model);

				return *this;
			}

			void draw(Context& context); // virtual override

			// TODO: Verify non-const access.
			inline Meshes& get_meshes() { return meshes; };
		protected:
			void process_node(pass_ref<Context> context, const aiScene* scene, const aiNode* node);
			MeshDescriptor process_mesh(pass_ref<Context> context, const aiScene* scene, const aiMesh* mesh);
	};
}