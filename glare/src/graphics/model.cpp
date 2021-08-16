#include "model.hpp"
#include "mesh.hpp"
#include "material.hpp"

#include <math/math.hpp>
#include <engine/resource_manager.hpp>

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <utility>

#define AI_CONFIG_PP_PTV_NORMALIZE   "PP_PTV_NORMALIZE"

#include <assimp/Importer.hpp>
#include <assimp/vector3.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <bullet/btBulletCollisionCommon.h>

namespace graphics
{
	// Internal TextureClasss are currently consistent with Assimp.
	static aiTextureType to_aiTextureType(TextureClass type)
	{
		// Mappings for weird texture-handling behavior from Assimp:
		switch (type)
		{
			case TextureClass::Normals:
				return aiTextureType_HEIGHT;
			case TextureClass::Height:
				return aiTextureType_AMBIENT;
		}

		return static_cast<aiTextureType>(type);
	}

	static aiMatrix4x4 get_scene_orientation(const aiScene* scene)
	{
		int upAxis = 1;
		int upAxisSign = 1;

		int frontAxis = 2;
		int frontAxisSign = 1;

		int coordAxis = 0;
		int coordAxisSign = -1;

		if (scene && scene->mMetaData)
		{
			scene->mMetaData->Get<int>("UpAxis", upAxis); // 1
			scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);
			scene->mMetaData->Get<int>("FrontAxis", frontAxis);
			scene->mMetaData->Get<int>("FrontAxisSign", frontAxisSign);
			scene->mMetaData->Get<int>("CoordAxis", coordAxis);
			scene->mMetaData->Get<int>("CoordAxisSign", coordAxisSign);
		}

		aiVector3D upVec = upAxis == 0 ? aiVector3D(upAxisSign, 0, 0) : upAxis == 1 ? aiVector3D(0, upAxisSign, 0) : aiVector3D(0, 0, upAxisSign);
		aiVector3D forwardVec = frontAxis == 0 ? aiVector3D(frontAxisSign, 0, 0) : frontAxis == 1 ? aiVector3D(0, frontAxisSign, 0) : aiVector3D(0, 0, frontAxisSign);
		aiVector3D rightVec = coordAxis == 0 ? aiVector3D(coordAxisSign, 0, 0) : coordAxis == 1 ? aiVector3D(0, coordAxisSign, 0) : aiVector3D(0, 0, coordAxisSign);
		
		return aiMatrix4x4
		(
			rightVec.x, rightVec.y, rightVec.z, 0.0f,
			upVec.x, upVec.y, upVec.z, 0.0f,
			forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
	}

	Model::Model(Meshes&& meshes, VertexWinding vertex_winding) noexcept
		: meshes(std::move(meshes)), vertex_winding(vertex_winding) {}

	Model::Model(Model&& model) noexcept
		: Model() { swap(*this, model); }

	void swap(Model& x, Model& y)
	{
		using std::swap;

		swap(x.meshes, y.meshes);
		swap(x.vertex_winding, y.vertex_winding);
	}

	const char* Model::get_texture_class_variable_raw(TextureClass type)
	{
		switch (type)
		{
			case TextureClass::Diffuse:
				return "diffuse";
			case TextureClass::Specular:
				return "specular";
			//case TextureClass::Ambient:
			//	return "ambient";
			case TextureClass::Emissive:
				return "emissive";
			case TextureClass::Height:
				return "height_map";
			case TextureClass::Normals:
				return "normal_map";
			case TextureClass::Shininess:
				return "shininess";
			case TextureClass::Opacity:
				return "opacity";
			case TextureClass::Displacement:
				return "displacement";
			case TextureClass::Lightmap:
				return "light_map";
			case TextureClass::Reflection:
				return "reflection";
			case TextureClass::BaseColor:
				return "base_color";
			case TextureClass::NormalCamera:
				return "normal_camera";
			case TextureClass::EmissionColor:
				return "emission_color";
			case TextureClass::Metalness:
				return "metalness";
			case TextureClass::DiffuseRoughness:
				return "diffuse_roughness";
			case TextureClass::AmbientOcclusion:
				return "ambient_occlusion";
			case TextureClass::Unknown:
				return "other";
		}

		return {};
	}

