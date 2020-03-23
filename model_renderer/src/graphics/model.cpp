#include "model.hpp"
#include "mesh.hpp"
#include "material.hpp"

#include <engine/resource_manager.hpp>

#include <algorithm>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/vector3.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <boost/filesystem.hpp>

static math::Vector3D to_vector(const aiVector3D& v)
{
	return { v.x, v.y, v.z };
}

namespace graphics
{
	// Internal TextureTypes are currently consistent with Assimp.
	static aiTextureType to_aiTextureType(TextureType type)
	{
		return static_cast<aiTextureType>(type);
	}

	Model::Model(const Meshes& meshes)
		: meshes(meshes) {}

	Model::Model(Model&& model)
		: Model() { swap(*this, model); }

	void swap(Model& x, Model& y)
	{
		using std::swap;

		swap(x.meshes, y.meshes);
	}

	std::string Model::get_texture_type_variable(TextureType type)
	{
		switch (type)
		{
			case TextureType::Diffuse:
				return "diffuse";
			case TextureType::Specular:
				return "specular";
			case TextureType::Ambient:
				return "ambient";
			case TextureType::Emissive:
				return "emissive";
			case TextureType::Height:
				return "height";
			case TextureType::Normals:
				return "normal";
			case TextureType::Shininess:
				return "shininess";
			case TextureType::Opacity:
				return "opacity";
			case TextureType::Displacement:
				return "displacement";
			case TextureType::Lightmap:
				return "light_map";
			case TextureType::Reflection:
				return "reflection";
			case TextureType::BaseColor:
				return "base_color";
			case TextureType::NormalCamera:
				return "normal_camera";
			case TextureType::EmissionColor:
				return "emission_color";
			case TextureType::Metalness:
				return "metalness";
			case TextureType::DiffuseRoughness:
				return "diffuse_roughness";
			case TextureType::AmbientOcclusion:
				return "ambient_occlusion";
			case TextureType::Unknown:
				return "other";
		}

		return {};
	}

	Model Model::Load(pass_ref<Context> context, ResourceManager& resource_manager, const std::string& path)
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
		//auto directory = path.substr(0, path.find_last_of('/'));

		auto model_path = filesystem::path(path);
		auto root_path = model_path.parent_path();

		model.process_node(context, importer, root_path, scene, scene->mRootNode);

		return model;
	}

	// Assimp:
	ref<Material> Model::process_material(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiMaterial* native_material, bool load_textures, bool load_values)
	{
		auto material_obj = memory::allocate<Material>();
		auto& material = (*material_obj);

		//auto texture_types = 0;
		
		if (load_textures)
		{
			// Load and store each texture according to its type:
			enumerate_texture_types([&](TextureType texture_type)
			{
				auto native_type = to_aiTextureType(static_cast<TextureType>(texture_type));
				auto texture_count = native_material->GetTextureCount(native_type);

				if (texture_count == 0)
				{
					return;
				}

				//texture_types++;

				auto& texture_container = textures[texture_type];

				texture_container = TextureArray(texture_count, nullptr);

				for (auto i = 0; i < texture_count; i++)
				{
					aiString path_raw;

					native_material->GetTexture(native_type, i, &path_raw);

					filesystem::path texture_path = path_raw.C_Str();
					bool file_exists = boost::filesystem::exists(texture_path);

					auto& texture_out = texture_container[i];

					if (file_exists)
					{
						// TODO: Review management of 'Texture' resources.
						texture_out = memory::allocate<Texture>(context, path_raw.C_Str());
					}
					else
					{
						auto alternate_path = (root_path / (texture_path.filename()));
						texture_out = memory::allocate<Texture>(context, alternate_path.string());
					}
				}

				auto texture_type_var = get_texture_type_variable(texture_type);

				material[texture_type_var] = texture_container;
			});
		}

		if (load_values)
		{
			// 'alpha':
			if (float transparency; native_material->Get(AI_MATKEY_OPACITY, transparency) == aiReturn_SUCCESS) // AI_MATKEY_COLOR_TRANSPARENT
			{
				auto alpha = transparency; // (1.0f - transparency);

				set_material_var(material, MATERIAL_VAR_ALPHA, alpha);
			}

			// 'diffuse_color':
			if (aiColor4D _diffuse_color; native_material->Get(AI_MATKEY_COLOR_DIFFUSE, _diffuse_color) == aiReturn_SUCCESS) // AI_MATKEY_COLOR_TRANSPARENT
			{
				auto diffuse_color = (*reinterpret_cast<ColorRGBA*>(&_diffuse_color));

				set_material_var(material, MATERIAL_VAR_DIFFUSE_COLOR, (diffuse_color));
			}
		}

		return material_obj;
	}

	void Model::process_node(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node)
	{
		// First node (root):
		if (node == scene->mRootNode)
		{
			for (auto i = 0; i < scene->mNumMaterials; i++)
			{
				const auto* material = scene->mMaterials[i];

				materials.push_back(process_material(context, importer, root_path, scene, material));
			}
		}

		// TODO: Review use of unsigned integer for indexing.

		// Process each mesh tied to this node:
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			auto mesh_index = node->mMeshes[i];
			aiMesh* mesh = scene->mMeshes[mesh_index];

			meshes.push_back(process_mesh(context, importer, root_path, scene, mesh));
		}

		// Recursively process each child node:
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			auto* child = node->mChildren[i];
			process_node(context, importer, root_path, scene, child);
		}
	}

	Model::MeshDescriptor Model::process_mesh(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiMesh* mesh)
	{
		using VertexType = StandardVertex;

		// TODO: Review use of unsigned integer for indexing.
		auto material_index = mesh->mMaterialIndex;

		// TODO: Review concept of 'optional materials'.
		auto& material = materials[material_index];
		
		// Material handling here.

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

		return { mesh_instance, material };
	}
}