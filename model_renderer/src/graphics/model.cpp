#include "model.hpp"
#include "mesh.hpp"
#include "material.hpp"

#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/vector3.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static math::Vector3D to_vector(const aiVector3D& v)
{
	return { v.x, v.y, v.z };
}

namespace graphics
{
	Model::Model(const Meshes& meshes)
		: meshes(meshes) {}

	Model::Model(Model&& model)
		: Model() { swap(*this, model); }

	void swap(Model& x, Model& y)
	{
		using std::swap;

		swap(x.meshes, y.meshes);
	}

	Model Model::Load(pass_ref<Context> context, const std::string& path)
	{
		Model model;

		Assimp::Importer importer;

		// TODO: Implement 'flags' parameter.
		unsigned int flags = (aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		// Load a scene from the path specified.
		const aiScene* scene = importer.ReadFile(path, flags);

		// Ensure the scene was loaded:
		if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			//cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;

			return {};
		}

		// The root directory of the scene; used to load resources. (Textures, etc)
		auto directory = path.substr(0, path.find_last_of('/'));

		model.process_node(context, scene, scene->mRootNode);

		return model;
	}

	// Assimp:
	void Model::process_node(pass_ref<Context> context, const aiScene* scene, const aiNode* node)
	{
		// TODO: Review use of unsigned integer for indexing.

		// Process each mesh tied to this node:
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			auto mesh_index = node->mMeshes[i];
			aiMesh* mesh = scene->mMeshes[mesh_index];

			meshes.push_back(process_mesh(context, scene, mesh));
		}

		// Recursively process each child node:
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			auto* child = node->mChildren[i];
			process_node(context, scene, child);
		}
	}

	Model::MeshDescriptor Model::process_mesh(pass_ref<Context> context, const aiScene* scene, const aiMesh* mesh)
	{
		using VertexType = StandardVertex;

		// TODO: Review use of unsigned integer for indexing.

		// TODO: Materials.
		//mesh->mMaterialIndex;

		const auto vertex_count = (mesh->mNumVertices);

		MeshData<VertexType> data = {};

		data.vertices.reserve(vertex_count);

		// Retrieve vertex data:

		// TODO: Look into optimizing for decoupled attributes, if needed.
		// TODO: Review use of unsigned integer for indexing.
		for (unsigned i = 0; i < vertex_count; i++)
		{
			VertexType vertex;

			vertex.position = to_vector(mesh->mVertices[i]);
			vertex.normal   = to_vector(mesh->mNormals[i]);

			auto uv_channels = mesh->mTextureCoords[0];

			if (uv_channels)
			{
				vertex.uv = to_vector(uv_channels[i]);
			}
			else
			{
				vertex.uv = {};
			}

			vertex.tangent = to_vector(mesh->mTangents[i]);
			vertex.bitangent = to_vector(mesh->mBitangents[i]);

			data.vertices.push_back(vertex);
		}

		// Retrieve indices per-face:
		for (unsigned i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				data.indices.push_back(face.mIndices[j]);
			}
		}

		auto mesh_instance = memory::allocate<Mesh>();
		*mesh_instance = Mesh::Generate(context, data);

		return { mesh_instance, nullptr };
	}
}