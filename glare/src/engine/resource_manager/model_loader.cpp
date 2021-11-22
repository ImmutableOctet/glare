#include "model_loader.hpp"

#include <graphics/texture.hpp>
#include <util/memory.hpp>

#include <assimp/vector3.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <bullet/btBulletCollisionCommon.h>

// Debugging related:
#include <iostream>

namespace engine
{
	// Internal TextureClasss are currently consistent with Assimp.
	static aiTextureType to_aiTextureType(graphics::TextureClass type)
	{
		// Mappings for weird texture-handling behavior from Assimp:
		switch (type)
		{
			case graphics::TextureClass::Normals:
				return aiTextureType_HEIGHT;
			case graphics::TextureClass::Height:
				return aiTextureType_AMBIENT;
		}

		return static_cast<aiTextureType>(type);
	}

	static aiMatrix4x4 get_scene_orientation(const aiScene* scene, bool is_right_handed)
	{
		int upAxis = 1;
		int upAxisSign = 1;

		int frontAxis = 2;
		int frontAxisSign = 1;

		int coordAxis = 0;
		int coordAxisSign = -1;

		if (is_right_handed)
		{
			//upAxis = 1;
			//upAxisSign = 1;

			//frontAxis = 2;
			frontAxisSign = -1;

			//coordAxis = 0;
			coordAxisSign = 1;
		}

		if (scene && scene->mMetaData)
		{
			scene->mMetaData->Get<int>("UpAxis", upAxis); // 1
			scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);
			scene->mMetaData->Get<int>("FrontAxis", frontAxis);
			scene->mMetaData->Get<int>("FrontAxisSign", frontAxisSign);
			scene->mMetaData->Get<int>("CoordAxis", coordAxis);
			scene->mMetaData->Get<int>("CoordAxisSign", coordAxisSign);
		}

		aiVector3D upVec = upAxis == 0 ? aiVector3D(static_cast<ai_real>(upAxisSign), 0, 0) : upAxis == 1 ? aiVector3D(0, static_cast<ai_real>(upAxisSign), 0) : aiVector3D(0, 0, static_cast<ai_real>(upAxisSign));
		aiVector3D forwardVec = frontAxis == 0 ? aiVector3D(static_cast<ai_real>(frontAxisSign), 0, 0) : frontAxis == 1 ? aiVector3D(0, static_cast<ai_real>(frontAxisSign), 0) : aiVector3D(0, 0, static_cast<ai_real>(frontAxisSign));
		aiVector3D rightVec = coordAxis == 0 ? aiVector3D(static_cast<ai_real>(coordAxisSign), 0, 0) : coordAxis == 1 ? aiVector3D(0, static_cast<ai_real>(coordAxisSign), 0) : aiVector3D(0, 0, static_cast<ai_real>(coordAxisSign));
		
		return aiMatrix4x4
		(
			rightVec.x, rightVec.y, rightVec.z, 0.0f,
			upVec.x, upVec.y, upVec.z, 0.0f,
			forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
	}

	ModelLoader::ModelLoader
	(
		pass_ref<graphics::Context> context,
		pass_ref<graphics::Shader> default_shader,
		pass_ref<graphics::Shader> default_animated_shader,
		const Config& cfg
	) :
		context(context),
		default_shader(default_shader),
		default_animated_shader(default_animated_shader),
		cfg(cfg)
	{
	}

	ModelLoader::ModelLoader
	(
		pass_ref<graphics::Context> context,
		pass_ref<graphics::Shader> default_shader,
		pass_ref<graphics::Shader> default_animated_shader,
		const filesystem::path& filepath,
		std::optional<NativeFlags> native_flags,
		const Config& cfg
	)
		: ModelLoader(context, default_shader, default_animated_shader, cfg)
	{
		load(filepath, native_flags);
	}

