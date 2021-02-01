#pragma once

//#include <vector>
#include <string>

#include <types.hpp>

#include "collision.hpp"

// Forward declarations:

// Bullet:
class btCollisionShape;
class btCapsuleShape;
class btBoxShape;

namespace graphics
{
	class Context;
	class Shader;
	class Texture;
	class Model;
}

namespace engine
{
	class ResourceManager
	{
		public:
			using CollisionRaw = btCollisionShape;
			using CollisionData = ref<CollisionRaw>;

			using ModelData = ref<graphics::Model>;
			using TextureData = ref<graphics::Texture>;

			ResourceManager(pass_ref<graphics::Context> context, pass_ref<graphics::Shader> default_shader);
			~ResourceManager();

			inline pass_ref<graphics::Context> get_context() const { return context; }
			inline pass_ref<graphics::Shader> get_default_shader() const { return default_shader; }

			ModelData load_model(const std::string& path) const;

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
			mutable ref<graphics::Context> context;
			mutable ref<graphics::Shader> default_shader;

			//std::unordered_map<std::string, CollisionData> collision_data;
			//std::unordered_map<std::string, ModelData> model_data;
			//std::unordered_map<std::string, TextureData> texture_data;
	};
}