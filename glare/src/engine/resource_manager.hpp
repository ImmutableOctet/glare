#pragma once

//#include <vector>
#include <memory>
#include <string>
#include <tuple>

#include <unordered_map>
#include <map>

//#include <utility>
#include <optional>

#include <types.hpp>
#include <graphics/model.hpp>

#include "collision.hpp"

// Forward declarations:

// Bullet:
//class btCollisionShape;
class btCapsuleShape;
class btBoxShape;

namespace graphics
{
	class Context;
	class Shader;
	class Texture;
	//class Model;
}

namespace engine
{
	class World;

	class ResourceManager
	{
		public:
			friend class World;

			using CollisionRaw = CollisionComponent::RawShape; // btCollisionShape;
			using CollisionShape = CollisionComponent::Shape; // ref<CollisionRaw>;
			using CollisionGeometry = graphics::Model::CollisionGeometry;

			//using CollisionData = CollisionShape;

			struct CollisionData
			{
				using Shape = CollisionShape;
				using Raw = CollisionRaw;

				using Geometry = CollisionGeometry;

				CollisionData() = default;

				CollisionData(CollisionData&&) = default;
				CollisionData(const CollisionData&) = default;

				CollisionData(const Shape& collision_shape);
				CollisionData(Geometry&& geometry_storage, bool optimize=true);

				CollisionData& operator=(CollisionData&&) = default;

				Shape collision_shape;
				std::optional<Geometry> geometry_storage = std::nullopt;

				inline bool has_shape() const { return collision_shape.operator bool(); }
				inline bool has_geometry() const { return geometry_storage.has_value(); }

				inline explicit operator bool() const { return has_shape(); }
				inline bool operator==(const CollisionData& data) const { return (this->collision_shape == data.collision_shape); }
			};

			// Reference to a 'Model' object; used internally for path lookups, etc.
			using ModelRef = ref<graphics::Model>;
			using WeakModelRef = weak_ref<graphics::Model>; // const graphics::Model*

			using ShaderRef = ref<graphics::Shader>;
			using WeakShaderRef = weak_ref<graphics::Shader>;

			// Output from load/creation function for models.
			using ModelData = std::tuple<ModelRef, const CollisionData*>; // ModelRef // std::optional<...>

			using TextureData = ref<graphics::Texture>;

			ResourceManager(pass_ref<graphics::Context> context, pass_ref<graphics::Shader> default_shader);
			~ResourceManager();

			inline pass_ref<graphics::Context> get_context() const { return context; }

			inline pass_ref<graphics::Shader> get_default_shader() const { return default_shader; }

			inline void set_default_shader(pass_ref<graphics::Shader> shader)
			{
				default_shader = shader;

				loaded_shaders["DEFAULT"] = shader;
			}

			ModelData load_model(const std::string& path, bool load_collision=false, pass_ref<graphics::Shader> shader={}, bool optimize_collision=true, bool force_reload=false, bool cache_result=true) const;

			ShaderRef get_shader(const std::string& vertex_path, const std::string& fragment_path, bool force_reload=false, bool cache_result=true) const; // const graphics::ShaderSource& shader_source

			// Optionally returns a pointer to a 'CollisionData' object for the 'model' specified.
			//const CollisionData* get_collision(WeakModelRef model);

			// TODO: Implement caching for primitive collision shapes.
			// NOTE: When using this template, you should already have the desired collision shapes included from Bullet.
			/*
			template <typename... Args>
			inline constexpr CollisionData get_collision_shape(CollisionShape shape, Args&&... args) const
			{
				switch (shape)
				{
					case CollisionShape::Capsule:
						return std::static_pointer_cast<CollisionRaw>(std::make_shared<btCapsuleShape>(std::forward<Args>(args)...));
					case CollisionShape::Box:
						return std::static_pointer_cast<CollisionRaw>(std::make_shared<btBoxShape>(std::forward<Args>(args)...));
				}

				return nullptr;
			}
			*/

			CollisionData get_capsule_collision(float radius, float height); // ref<btCapsuleShape>
		protected:
			//inline static std::string resolve_path(const std::string& path) { return path; }

			static std::string resolve_path(const std::string& path);
			static CollisionShape build_mesh_shape(const CollisionGeometry& geometry_storage, bool optimize=true);

			mutable ref<graphics::Context> context;
			mutable ref<graphics::Shader> default_shader;

			mutable std::unordered_map<std::string, ModelRef> loaded_models; // ModelData // std::map
			mutable std::unordered_map<std::string, ShaderRef> loaded_shaders; // std::map

			mutable std::map<WeakModelRef, CollisionData, std::owner_less<>> collision_data; //std::unordered_map<WeakModelRef, CollisionData, std::hash<WeakModelRef>, std::owner_less<>> collision_data;	

			//std::unordered_map<std::string, TextureData> texture_data;
	};
}