	std::tuple<Model, std::optional<Model::CollisionGeometry>> Model::Load(pass_ref<Context> context, const std::string& path, pass_ref<Shader> default_shader, bool load_collision)
	{
		Model model;

		Assimp::Importer importer;

		// TODO: Implement 'flags' parameter.
		unsigned int flags = ( aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices ); // aiProcess_JoinIdenticalVertices

		VertexWinding vert_direction = VertexWinding::Clockwise;

		bool needs_reorientation = false;

		if (path.ends_with(".obj") || path.ends_with(".fbx") || path.ends_with(".3ds"))
		{
			vert_direction = VertexWinding::CounterClockwise;

			// Doesn't work, for some reason.
			//flags |= aiProcess_FlipWindingOrder | aiProcess_PreTransformVertices;

			//flags |= aiProcess_PreTransformVertices;
			//flags |= aiProcess_JoinIdenticalVertices;

			//flags |= aiProcess_MakeLeftHanded;
			needs_reorientation = true;
		}
		
		//flags |= aiProcess_FlipUVs;


		// Load a scene from the path specified.
		const aiScene* scene = importer.ReadFile(path, flags);
		
		if (needs_reorientation)
		{
			aiMatrix4x4 m;
			bool test = m.IsIdentity();

			//aiMatrix4x4::RotationZ(90.0f, m);

			scene->mRootNode->mTransformation *= m;
			//scene->mRootNode->mTransformation *= aiMatrix4x4::RotationX(-90.0f);
		}

		// Ensure the scene was loaded:
		if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			//cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;

			return {};
		}

		aiMatrix4x4 scene_orientation;

		if (needs_reorientation)
		{
			scene_orientation = get_scene_orientation(scene);
		}

		// The root directory of the scene; used to load resources. (Textures, etc)
		//auto directory = path.substr(0, path.find_last_of('/'));

		auto model_path = filesystem::path(path);
		auto root_path = model_path.parent_path();

		CollisionGeometry::Container collision_out;

		model.process_node
		(
			context,
			importer, root_path,
			scene, scene->mRootNode,
			default_shader,
			vert_direction,
			((load_collision) ? &collision_out : nullptr),

			(needs_reorientation) ? reinterpret_cast<_aiMatrix4x4*>(&scene_orientation) : nullptr
		);

		////model.vertex_winding = vert_direction;

		if (load_collision)
		{
			return { std::move(model), std::move(collision_out) };
		}

		return { std::move(model), std::nullopt };
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
			enumerate_texture_types([&](TextureClass texture_type)
			{
				auto native_type = to_aiTextureType(static_cast<TextureClass>(texture_type));
				auto texture_count = native_material->GetTextureCount(native_type);

				if (texture_count == 0)
				{
					return; // Continue enumeration.
				}

				std::cout << "Texture: " << native_type << " (" << texture_count << ")" << '\n';

				auto texture_type_var = get_texture_class_variable(texture_type);

				if (texture_type_var.empty())
				{
					return; // Continue enumeration.
				}

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
			if (aiColor4D _diffuse_color; native_material->Get(AI_MATKEY_COLOR_DIFFUSE, _diffuse_color) == aiReturn_SUCCESS)
			{
				auto diffuse_color = (*reinterpret_cast<ColorRGBA*>(&_diffuse_color));

				material.set_var(Material::DIFFUSE_COLOR, (diffuse_color));
			}

			// 'height_map_scale':
			if (float height_map_scale; native_material->Get(AI_MATKEY_BUMPSCALING, height_map_scale) == aiReturn_SUCCESS)
			{
				material.set_var(Material::HEIGHT_MAP_SCALE, height_map_scale);
			}

			// 'shininess':
			if (float shininess; native_material->Get(AI_MATKEY_SHININESS, shininess) == aiReturn_SUCCESS)
			{
				material.set_var(Material::SHININESS, shininess);
			}
		}

		return material;
	}

	void Model::process_node(pass_ref<Context> context, Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node, pass_ref<Shader> default_shader, VertexWinding vert_direction, CollisionGeometry::Container* opt_collision_out, const _aiMatrix4x4* _scene_orientation)
	{
		bool collision_enabled = (opt_collision_out != nullptr);

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

			auto mesh_data = process_mesh(importer, root_path, scene, node, mesh, vert_direction, _scene_orientation);

			if (collision_enabled)
			{
				opt_collision_out->push_back({ copy_simple_vertices(*mesh), mesh_data.indices }); // TODO: Look into reducing unneeded copies.
			}

			mesh_descriptor.meshes.push_back(Mesh::Generate(context, mesh_data));
		}

		// Recursively process each child node:
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			auto* child = node->mChildren[i];

			process_node(context, importer, root_path, scene, child, default_shader, vert_direction, opt_collision_out, _scene_orientation);
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

