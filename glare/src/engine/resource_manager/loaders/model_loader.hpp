#pragma once

#include <engine/resource_manager/animation_data.hpp>
#include <engine/world/physics/types.hpp>

#include <graphics/types.hpp>
#include <graphics/model.hpp>
#include <graphics/mesh.hpp>
#include <graphics/material.hpp>
#include <graphics/collision_geometry.hpp>

#include <util/variant.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <filesystem>
#include <variant>
#include <functional>

#define AI_CONFIG_PP_PTV_NORMALIZE   "PP_PTV_NORMALIZE"

#include <assimp/Importer.hpp>

/*
namespace Assimp
{
	class Importer;
}
*/

namespace filesystem = std::filesystem;

namespace graphics
{
	class Context;
	class Texture;
	class Shader;
	class Material;
}

// Assimp:
struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;

struct aiNodeAnim;
struct aiVectorKey;
struct aiQuatKey;

//class aiMatrix4x4;
using _aiMatrix4x4 = void*;

// Bullet:
class btTriangleIndexVertexArray;

namespace engine
{
	struct Skeleton;
	struct Bone;

	struct SkeletalFrameData;
	struct SkeletalKeySequence;

	class ModelLoader
	{
		public:
			using Context       = graphics::Context;
			using Model         = graphics::Model;
			using Mesh          = graphics::Mesh;
			using Texture       = graphics::Texture;
			using Shader        = graphics::Shader;
			using Material      = graphics::Material;

			using MeshIndex     = graphics::MeshIndex;
			using VertexType    = Model::VertexType;
			using AVertexType   = Model::AVertexType;
			using MeshData      = graphics::MeshData<VertexType>;
			using AnimMeshData  = graphics::MeshData<AVertexType>;

			using Materials     = std::vector<std::shared_ptr<Material>>;
			
			using NativeFlags = unsigned int;

			struct Config
			{
				std::optional<filesystem::path> root_path = std::nullopt;

				bool maintain_storage = true;

				bool load_textures     = true;
				bool load_bones        = true;
				bool load_collision    = true;
				bool load_mesh_content = true;

				std::optional<bool> is_animated = std::nullopt;

				struct
				{
					bool flip_normals = false;

					graphics::VertexWinding vert_direction = graphics::VertexWinding::CounterClockwise;
				} orientation;
			};

			// A manifold containing objects loaded from a model resource.
			struct ModelData
			{
				Model model;

				math::Matrix transform;

				std::optional<CollisionGeometry> collision = std::nullopt;

				const Skeleton* skeleton = nullptr;
				const SkeletalFrameData* animation_data = nullptr;

				inline explicit operator bool() const { return static_cast<bool>(model); }

				inline ModelData
				(
					Model&& model,
					const math::Matrix& transform,
					std::optional<CollisionGeometry>&& collision=std::nullopt,

					const Skeleton* skeleton=nullptr,
					const SkeletalFrameData* animation_data=nullptr
				) :
					model(std::move(model)),
					collision(std::move(collision)),
					skeleton(skeleton),
					animation_data(animation_data)
				{}

				ModelData(ModelData&&) noexcept = default;
				ModelData(const ModelData&) = delete;

				ModelData& operator=(ModelData&&) noexcept = default;
			};
			
			using ModelVector = std::vector<ModelData>;
			using ModelStorage = std::variant<std::monostate, ModelData, ModelVector>;

			template <typename T>
			static constexpr std::size_t StorageType()
			{
				return util::variant_index<ModelStorage, T>();
			}

			inline static constexpr auto NoStorage = StorageType<std::monostate>();

		private:
			std::shared_ptr<Context> context;

			std::shared_ptr<Shader> default_shader;
			std::shared_ptr<Shader> default_animated_shader;

			Config cfg;

			ModelStorage model_storage;
			Materials materials;

			Skeleton skeleton;
			SkeletalFrameData animations;

		public:
			// TODO: Look into refactoring this to use templated callbacks or events.
			std::function<void(ModelLoader&, Texture&)>   on_texture;
			std::function<void(ModelLoader&, Material&)>  on_material;
			std::function<void(ModelLoader&, ModelData&)> on_model;
			//std::function<void(ModelLoader&, const std::shared_ptr<ModelData>&, const Bone&)> on_bone;

			ModelLoader
			(
				const std::shared_ptr<graphics::Context>& context,

				const std::shared_ptr<graphics::Shader>& default_shader,
				const std::shared_ptr<graphics::Shader>& default_animated_shader,

				const Config& cfg={}
			);

			ModelLoader
			(
				const std::shared_ptr<graphics::Context>& context,
				
				const std::shared_ptr<graphics::Shader>& default_shader,
				const std::shared_ptr<graphics::Shader>& default_animated_shader,
				
				const filesystem::path& filepath,
				
				std::optional<NativeFlags> native_flags=std::nullopt,
				const Config& cfg={}
			);

			ModelStorage& load
			(
				const filesystem::path& filepath,
				std::optional<NativeFlags> native_flags=std::nullopt,
				bool update_root_path=false
			);

			inline std::size_t get_storage_type() const
			{
				return model_storage.index();
			}

			inline const Config& get_config() const
			{
				return cfg;
			}

			inline const std::shared_ptr<Context>& get_context() const
			{
				return context;
			}

			//inline const ModelStorage& get_model_storage() const { return model_storage; }
			inline ModelStorage& get_model_storage() { return model_storage; }
			bool has_model_storage() const;

			inline Materials& get_materials() { return materials; }
			inline bool has_materials() const { return !materials.empty(); }

			inline Skeleton& get_skeleton() { return skeleton; }
			inline bool has_skeleton() { return skeleton.exists(); }

			inline SkeletalFrameData& get_animations() { return animations; }
			inline const SkeletalFrameData& get_animations() const { return animations; }

			inline bool has_animations() const
			{
				return static_cast<bool>(animations);
			}

		protected:
			void store_model_data(ModelData&& model);

			std::shared_ptr<Material> process_material(const aiScene* scene, const aiMaterial* native_material, bool load_values=true, bool load_textures=true);
			std::shared_ptr<Texture> process_texture(const filesystem::path& texture_path);
			void process_node(const aiScene* scene, const aiNode* node, const _aiMatrix4x4* orientation=nullptr, const _aiMatrix4x4* global_orientation=nullptr);
			//MeshData process_mesh(const aiScene* scene, const aiNode* node, const aiMesh* mesh, const Skeleton* skeleton=nullptr, const _aiMatrix4x4* orientation=nullptr);
			
			const Bone* process_bone(const aiScene& scene, Skeleton& skeleton, const aiString& bone_name, const aiMatrix4x4& offset_matrix); // std::string_view
			
			std::size_t process_bones(const aiScene& scene, const aiNode& node, const aiMesh& mesh, Skeleton& skeleton);
			std::size_t handle_missing_bone(const aiScene& scene, Skeleton& skeleton, BoneID bone_id, bool recursive=true);

			SkeletalFrameData& process_animations(const aiScene* scene, Skeleton& skeleton, const _aiMatrix4x4* orientation=nullptr);
		
			SkeletalKeySequence& load_key_sequence
			(
				SkeletalKeySequence& sequence_out,
				const aiNodeAnim& channel,
				const math::Matrix& tform={}
			);
	};
}