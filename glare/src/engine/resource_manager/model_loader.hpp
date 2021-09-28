#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <variant>
#include <functional>

#include <util/variant.hpp>
#include <types.hpp>
#include <graphics/types.hpp>
#include <graphics/model.hpp>
#include <graphics/mesh.hpp>
#include <graphics/material.hpp>

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

//class aiMatrix4x4;

using _aiMatrix4x4 = void*;

// Bullet:
class btTriangleIndexVertexArray;

namespace engine
{
	class ModelLoader
	{
		public:

			using Context     = graphics::Context;
			using Model       = graphics::Model;
			using Mesh        = graphics::Mesh;
			using Texture     = graphics::Texture;
			using Shader      = graphics::Shader;
			using Material    = graphics::Material;

			using MeshIndex   = graphics::MeshIndex;
			using VertexType  = Model::VertexType;
			using MeshData    = graphics::MeshData<VertexType>;

			using Materials   = std::vector<ref<Material>>;
			
			using NativeFlags = unsigned int;

			struct Config
			{
				std::optional<filesystem::path> root_path = std::nullopt;

				bool maintain_storage = true;

				bool load_textures  = true;
				bool load_bones     = true;
				bool load_animation = true;
				bool load_collision = true;

				std::optional<bool> is_animated = std::nullopt;

				struct
				{
					bool is_right_handed = false;
					bool flip_normals = false;
					bool needs_reorientation = false;

					std::optional<graphics::VertexWinding> vert_direction = std::nullopt;
				} orientation;
			};

			struct ModelData
			{
				Model model;
				std::optional<Model::CollisionGeometry> collision = std::nullopt;

				inline explicit operator bool() const { return static_cast<bool>(model); }

				inline ModelData(Model&& model, std::optional<Model::CollisionGeometry>&& collision)
					: model(std::move(model)), collision(std::move(collision)) {}

				ModelData(ModelData&&) = default;
				ModelData(const ModelData&) = delete;

				ModelData& operator=(ModelData&&) = default;
			};
			
			using ModelVector = std::vector<ModelData>;
			using ModelStorage = std::variant<std::monostate, ModelData, ModelVector>;

			template <typename T>
			static constexpr std::size_t StorageType()
			{
				return util::variant_index<ModelStorage, T>();
			}

			static constexpr auto NoStorage = StorageType<std::monostate>();

			inline std::size_t get_storage_type() const
			{
				return model_storage.index();
			}
		private:
			Config cfg;

			ModelStorage model_storage;
			Materials materials;
		public:
			std::function<void(ModelLoader&, Texture&)> on_texture;
			std::function<void(ModelLoader&, Material&)> on_material;
			std::function<void(ModelLoader&, ModelData&)> on_model;
			//std::function<void(ModelLoader&, pass_ref<ModelData>, const Bone&)> on_bone;

			ModelLoader
			(
				pass_ref<graphics::Context> context,
				pass_ref<graphics::Shader> default_shader,
				pass_ref<graphics::Shader> default_animated_shader,

				const Config& cfg = {}
			);

			ModelLoader
			(
				pass_ref<graphics::Context> context,
				pass_ref<graphics::Shader> default_shader,
				pass_ref<graphics::Shader> default_animated_shader,
				const filesystem::path& filepath,
				std::optional<NativeFlags> native_flags=std::nullopt,
				const Config& cfg = {}
			);

			ModelStorage& load
			(
				const filesystem::path& filepath,
				std::optional<NativeFlags> native_flags=std::nullopt,
				bool update_root_path=false
			);

			//inline const ModelStorage& get_model_storage() const { return model_storage; }
			inline ModelStorage& get_model_storage() { return model_storage; }
			bool has_model_storage() const;

			inline Materials& get_materials() { return materials; }
			inline bool has_materials() const { return !materials.empty(); }
		protected:
			void store_model_data(ModelData&& model);

			ref<Material> process_material(const aiScene* scene, const aiMaterial* native_material, bool load_values=true, bool load_textures=true);
			ref<Texture> process_texture(const filesystem::path& texture_path);
			void process_node(const aiScene* scene, const aiNode* node, const _aiMatrix4x4* scene_orientation=nullptr);
			MeshData process_mesh(const aiScene* scene, const aiNode* node, const aiMesh* mesh, const _aiMatrix4x4* _scene_orientation=nullptr);

			ref<Context> context;

			ref<Shader> default_shader;
			ref<Shader> default_animated_shader;
	};
}