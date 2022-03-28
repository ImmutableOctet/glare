#include "model_loader.hpp"

#include <graphics/texture.hpp>
#include <graphics/vertex.hpp>
#include <util/memory.hpp>
#include <util/string.hpp>

#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <bullet/btBulletCollisionCommon.h>

//#include <type_traits>

#include <util/log.hpp>

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
		//return scene->mRootNode->mTransformation;

		/*
		int upAxis = 1;
		int upAxisSign = 1;

		int frontAxis = 2;
		int frontAxisSign = 1;

		int coordAxis = 0;
		int coordAxisSign = -1;
		*/

		int coordAxis = 0;
		int coordAxisSign = 1;

		int upAxis = 1;
		int upAxisSign = 1;

		int frontAxis = 2;
		int frontAxisSign = -1;

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
		
		auto out = aiMatrix4x4
		(
			rightVec.x, rightVec.y, rightVec.z, 0.0f,
			upVec.x, upVec.y, upVec.z, 0.0f,
			forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		aiMatrix4x4 rotation;
		aiMatrix4x4::RotationZ(math::radians(90.0f), rotation);

		out = out * rotation;

		return out;
	}

	template <typename MeshData>
	static MeshData process_mesh
	(
		const ModelLoader::Config& cfg, const aiScene* scene, const aiNode* node, const aiMesh* mesh,
		const ModelLoader::Skeleton* skeleton=nullptr, const _aiMatrix4x4* orientation=nullptr
	)
	{
		using VertexType = typename MeshData::Vertex;
		using MeshIndex  = ModelLoader::MeshIndex;
		
		bool _test = (util::to_string_view(node->mName) == "Geosphere");

		const auto vertex_count = (mesh->mNumVertices);

		MeshData data = { {}, memory::allocate<std::vector<MeshIndex>>() };

		data.vertices.reserve(vertex_count);

		// Retrieve vertex data:

		// TODO: Look into optimizing for decoupled attributes, if needed.
		// TODO: Review use of unsigned integer for indexing.
		for (unsigned i = 0; i < vertex_count; i++)
		{
			VertexType vertex; // = {};

			//aiMatrix4x4 rm = node->mTransformation;
			//aiMatrix4x4::Rotation(1.0f, { 1.0, 1.0, 1.0 }, rm);

			auto uv_channels = mesh->mTextureCoords[0];

			auto position  = mesh->mVertices[i];
			auto normal    = mesh->mNormals[i];

			auto tangent   = (mesh->mTangents) ? mesh->mTangents[i] : aiVector3D {};
			auto bitangent = (mesh->mBitangents) ? mesh->mBitangents[i] : aiVector3D {};

			if (orientation)
			{
				const auto& _m = (*reinterpret_cast<const aiMatrix4x4*>(orientation));
				//auto m = _m; m.Inverse();
				const auto& m = _m;

				bool _is_identity = m.IsIdentity();

				vertex.position  =  math::to_vector(m * position);
				vertex.normal    =  math::to_vector(m * normal);
				vertex.tangent   =  math::to_vector(m * tangent);

				////vertex.bitangent = (math::to_vector(m * bitangent) * -1.0f);
				vertex.bitangent = math::to_vector(m * bitangent);

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

			if constexpr (MeshData::is_animated_vertex)
			{
				// `-1` is used to indicate no binding.
				vertex.bone_indices = { -1, -1, -1, -1 };
				vertex.bone_weights = {};
			}

			data.vertices.push_back(vertex);
		}

		if constexpr (MeshData::is_animated_vertex)
		{
			if (skeleton)
			{
				//int weight_channel = 0;

				for (unsigned int i = 0; i < mesh->mNumBones; i++)
				{
					auto* bone_raw = mesh->mBones[i];
					auto* bone = skeleton->get_bone(bone_raw);

					assert(bone);

					for (unsigned int w = 0; w < bone_raw->mNumWeights; w++)
					{
						const aiVertexWeight& weight = bone_raw->mWeights[w];
						auto& vertex = data.vertices[weight.mVertexId];

						auto weight_channel = get_next_weight_channel(vertex);

						if (weight_channel == -1)
						{
							continue;
						}

						vertex.bone_indices[weight_channel] = static_cast<graphics::BoneIndexType>(bone->id);
						vertex.bone_weights[weight_channel] = weight.mWeight;
					}

					/*
					if (++weight_channel >= graphics::VERTEX_MAX_BONE_INFLUENCE)
					{
						break;
					}
					*/
				}
			}
		}

		// Retrieve indices per-face:
		for (unsigned i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];

			/*
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				data.indices->push_back(face.mIndices[j]);
			}
			*/

			///*
			assert(face.mNumIndices == 3);

			const auto& a = face.mIndices[0];
			const auto& b = face.mIndices[1];
			const auto& c = face.mIndices[2];

			switch (cfg.orientation.vert_direction)
			{
				case graphics::VertexWinding::CounterClockwise:
					data.indices->push_back(a);
					data.indices->push_back(b);
					data.indices->push_back(c);

					break;
				case graphics::VertexWinding::Clockwise:
					data.indices->push_back(b);
					data.indices->push_back(c);
					data.indices->push_back(a);

					break;
			}
			//*/
		}

		return data;
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
		unsigned int flags = (aiProcess_Triangulate | aiProcess_CalcTangentSpace); // aiProcess_PreTransformVertices // aiProcess_JoinIdenticalVertices // aiProcess_GenSmoothNormals

		if (native_flags)
		{
			flags = *native_flags;
		}

		//flags |= aiProcess_FlipUVs;

		//flags |= aiProcess_MakeLeftHanded;
		//flags |= aiProcess_ConvertToLeftHanded;
		//flags |= aiProcess_FlipWindingOrder;
		//flags |= aiProcess_ValidateDataStructure;

		//bool _dbg = (path == "assets/geometry/boxes.b3d");

		if (path.ends_with(".b3d"))
		{
			//flags |= aiProcess_MakeLeftHanded;
			//flags |= aiProcess_PreTransformVertices; // <-- This removes animations.
			//flags |= aiProcess_ConvertToLeftHanded;

			// Doesn't work, for some reason.
			//flags |= aiProcess_FlipWindingOrder | aiProcess_PreTransformVertices;


			//flags |= aiProcess_PreTransformVertices;
			//flags |= aiProcess_JoinIdenticalVertices;

			cfg.orientation.vert_direction = graphics::VertexWinding::Clockwise;
			//flags |= aiProcess_FlipWindingOrder;
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

		bool is_animated = (cfg.is_animated.value_or(scene->HasAnimations())); // (scene->mNumAnimations > 0);

		// Ensure the `is_animated` flag is set.
		cfg.is_animated = is_animated;

		aiMatrix4x4 scene_orientation;

		//scene_orientation = get_scene_orientation(scene, false);
		scene_orientation = scene->mRootNode->mTransformation;
		//scene_orientation.Inverse();

		// Handle materials & textures:
		materials.reserve(scene->mNumMaterials);
			
		for (decltype(scene->mNumMaterials) i = 0; i < scene->mNumMaterials; i++) // auto
		{
			const auto* material = scene->mMaterials[i];

			materials.emplace_back(process_material(scene, material)); // materials[i] = ...
		}

		process_node
		(
			scene,
			scene->mRootNode,

			reinterpret_cast<_aiMatrix4x4*>(&scene_orientation),
			reinterpret_cast<_aiMatrix4x4*>(&scene_orientation)
		);

		if (is_animated)
		{
			print("Generating missing bones...");

			unsigned int additional_bones = 0;

			for (const auto& bone_entry : skeleton.bones)
			{
				additional_bones += handle_missing_bone(*scene, skeleton, bone_entry.second.parent_name);
			}

			print("{} additional bone(s) generated.", additional_bones);

			process_animations(scene, skeleton, nullptr); // reinterpret_cast<_aiMatrix4x4*>(&inv_root_tform) <-- Not needed; these are relative movements.
		}

		return get_model_storage();
	}

	unsigned int ModelLoader::handle_missing_bone(const aiScene& scene, Skeleton& skeleton, const std::string& bone_name, bool recursive)
	{
		if (bone_name.empty())
		{
			return 0;
		}

		unsigned int bones_generated = 0;

		auto& bones = skeleton.bones;
		auto bone_it = bones.find(bone_name);

		if (bone_it == bones.end())
		{
			print("Generating missing bone: \"{}\"", bone_name);

			auto* bone = process_bone(scene, skeleton, aiString(bone_name), {});

			if (bone)
			{
				if (recursive)
				{
					bones_generated += handle_missing_bone(scene, skeleton, bone->parent_name, true);
				}

				bones_generated++;
			}
		}

		return bones_generated;
	}

	bool ModelLoader::has_model_storage() const
	{
		return (model_storage.index() != NoStorage);
	}

	template <typename MeshData>
	static graphics::Mesh handle_mesh(ModelLoader& loader, const aiScene* scene, const aiNode* node, aiMesh* mesh, ModelLoader::Model::CollisionGeometry::Container* collision_geometry=nullptr, const ModelLoader::Skeleton* skeleton=nullptr, const _aiMatrix4x4* scene_orientation=nullptr)
	{
		auto mesh_data = process_mesh<MeshData>(loader.get_config(), scene, node, mesh, skeleton, scene_orientation);

		// TODO: Add additional step to look up separate collision mesh, rather than using raw mesh data from graphical element.
		if (collision_geometry)
		{
			collision_geometry->emplace_back(graphics::copy_simple_vertices(*mesh), mesh_data.indices); // TODO: Look into reducing unneeded copies.
		}

		return graphics::Mesh::Generate(loader.get_context(), mesh_data);
	}

	void ModelLoader::process_node(const aiScene* scene, const aiNode* node, const _aiMatrix4x4* orientation, const _aiMatrix4x4* global_orientation)
	{
		print("Node: {}, Children: {}, Meshes: {}", util::to_string_view(node->mName), node->mNumChildren, node->mNumMeshes);

		aiMatrix4x4 node_matrix;

		if (orientation)
		{
			node_matrix = ((*reinterpret_cast<const aiMatrix4x4*>(orientation)) * node->mTransformation);
		}
		else
		{
			node_matrix = node->mTransformation;
		}

		bool is_model_node = (node->mNumMeshes > 0);

		if (is_model_node)
		{
			Model model;
			Model::CollisionGeometry::Container collision_geometry;

			bool collision_enabled = cfg.load_collision;
			bool is_animated = cfg.is_animated.value_or(false);

			model.animated = is_animated;

			if (node->mName.length > 0)
			{
				model.name = util::to_string_view(node->mName);
			}

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

				assert(mesh);

				print("Bones: {}", mesh->mNumBones);

				if (mesh->mNumBones > 0)
				{
					process_bones(*scene, *node, *mesh, skeleton);
				}

				auto material_index = mesh->mMaterialIndex;
				auto& mesh_descriptor = model.meshes[material_index];

				Mesh mesh_out;

				if (is_animated)
				{
					mesh_out = handle_mesh<AnimMeshData>
					(
						*this, scene, node, mesh,
						((collision_enabled) ? &collision_geometry : nullptr),
						((skeleton.exists()) ? &skeleton : nullptr),

						//orientation
						//reinterpret_cast<const _aiMatrix4x4*>(&node_matrix)
						//reinterpret_cast<const _aiMatrix4x4*>(&node->mTransformation)
						//nullptr
						
						global_orientation
					);
				}
				else
				{
					//auto _global = *reinterpret_cast<const aiMatrix4x4*>(global_orientation); _global.Inverse();
					//aiMatrix4x4 _inv_parent;

					/*
					if (node->mParent)
					{
						_inv_parent = node->mParent->mTransformation; _inv_parent.Inverse();
					}
					*/

					//_inv_parent = node->mTransformation; _inv_parent.Inverse();

					//auto test = _global * node_matrix;
					//auto test = _global * node_matrix * _inv_parent;
					//auto test = _global * node->mTransformation;
					//test.Inverse();

					mesh_out = handle_mesh<MeshData>
					(
						*this, scene, node, mesh,
						((collision_enabled) ? &collision_geometry : nullptr),
						((skeleton.exists()) ? &skeleton : nullptr),
						
						//reinterpret_cast<const _aiMatrix4x4*>(&test)

						//orientation
						//reinterpret_cast<const _aiMatrix4x4*>(&node_matrix)
						//reinterpret_cast<const _aiMatrix4x4*>(&node->mTransformation)
						//nullptr
						
						global_orientation
					);
				}

				mesh_descriptor.meshes.push_back(std::move(mesh_out));
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

			//model.vertex_winding = cfg.orientation.vert_direction;

			store_model_data
			(
				ModelData
				{
					std::move(model),

					math::to_matrix(node_matrix),

					// Construct collision geometry object from populated container.
					(collision_enabled)
					? std::optional<Model::CollisionGeometry>{std::move(collision_geometry)}
					: std::nullopt,

					((skeleton.exists()) ? (&skeleton) : nullptr),
					((!animations.empty()) ? (&animations) : nullptr)
				}
			);
		}

		// Recursively process each child node:
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			const auto* child = node->mChildren[i];

			process_node(scene, child, reinterpret_cast<const _aiMatrix4x4*>(&node_matrix), global_orientation);
		}
	}

	static aiMatrix4x4 get_recursive_transform(const aiScene& scene, const aiNode& node, bool inv_root=false)
	{
		if (node.mParent != nullptr)
		{
			return (get_recursive_transform(scene, *node.mParent, inv_root) * node.mTransformation);
		}
		else if ((inv_root) && (&node == scene.mRootNode))
		{
			auto inv_root_matrix = node.mTransformation;
			inv_root_matrix.Inverse();

			return inv_root_matrix;
		}

		return node.mTransformation;
	}

	static aiMatrix4x4 get_bone_transform(const aiScene& scene, const aiString& bone_name)
	{
		auto* bone_node = scene.mRootNode->FindNode(bone_name);

		if (!bone_node)
		{
			return {}; // Identity matrix.
		}

		return get_recursive_transform(scene, *bone_node, true);
	}

	const graphics::Bone* ModelLoader::process_bone(const aiScene& scene, Skeleton& skeleton, const aiString& bone_name, const aiMatrix4x4& offset_matrix) // std::string_view bone_name
	{
		std::string parent_bone_name;

		const auto* bone_node = scene.mRootNode->FindNode(bone_name);

		if (!bone_node)
		{
			print_warn("Unable to determine node for bone \"{}\"", util::to_string_view(bone_name));

			return nullptr;
		}

		if (bone_node->mParent)
		{
			if (bone_node->mParent->mName.length > 0)
			{
				parent_bone_name = std::string(util::to_string_view(bone_node->mParent->mName));
			}
		}

		/*
		auto local_transform = bone_node->mTransformation;

		if (parent_bone_name.empty())
		{
			auto inv_root_matrix = scene.mRootNode->mTransformation; inv_root_matrix.Inverse();
			local_transform = inv_root_matrix * local_transform;
		}

		// Debugging related:
		//auto tform = get_bone_transform(*scene, bone_data->mName);
		//auto tform = get_recursive_transform(scene, *bone_node, true);

		const auto& tform = local_transform;
		*/

		///*
		auto bt = bone_node->mTransformation;

		if (parent_bone_name.empty())
		{
			auto inv_root_matrix = scene.mRootNode->mTransformation; inv_root_matrix.Inverse();
			bt = inv_root_matrix * bt;
		}

		const auto& tform = bt;
		//*/

		print("Creating bone: \"{}\" (parent: \"{}\")", util::to_string_view(bone_name), (parent_bone_name.empty()) ? "null" : parent_bone_name);

		//return skeleton.add_bone(&bone_data, math::to_matrix(tform), parent_bone_name);

		return skeleton.add_bone
		(
			util::to_string_view(bone_name),
			math::to_matrix(tform),
			math::to_matrix(offset_matrix),
			parent_bone_name
		);

		//const Bone* add_bone(std::string_view name, const math::Matrix & node_transform, const math::Matrix & offset, const std::string & parent_bone_name = {});
		//const Bone* add_bone(std::string_view name, const std::string & parent_bone_name = {});
		//const Bone* add_bone(const aiBone * bone_raw, const math::Matrix & node_transform, const std::string & parent_bone_name = {});
	}

	unsigned int ModelLoader::process_bones(const aiScene& scene, const aiNode& node, const aiMesh& mesh, Skeleton& skeleton)
	{
		unsigned int bones_processed = 0;

		for (unsigned int i = 0; i < mesh.mNumBones; i++)
		{
			const aiBone* bone_data = mesh.mBones[i];

			auto* bone_ptr = skeleton.get_bone(bone_data);

			if ((!bone_ptr) && (bone_data))
			{
				if (process_bone(scene, skeleton, bone_data->mName, bone_data->mOffsetMatrix))
				{
					bones_processed++;
				}
			}
		}

		return bones_processed;
	}

	const std::vector<ModelLoader::Animation> ModelLoader::process_animations(const aiScene* scene, Skeleton& skeleton, const _aiMatrix4x4* orientation)
	{
		assert(scene);
		assert(scene->mRootNode);

		math::Matrix tform;

		if (orientation)
		{
			tform = math::to_matrix(*(reinterpret_cast<const aiMatrix4x4*>(orientation)));
		}
		else
		{
			tform = math::identity_matrix();
		}

		for (unsigned int a = 0; a < scene->mNumAnimations; a++)
		{
			auto* a_raw = scene->mAnimations[a];

			float duration = static_cast<float>(a_raw->mDuration);
			float rate     = static_cast<float>(a_raw->mTicksPerSecond);

			Animation::FrameData frame_data;

			for (unsigned int c = 0; c < a_raw->mNumChannels; c++)
			{
				auto* channel = a_raw->mChannels[c];
				auto bone_name = util::to_string_view(channel->mNodeName);

				auto* bone = skeleton.get_bone(bone_name);
				//auto* bone = skeleton.get_or_create_bone(bone_name);

				//assert(bone);

				if (!bone)
				{
					print_warn("Unable to resolve bone: {}", bone_name);

					continue;
				}

				auto bone_id = bone->id;

				frame_data[bone_id] = Animation::KeySequence(*channel, tform);
			}

			animations.emplace_back(static_cast<AnimationID>(a), duration, rate, frame_data);
		}

		return animations;
	}

	ref<ModelLoader::Material> ModelLoader::process_material
	(
		const aiScene* scene,
		const aiMaterial* native_material,
		bool load_values,
		bool load_textures
	)
	{
		bool is_animated = cfg.is_animated.value_or(false);
		auto material = memory::allocate<Material>((is_animated) ? default_animated_shader : default_shader);

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

				print("Texture: {} ({})", native_type, texture_count);

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