	ModelLoader::ModelStorage& ModelLoader::load
	(
		const filesystem::path& filepath,
		std::optional<NativeFlags> native_flags,
		bool update_root_path
	)
	{
		auto path = filepath.string();

		// TODO: Implement 'flags' parameter.
		unsigned int flags = (aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices); // aiProcess_JoinIdenticalVertices

		if (native_flags)
		{
			flags = *native_flags;
		}

		//flags |= aiProcess_FlipUVs;

		if (!cfg.orientation.vert_direction.has_value())
		{
			graphics::VertexWinding vert_direction = graphics::VertexWinding::Clockwise;

			if (path.ends_with(".fbx") || path.ends_with(".3ds") || path.ends_with(".dae") || path.ends_with(".obj"))
			{
				flags |= aiProcess_MakeLeftHanded;
				//flags |= aiProcess_PreTransformVertices;
				//flags |= aiProcess_ConvertToLeftHanded;

				// Doesn't work, for some reason.
				//flags |= aiProcess_FlipWindingOrder | aiProcess_PreTransformVertices;

				//flags |= aiProcess_PreTransformVertices;
				//flags |= aiProcess_JoinIdenticalVertices;

				if (path.ends_with(".obj"))
				{
					cfg.orientation.flip_normals = true;
				}
				else
				{
					vert_direction = graphics::VertexWinding::CounterClockwise;
				}

				cfg.orientation.is_right_handed = true;
				cfg.orientation.needs_reorientation = true;
			}

			cfg.orientation.vert_direction = vert_direction;
		}

		Assimp::Importer importer;

		// Load a scene from the path specified.
		const auto* scene = importer.ReadFile(path, flags);

		// Ensure the scene was loaded:
		if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			//cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;

			return get_model_storage(); // {};
		}

		if ((update_root_path) || (!cfg.root_path.has_value()))
		{
			cfg.root_path = filepath.parent_path();
		}

		if (!cfg.is_animated.has_value())
		{
			cfg.is_animated = (scene->mNumAnimations > 0);
		}

		aiMatrix4x4 scene_orientation;

		if (cfg.orientation.needs_reorientation)
		{
			scene_orientation = get_scene_orientation(scene, cfg.orientation.is_right_handed);
		}

		// Handle materials & textures:
		//if (node == scene->mRootNode)
		{
			materials.reserve(scene->mNumMaterials);
			
			for (decltype(scene->mNumMaterials) i = 0; i < scene->mNumMaterials; i++) // auto
			{
				const auto* material = scene->mMaterials[i];

				materials.emplace_back(process_material(scene, material)); // materials[i] = ...
			}
		}

		//ModelData model_data;
		process_node
		(
			scene,
			scene->mRootNode,

			((cfg.orientation.needs_reorientation) ? reinterpret_cast<_aiMatrix4x4*>(&scene_orientation) : nullptr)
		);

