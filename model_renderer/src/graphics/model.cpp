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

#include <filesystem>

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

	Model::Model(Meshes&& meshes) noexcept
		: meshes(std::move(meshes)) {}

	Model::Model(Model&& model) noexcept
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

	Model Model::Load(pass_ref<Context> context, ResourceManager& resource_manager, const std::string& path, pass_ref<Shader> default_shader)
	{
		Model model;

		Assimp::Importer importer;

		// TODO: Implement 'flags' parameter.
		unsigned int flags = ( aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace ); // aiProcess_PreTransformVertices  // aiProcess_JoinIdenticalVertices

		VertexWinding vert_direction = VertexWinding::Clockwise;

		/*
		if (!path.ends_with(".obj"))
		{
			vert_direction = VertexWinding::CounterClockwise;
		}
		*/

		flags |= aiProcess_FlipWindingOrder;

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

		model.process_node(context, importer, root_path, scene, scene->mRootNode, default_shader, vert_direction);

		return model;
	}

	ref<Texture> Model::process_texture(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const filesystem::path& texture_path)
	{
		bool file_exists = filesystem::exists(texture_path);

		if (file_exists)
		{
			// TODO: Review management of 'Texture' resources.
			return memory::allocate<Texture>(context, texture_path.string());
		}
		else
		{
			auto alternate_path = (root_path / (texture_path.filename()));

			alternate_path.make_preferred();

			return memory::allocate<Texture>(context, alternate_path.string());
		}
	}

	// Assimp:
	Material Model::process_material(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiMaterial* native_material, pass_ref<Shader> default_shader, bool load_textures, bool load_values)
	{
		auto material = Material(default_shader);

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

				auto texture_type_var = get_texture_type_variable(texture_type);

				if (texture_count > 1)
				{
					auto texture_container = TextureArray(texture_count, nullptr);

					for (decltype(texture_count) i = 0; i < texture_count; i++) // auto
					{
						aiString path_raw;

						native_material->GetTexture(native_type, i, &path_raw);

						filesystem::path texture_path = path_raw.C_Str();
						
						texture_container[i] = process_texture(context, importer, root_path, texture_path);
					}

					material.textures[texture_type_var] = std::move(texture_container);
				}
				else
				{
					aiString path_raw;

					native_material->GetTexture(native_type, 0, &path_raw);

					filesystem::path texture_path = path_raw.C_Str();

					material.textures[texture_type_var] = process_texture(context, importer, root_path, texture_path);
				}

				//texture_types++;
			});
		}

		if (load_values)
		{
			// 'alpha':
			if (float transparency; native_material->Get(AI_MATKEY_OPACITY, transparency) == aiReturn_SUCCESS) // AI_MATKEY_COLOR_TRANSPARENT
			{
				auto alpha = transparency; // (1.0f - transparency);

				material.set_var(Material::ALPHA, alpha);
			}

			// 'diffuse_color':
			if (aiColor4D _diffuse_color; native_material->Get(AI_MATKEY_COLOR_DIFFUSE, _diffuse_color) == aiReturn_SUCCESS) // AI_MATKEY_COLOR_TRANSPARENT
			{
				auto diffuse_color = (*reinterpret_cast<ColorRGBA*>(&_diffuse_color));

				material.set_var(Material::DIFFUSE_COLOR, (diffuse_color));
			}
		}

		return material;
	}

	void Model::process_node(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node, pass_ref<Shader> default_shader, VertexWinding vert_direction)
	{
		// First node (root):
		if (node == scene->mRootNode)
		{
			meshes.reserve(scene->mNumMaterials);

			for (decltype(scene->mNumMaterials) i = 0; i < scene->mNumMaterials; i++) // auto
			{
				const auto* material = scene->mMaterials[i];

				meshes.emplace_back(process_material(context, importer, root_path, scene, material, default_shader, true, true)); // meshes[i] = ...
			}
		}

		// TODO: Review use of unsigned integer for indexing.

		// Process each mesh tied to this node:
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			auto mesh_index = node->mMeshes[i];
			aiMesh* mesh = scene->mMeshes[mesh_index];

			// TODO: Review use of unsigned integer for indexing.
			auto material_index = mesh->mMaterialIndex;

			// TODO: Review concept of 'optional materials'.
			auto& mesh_descriptor = meshes[material_index];

			mesh_descriptor.meshes.push_back(process_mesh(context, importer, root_path, scene, node, mesh, vert_direction));
		}

		// Recursively process each child node:
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			auto* child = node->mChildren[i];

			process_node(context, importer, root_path, scene, child, default_shader, vert_direction);
		}

		// First node (root):
		if (node == scene->mRootNode)
		{
			meshes.erase
			(
				std::remove_if
				(
					meshes.begin(), meshes.end(),
					[](const auto& descriptor) -> bool
					{
						bool res = (!descriptor);

						return res;
					}
				),

				meshes.end()
			);

			std::sort(meshes.begin(), meshes.end(),
			[](const auto& a, const auto& b)
			{
				return (a.material.textures.size() > b.material.textures.size());
			});
		}
	}

	Mesh Model::process_mesh(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node, const aiMesh* mesh, VertexWinding vert_direction)
	{
		using VertexType = StandardVertex;

		const auto vertex_count = (mesh->mNumVertices);

		MeshData<VertexType> data = {};

		data.vertices.reserve(vertex_count);

		// Retrieve vertex data:

		// TODO: Look into optimizing for decoupled attributes, if needed.
		// TODO: Review use of unsigned integer for indexing.
		for (unsigned i = 0; i < vertex_count; i++)
		{
			VertexType vertex;

			//aiMatrix4x4 rm = node->mTransformation;
			//aiMatrix4x4::Rotation(1.0f, { 1.0, 1.0, 1.0 }, rm);

			vertex.position = to_vector(mesh->mVertices[i]); // rm * ...
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

		return Mesh::Generate(context, data);
	}
}