	MeshData<Model::VertexType> Model::process_mesh(Assimp::Importer& importer, const filesystem::path& root_path, const aiScene* scene, const aiNode* node, const aiMesh* mesh, VertexWinding vert_direction, const _aiMatrix4x4* _scene_orientation)
	{
		const auto vertex_count = (mesh->mNumVertices);

		MeshData<VertexType> data = { {}, std::make_shared<std::vector<MeshIndex>>() };

		data.vertices.reserve(vertex_count);

		const auto* scene_orientation = reinterpret_cast<const aiMatrix4x4*>(_scene_orientation);

		// Retrieve vertex data:

		// TODO: Look into optimizing for decoupled attributes, if needed.
		// TODO: Review use of unsigned integer for indexing.
		for (unsigned i = 0; i < vertex_count; i++)
		{
			VertexType vertex;

			//aiMatrix4x4 rm = node->mTransformation;
			//aiMatrix4x4::Rotation(1.0f, { 1.0, 1.0, 1.0 }, rm);

			auto uv_channels = mesh->mTextureCoords[0];

			///*
			//if (vert_direction == VertexWinding::CounterClockwise)
			if (false)
			{
				// Test:
				auto p = math::to_vector(mesh->mVertices[i]); // rm * ...

				vertex.position = math::vec3(-p.x, p.y, p.z);

				auto t = math::to_vector(mesh->mTangents[i]);

				vertex.tangent = math::vec3(t.x, t.y, -t.z);

				auto b = math::to_vector(mesh->mBitangents[i]);

				vertex.bitangent = math::vec3(b.x, b.y, b.z);
			}
			else
			{
				auto position  = mesh->mVertices[i];
				auto normal    = mesh->mNormals[i];

				auto tangent   = (mesh->mTangents) ? mesh->mTangents[i] : aiVector3D {};
				auto bitangent = (mesh->mBitangents) ? mesh->mBitangents[i] : aiVector3D {};

				if (scene_orientation)
				{
					const auto& m = (*scene_orientation);

					vertex.position  = math::to_vector(m * position);
					vertex.normal    = math::to_vector(m * normal);
					vertex.tangent   = math::to_vector(m * tangent);
					vertex.bitangent = (math::to_vector(m * bitangent) * -1.0f);

					if (uv_channels)
					{
						auto uv = uv_channels[i];

						//auto uvt = (math::to_vector(m * aiVector3D(1.0 - uv.x, uv.y, 0.0)));
						auto uvt = math::to_vector(uv);

						vertex.uv = math::vec2f(uvt.x, uvt.y);

						//vertex.uv = math::to_vector(uv_channels[i]);

						//auto uv = math::to_vector(uv_channels[i]);;
						//vertex.uv = math::vec2(uv.x, 1.0 - uv.y);
					}
					else
					{
						vertex.uv = {};
					}
				}
				else
				{
					vertex.position  = math::to_vector(position);
					vertex.normal    = math::to_vector(normal);
					vertex.tangent   = math::to_vector(tangent);
					vertex.bitangent = math::to_vector(bitangent);

					if (uv_channels)
					{
						vertex.uv = math::to_vector(uv_channels[i]);

						//vertex.uv.y -= 1.0;

						//auto uv = math::to_vector(uv_channels[i]);;
						//vertex.uv = math::vec2(uv.x, 1.0 - uv.y);
					}
					else
					{
						vertex.uv = {};
					}
				}
			}
			//*/

			//vertex.position = math::to_vector(mesh->mVertices[i]); // rm * ...

			data.vertices.push_back(vertex);
		}

		// Retrieve indices per-face:
		for (unsigned i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				data.indices->push_back(face.mIndices[j]);
			}
		}

		return data;
	}
	
	Model::CollisionGeometry::CollisionGeometry(Container&& mesh_data)
		: mesh_interface(std::make_unique<Descriptor>()), mesh_data(std::move(mesh_data))
	{
		constexpr auto ph_index_type = ((sizeof(SimpleMeshData::Index) == 4) ? PHY_INTEGER : PHY_SHORT);

		for (const auto& m : this->mesh_data)
		{
			btIndexedMesh part = {};

			part.m_vertexBase = reinterpret_cast<const unsigned char*>(m.vertices.data());
			part.m_vertexStride = sizeof(SimpleMeshData::Vertex); // SimpleVertex
			part.m_numVertices = static_cast<int>(m.vertices.size());

			if (m.indices)
			{
				part.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(m.indices->data());
				part.m_triangleIndexStride = (sizeof(SimpleMeshData::Index) * 3); // 3 indices per-triangle.
				part.m_numTriangles = static_cast<int>(m.indices->size() / 3); // 3 indices per-triangle.
				part.m_indexType = ph_index_type;
			}

			mesh_interface->addIndexedMesh(part, ph_index_type);
		}
	}
}