		return get_model_storage();
	}

	bool ModelLoader::has_model_storage() const
	{
		return (model_storage.index() != NoStorage);
	}

	void ModelLoader::process_node(const aiScene* scene, const aiNode* node, const _aiMatrix4x4* scene_orientation)
	{
		bool is_model_node = (node->mNumMeshes > 0);

		if (is_model_node)
		{
			Model model;
			Model::CollisionGeometry::Container collision_geometry;

			bool collision_enabled = cfg.load_collision;

			// Allocate mesh descriptors for each material:
			model.meshes.reserve(materials.size());

			for (const auto& material : materials)
			//for (std::size_t i = 0; i < materials.size(); i++) // model.meshes.size()
			{
				//model.meshes[i] = Model::MeshDescriptor(materials[i]);
				model.meshes.emplace_back(material);
			}

			// TODO: Review use of unsigned integer for indexing.
			for (unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				auto mesh_index = node->mMeshes[i];
				aiMesh* mesh = scene->mMeshes[mesh_index];

				auto material_index = mesh->mMaterialIndex;
				auto& mesh_descriptor = model.meshes[material_index];

				auto mesh_data = process_mesh(scene, node, mesh, scene_orientation);

				// TODO: Add additional step to look up separate collision mesh, rather than using raw mesh data from graphical element.
				if (collision_enabled)
				{
					collision_geometry.push_back({ graphics::copy_simple_vertices(*mesh), mesh_data.indices }); // TODO: Look into reducing unneeded copies.
				}

				mesh_descriptor.meshes.push_back(Mesh::Generate(context, mesh_data));
			}

			// First node (root):
			//if (node == scene->mRootNode)
			{
				// Remove empty mesh descriptors:
				model.meshes.erase
				(
					std::remove_if
					(
						model.meshes.begin(), model.meshes.end(),
						[](const auto& descriptor) -> bool
						{
							bool res = (!descriptor);

							return res;
						}
					),

					model.meshes.end()
				);

				// TODO: Review why this was added.
				// Sort mesh descriptors based on numebr of textures.
				std::sort(model.meshes.begin(), model.meshes.end(),
				[](const auto& a, const auto& b)
				{
					return (a.material->textures.size() > b.material->textures.size());
				});
			}

			model.vertex_winding = *cfg.orientation.vert_direction;

			store_model_data
			(
				{
					std::move(model),

					// Construct collision geometry object from populated container.
					(collision_enabled)
					? std::optional<Model::CollisionGeometry>{std::move(collision_geometry)}
					: std::nullopt
				}
			);
		}

		// Recursively process each child node:
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			const auto* child = node->mChildren[i];

			process_node(scene, child, scene_orientation);
		}
	}

	ModelLoader::MeshData ModelLoader::process_mesh(const aiScene* scene, const aiNode* node, const aiMesh* mesh, const _aiMatrix4x4* _scene_orientation)
	{
		const auto vertex_count = (mesh->mNumVertices);

		MeshData data = { {}, memory::allocate<std::vector<MeshIndex>>() };

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

			if (cfg.orientation.flip_normals)
			{
				vertex.normal *= -1;
			}

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

	ref<ModelLoader::Material> ModelLoader::process_material
	(
		const aiScene* scene,
		const aiMaterial* native_material,
		bool load_values,
		bool load_textures
	)
	{
		auto material = memory::allocate<Material>(default_shader);

		//auto texture_types = 0;
		
		if (load_textures)
		{
			// Load and store each texture according to its type:
			graphics::enumerate_texture_types([&](graphics::TextureClass texture_type)
			{
				auto native_type = to_aiTextureType(static_cast<graphics::TextureClass>(texture_type));
				auto texture_count = native_material->GetTextureCount(native_type);

				if (texture_count == 0)
				{
					return; // Continue enumeration.
				}

				std::cout << "Texture: " << native_type << " (" << texture_count << ")" << '\n';

				auto texture_type_var = Model::get_texture_class_variable(texture_type);

				if (texture_type_var.empty())
				{
					return; // Continue enumeration.
				}

				if (texture_count > 1)
				{
					auto texture_container = graphics::TextureArray(texture_count, nullptr);

					for (decltype(texture_count) i = 0; i < texture_count; i++) // auto
					{
						aiString path_raw;

						native_material->GetTexture(native_type, i, &path_raw);

						filesystem::path texture_path = path_raw.C_Str();
						
						texture_container[i] = process_texture(texture_path);
					}

					material->textures[texture_type_var] = std::move(texture_container);
				}
				else
				{
					aiString path_raw;

					native_material->GetTexture(native_type, 0, &path_raw);

					filesystem::path texture_path = path_raw.C_Str();

					material->textures[texture_type_var] = process_texture(texture_path);
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

				material->set_var(Material::ALPHA, alpha);
			}

			// 'diffuse_color':
			if (aiColor4D _diffuse_color; native_material->Get(AI_MATKEY_COLOR_DIFFUSE, _diffuse_color) == aiReturn_SUCCESS)
			{
				auto diffuse_color = (*reinterpret_cast<graphics::ColorRGBA*>(&_diffuse_color));

				material->set_var(Material::DIFFUSE_COLOR, (diffuse_color));
			}

			// 'height_map_scale':
			if (float height_map_scale; native_material->Get(AI_MATKEY_BUMPSCALING, height_map_scale) == aiReturn_SUCCESS)
			{
				material->set_var(Material::HEIGHT_MAP_SCALE, height_map_scale);
			}

			// 'shininess':
			if (float shininess; native_material->Get(AI_MATKEY_SHININESS, shininess) == aiReturn_SUCCESS)
			{
				material->set_var(Material::SHININESS, shininess);
			}
		}

		if (on_material)
		{
			on_material(*this, *material);
		}

		return material;
	}

	ref<ModelLoader::Texture> ModelLoader::process_texture(const filesystem::path& texture_path)
	{
		bool file_exists = filesystem::exists(texture_path);

		ref<Texture> t;

		if (file_exists)
		{
			// TODO: Review management of 'Texture' resources.
			t = memory::allocate<Texture>(context, texture_path.string());
		}
		else
		{
			auto alternate_path = (*cfg.root_path / (texture_path.filename()));

			alternate_path.make_preferred();

			t = memory::allocate<Texture>(context, alternate_path.string());
		}

		if (on_texture)
		{
			on_texture(*this, *t);
		}

		return t;
	}

	void ModelLoader::store_model_data(ModelData&& model)
	{
		if (on_model)
		{
			on_model(*this, model);
		}

		if ((!cfg.maintain_storage) || (!model))
		{
			return;
		}

		switch (get_storage_type())
		{
			case StorageType<ModelVector>():
			{
				auto& model_container = std::get<ModelVector>(model_storage);

				model_container.push_back(std::move(model));

				break;
			}
			case StorageType<ModelData>():
			{
				ModelVector model_container;

				model_container.push_back(std::move(std::get<ModelData>(model_storage)));
				model_container.push_back(std::move(model));

				model_storage = std::move(model_container);

				break;
			}

			//case NoStorage:
			default:
				model_storage = std::move(model);

				break;
		}
	}
}