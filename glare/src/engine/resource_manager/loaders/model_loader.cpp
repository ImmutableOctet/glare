#include "model_loader.hpp"

#include <engine/meta/hash.hpp>

#include <util/memory.hpp>
#include <util/string.hpp>

#include <math/types.hpp>
#include <math/assimp.hpp>

#include <graphics/texture.hpp>
#include <graphics/vertex.hpp>

#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/anim.h>

#include <bullet/btBulletCollisionCommon.h>

#include <optional>
//#include <type_traits>

#include <cassert>

// Debugging related:
#include <util/log.hpp>

#include <iostream>

namespace engine
{
	namespace impl
	{
		template <typename KeyTypeIn, typename SizeType, typename ContainerOut, typename ConversionFn>
		static ContainerOut& load_skeletal_keys(const KeyTypeIn* keys, SizeType key_count, ContainerOut& out, ConversionFn&& cnv_fn)
		{
			using KeyTypeOut = typename ContainerOut::value_type;

			for (SizeType key_index = 0; key_index < key_count; key_index++)
			{
				const auto& key_in = keys[key_index];

				out.push_back(KeyTypeOut { cnv_fn(key_in.mValue), static_cast<float>(key_in.mTime) });
				//out.emplace_back(cnv_fn(key_in.mValue), static_cast<float>(key_in.mTime));
			}

			return out;
		}

		template <typename ...Args>
		decltype(auto) load_skeletal_vectors(const math::Matrix& orientation, Args&&... args)
		{
			return load_skeletal_keys
			(
				std::forward<Args>(args)...,

				[&orientation](const aiVector3D& v)
				{
					auto vec = math::to_vector(v);

					return math::Vector3D { (orientation * math::Vector4D {vec, 1.0f }) };
				}
			);
		}

