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

#include <engine/collision.hpp>

#include "animation_data.hpp"
#include "collision_data.hpp"
#include "model_data.hpp"

#include "loaders/loaders.hpp"

namespace game
{
	class Game;
}

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
	class WorldRenderer;

	struct RenderScene;

	// TODO: Revisit weak vs. strong references for caching.
	// Theoretically we could use weak references but return strong references upon initial request.

	struct Resources
	{
		// Aliases:
		using AnimationData = engine::AnimationData;

		// Reference to a 'Model' object; used internally for path lookups, etc.
		using Models = engine::Models;

		using ShaderRef = ref<graphics::Shader>;
		using WeakShaderRef = weak_ref<graphics::Shader>;

		using TextureData = ref<graphics::Texture>;

		// Output from load/creation function for models.
		//using ModelData = Models; // std::tuple<Models, const CollisionData*>; // ModelRef // std::optional<...> // ref<ModelLoader::ModelStorage>;
		using ModelData = engine::ModelData;

		// Resource collections:
		mutable std::unordered_map<std::string, ModelData> loaded_models; // Models // std::map // ModelRef
		mutable std::unordered_map<std::string, ShaderRef> loaded_shaders; // std::map

		mutable std::map<const WeakModelRef, CollisionData, std::owner_less<>> collision_data; //std::unordered_map<WeakModelRef, CollisionData, std::hash<WeakModelRef>, std::owner_less<>> collision_data;

		mutable std::map<const WeakModelRef, ref<AnimationData>, std::owner_less<>> animation_data;

		//std::unordered_map<std::string, TextureData> texture_data;
	};

	class ResourceManager : protected Resources
	{
		public:
			friend class World;
			friend class WorldRenderer;
			friend struct RenderScene;

			// TODO: Look into removing this. (Needed for construction of `RenderScene` object in `Game`)
			friend class game::Game;

			ResourceManager(pass_ref<graphics::Context> context, pass_ref<graphics::Shader> default_shader={}, pass_ref<graphics::Shader> default_animated_shader={});
			~ResourceManager();

			inline pass_ref<graphics::Context> get_context() const { return context; }

			inline pass_ref<graphics::Shader> get_default_shader() const { return default_shader; }
			inline pass_ref<graphics::Shader> get_default_animated_shader() const { return default_animated_shader; }

			inline void set_default_shader(pass_ref<graphics::Shader> shader)
			{
				default_shader = shader;

				loaded_shaders["DEFAULT"] = shader;
			}

			inline void set_default_animated_shader(pass_ref<graphics::Shader> shader)
			{
				default_animated_shader = shader;

				loaded_shaders["DEFAULT_ANIMATED"] = shader;
			}

			const ModelData& load_model
			(
				const std::string& path,
				bool load_collision=false,
				
				pass_ref<graphics::Shader> shader={},

				bool optimize_collision=true,
				bool force_reload=false,
				bool cache_result=true
			) const;

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

			CollisionData generate_capsule_collision(float radius, float height); // ref<btCapsuleShape>
			const CollisionData* get_collision(const WeakModelRef model) const;
			const ref<AnimationData> get_animation_data(const WeakModelRef model) const;

			// Links events from `world` to this resource manager instance.
			void subscribe(World& world);
		protected:
			//inline static std::string resolve_path(const std::string& path) { return path; }

			static std::string resolve_path(const std::string& path);

			mutable ref<graphics::Context> context;

			mutable ref<graphics::Shader> default_shader;
			mutable ref<graphics::Shader> default_animated_shader;
	};
}