		template <typename ...Args>
		decltype(auto) load_skeletal_rotations(const math::Matrix& orientation, Args&&... args)
		{
			return load_skeletal_keys
			(
				std::forward<Args>(args)...,

				[&orientation](const aiQuaternion& q)
				{
					//auto quat = math::to_quat_flipped(q);
					auto quat = math::to_quat(q);

					return quat; // orientation * ...;
				}
			);
		}
	}

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
		const Skeleton* skeleton=nullptr, const _aiMatrix4x4* orientation=nullptr
	)
	{
		using VertexType = typename MeshData::Vertex;
		using MeshIndex  = ModelLoader::MeshIndex;
		
		bool _test = (util::to_string_view(node->mName) == "Geosphere");

		const auto vertex_count = (mesh->mNumVertices);

		MeshData data = { {}, std::make_shared<std::vector<MeshIndex>>() };

		data.vertices.reserve(vertex_count);

		// Retrieve vertex data:

		// TODO: Look into optimizing for decoupled attributes, if needed.
		// TODO: Review use of unsigned integer for indexing.
		for (unsigned i = 0; i < vertex_count; i++)
		{
			auto vertex = VertexType {};

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
				vertex.bone_indices =
				{
					graphics::VERTEX_BONE_CHANNEL_OPEN,
					graphics::VERTEX_BONE_CHANNEL_OPEN,
					graphics::VERTEX_BONE_CHANNEL_OPEN,
					graphics::VERTEX_BONE_CHANNEL_OPEN
				};

				vertex.bone_weights = {};
			}

			data.vertices.push_back(vertex);
		}

		if constexpr (MeshData::is_animated_vertex)
		{
			if (skeleton)
			{
				//int weight_channel = 0;

				for (unsigned int bone_index = {}; bone_index < mesh->mNumBones; bone_index++)
				{
					auto* bone_raw = mesh->mBones[bone_index];
					
					assert(bone_raw);

					auto* bone = skeleton->get_bone(util::to_string_view(bone_raw->mName));

					assert(bone);

					for (unsigned int weight_index = {}; weight_index < bone_raw->mNumWeights; weight_index++)
					{
						const auto& weight = bone_raw->mWeights[weight_index];
						
						auto& vertex = data.vertices[weight.mVertexId];

						if (auto weight_channel = get_next_weight_channel(vertex))
						{
							vertex.bone_indices[*weight_channel] = static_cast<graphics::BoneIndexType>(bone_index);
							vertex.bone_weights[*weight_channel] = weight.mWeight;
						}
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
		const std::shared_ptr<graphics::Context>& context,
		const std::shared_ptr<graphics::Shader>& default_shader,
		const std::shared_ptr<graphics::Shader>& default_animated_shader,

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
		const std::shared_ptr<graphics::Context>& context,
		const std::shared_ptr<graphics::Shader>& default_shader,
		const std::shared_ptr<graphics::Shader>& default_animated_shader,
		
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

			auto additional_bones = std::size_t {};

			for (const auto& bone_entry : skeleton.bones)
			{
				additional_bones += handle_missing_bone(*scene, skeleton, bone_entry.parent_name);
			}

			print("{} additional bone(s) generated.", additional_bones);

			process_animations(scene, skeleton, nullptr); // reinterpret_cast<_aiMatrix4x4*>(&inv_root_tform) <-- Not needed; these are relative movements.
		}

		return get_model_storage();
	}

	std::size_t ModelLoader::handle_missing_bone(const aiScene& scene, Skeleton& skeleton, BoneID bone_id, bool recursive)
	{
		if (!bone_id)
		{
			return 0;
		}

		auto bones_generated = std::size_t {};

		auto& bones = skeleton.bones;

		const auto bone_it = std::find_if
		(
			bones.begin(), bones.end(),
			[bone_id](const auto& bone) { return (bone.name == bone_id); }
		);

		if (bone_it == bones.end())
		{
			print("Generating missing bone: \"{}\" ({})", get_known_string_from_hash(bone_id), bone_id);

			// TODO: Optimize. (Temporary string created + hash lookup, rather than having storage of bone names upfront)
			auto bone_name = std::string { get_known_string_from_hash(bone_id) };

			if (!bone_name.empty())
			{
				if (auto* bone = process_bone(scene, skeleton, aiString(std::move(bone_name)), {}))
				{
					if (recursive)
					{
						bones_generated += handle_missing_bone(scene, skeleton, bone->parent_name, true);
					}

					bones_generated++;
				}
			}
		}

		return bones_generated;
	}

	bool ModelLoader::has_model_storage() const
	{
		return (model_storage.index() != NoStorage);
	}

	template <typename MeshData>
	static std::optional<graphics::Mesh> handle_mesh
	(
		ModelLoader& loader,
		
		const aiScene* scene,
		const aiNode* node,
		aiMesh* mesh,
		
		bool generate_graphical_mesh=true,
		CollisionGeometry::Container* collision_geometry=nullptr,
		const Skeleton* skeleton=nullptr,
		const _aiMatrix4x4* scene_orientation=nullptr
	)
	{
		auto mesh_data = process_mesh<MeshData>(loader.get_config(), scene, node, mesh, skeleton, scene_orientation);

		// TODO: Add additional step to look up separate collision mesh, rather than using raw mesh data from graphical element.
		if (collision_geometry)
		{
			collision_geometry->emplace_back(graphics::copy_simple_vertices(*mesh), mesh_data.indices); // TODO: Look into reducing unneeded copies.
		}

		if (generate_graphical_mesh)
		{
			return graphics::Mesh::Generate(loader.get_context(), mesh_data);
		}

		return std::nullopt;
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
			CollisionGeometry::Container collision_geometry;

			bool collision_enabled = cfg.load_collision;
			bool is_animated = cfg.is_animated.value_or(false);

			model.animated = is_animated;

			if (node->mName.length > 0)
			{
				model.name = util::to_string_view(node->mName);
			}

			if (cfg.load_mesh_content)
			{
				// Allocate mesh descriptors for each material:
				model.meshes.reserve(materials.size());

				for (const auto& material : materials)
					//for (std::size_t i = 0; i < materials.size(); i++) // model.meshes.size()
				{
					//model.meshes[i] = Model::MeshDescriptor(materials[i]);
					model.meshes.emplace_back(material);
				}
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

				std::optional<Mesh> mesh_out;

				if (is_animated)
				{
					mesh_out = handle_mesh<AnimMeshData>
					(
						*this, scene, node, mesh,
						cfg.load_mesh_content,
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
						cfg.load_mesh_content,
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

				if ((cfg.load_mesh_content) && (mesh_out))
				{
					auto material_index = mesh->mMaterialIndex;
					auto& mesh_descriptor = model.meshes[material_index];

					mesh_descriptor.meshes.push_back(std::move(*mesh_out));
				}
			}

			if (cfg.load_mesh_content)
			{
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

					// TODO: Review why this was added. (probably an optimization for overlapping texture binds)
					// Sort mesh descriptors based on number of textures.
					std::sort(model.meshes.begin(), model.meshes.end(),
					[](const auto& a, const auto& b)
					{
						return (a.material->textures.size() > b.material->textures.size());
					});
				}
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
					? std::optional<CollisionGeometry>{std::move(collision_geometry)}
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

	const Bone* ModelLoader::process_bone(const aiScene& scene, Skeleton& skeleton, const aiString& bone_name, const aiMatrix4x4& offset_matrix) // std::string_view bone_name
	{
		const auto* bone_node = scene.mRootNode->FindNode(bone_name);

		if (!bone_node)
		{
			print_warn("Unable to determine node for bone \"{}\"", util::to_string_view(bone_name));

			return nullptr;
		}

		const auto parent_bone_node = bone_node->mParent;

		const auto parent_bone_name = ((parent_bone_node) && (parent_bone_node->mName.length > 0))
			? util::to_string_view(parent_bone_node->mName)
			: std::string_view {}
		;

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

		const auto& tform_raw = local_transform;
		*/

		auto tform_raw = bone_node->mTransformation;

		if (parent_bone_name.empty())
		{
			auto inv_root_matrix = scene.mRootNode->mTransformation; inv_root_matrix.Inverse();
			
			tform_raw = inv_root_matrix * tform_raw;
		}

		return skeleton.add_bone
		(
			hash(util::to_string_view(bone_name)),
			
			math::to_matrix(tform_raw),
			math::to_matrix(offset_matrix),

			(parent_bone_name.empty())
				? BoneID {}
				: hash(parent_bone_name)
		);
	}

	std::size_t ModelLoader::process_bones(const aiScene& scene, const aiNode& node, const aiMesh& mesh, Skeleton& skeleton)
	{
		std::size_t bones_processed = {};

		for (unsigned int i = 0; i < mesh.mNumBones; i++)
		{
			const auto& bone_data = mesh.mBones[i];

			assert(bone_data);

			const auto existing_bone = skeleton.get_bone_by_name(util::to_string_view(bone_data->mName));

			if (existing_bone)
			{
				continue;
			}

			if (process_bone(scene, skeleton, bone_data->mName, bone_data->mOffsetMatrix))
			{
				bones_processed++;
			}
		}

		return bones_processed;
	}

	SkeletalFrameData& ModelLoader::process_animations(const aiScene* scene, Skeleton& skeleton, const _aiMatrix4x4* orientation)
	{
		using index_t = std::uint32_t;

		assert(scene);
		assert(scene->mRootNode);

		auto tform = math::Matrix {};

		if (orientation)
		{
			tform = math::to_matrix(*(reinterpret_cast<const aiMatrix4x4*>(orientation)));
		}
		else
		{
			tform = math::identity_matrix();
		}

		auto duration = 0.0f;

		const auto animation_count_raw = scene->mNumAnimations;

		for (index_t animation_index = {}; animation_index < animation_count_raw; animation_index++)
		{
			const auto* animation_entry = scene->mAnimations[animation_index];

			duration += static_cast<float>(animation_entry->mDuration);
		}

		auto& frame_data = animations.skeletal_sequences;

		frame_data.resize(skeleton.bones.size());

		for (index_t animation_index = {}; animation_index < animation_count_raw; animation_index++)
		{
			assert(scene->mAnimations[animation_index]);

			const auto& animation_entry = *(scene->mAnimations[animation_index]);

			//const auto animation_entry_duration = static_cast<float>(animation_entry.mDuration);

			for (index_t channel_index = {}; channel_index < animation_entry.mNumChannels; channel_index++)
			{
				assert(animation_entry.mChannels[channel_index]);

				const auto& channel = *(animation_entry.mChannels[channel_index]);

				if (const auto bone_name = util::to_string_view(channel.mNodeName); !bone_name.empty())
				{
					if (const auto bone_index = skeleton.get_bone_index(bone_name))
					{
						auto& sequence_out = frame_data[*bone_index];

						load_key_sequence(sequence_out, channel, tform);
					}
					else
					{
						print_warn("Unable to resolve bone: {}", bone_name);
					}
				}
			}
		}

		return animations;
	}

	SkeletalKeySequence& ModelLoader::load_key_sequence(SkeletalKeySequence& sequence_out, const aiNodeAnim& channel, const math::Matrix& tform)
	{
		impl::load_skeletal_vectors(tform, channel.mPositionKeys, channel.mNumPositionKeys, sequence_out.positions);
		impl::load_skeletal_rotations(tform, channel.mRotationKeys, channel.mNumRotationKeys, sequence_out.rotations);
		impl::load_skeletal_vectors(tform, channel.mScalingKeys, channel.mNumScalingKeys, sequence_out.scales);

		return sequence_out;
	}

	std::shared_ptr<ModelLoader::Material> ModelLoader::process_material
	(
		const aiScene* scene,
		const aiMaterial* native_material,
		bool load_values,
		bool load_textures
	)
	{
		const bool is_animated = cfg.is_animated.value_or(false);

		auto material = std::make_shared<Material>((is_animated) ? default_animated_shader : default_shader);

		//auto texture_types = 0;
		
		if (load_textures)
		{
			// Load and store each texture according to its type:
			graphics::enumerate_texture_classes([&](graphics::TextureClass texture_type)
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

	std::shared_ptr<ModelLoader::Texture> ModelLoader::process_texture(const filesystem::path& texture_path)
	{
		bool file_exists = filesystem::exists(texture_path);

		std::shared_ptr<Texture> t;

		if (file_exists)
		{
			// TODO: Review management of 'Texture' resources.
			t = std::make_shared<Texture>(context, texture_path.string());
		}
		else
		{
			auto alternate_path = (*cfg.root_path / (texture_path.filename()));

			alternate_path.make_preferred();

			t = std::make_shared<Texture>(context, alternate_path.string());
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