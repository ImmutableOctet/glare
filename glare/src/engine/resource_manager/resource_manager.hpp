#pragma once

#include "types.hpp"

//#include <engine/world/physics/collision.hpp>

#include "collision_data.hpp"
#include "model_data.hpp"

//#include "loaders/loaders.hpp"

#include <engine/meta/meta_parsing_context.hpp>
#include <engine/meta/meta_type_resolution_context.hpp>
//#include <engine/meta/meta_variable_context.hpp>

//#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <unordered_map>
#include <map>

//#include <utility>
#include <optional>

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
	class EntityDescriptor;

	struct RenderScene;
	struct CollisionShapeDescription;

	struct EntityFactoryData;
	struct EntityFactoryContext;
	struct EntityConstructionContext;

	using EntityFactoryKey = std::string; // std::string_view; // EntityFactory::FactoryKey:

	// TODO: Revisit weak vs. strong references for caching.
	// Theoretically we could use weak references but return strong references upon initial request.

	struct Resources
	{
		// Aliases:

		// Reference to a 'Model' object; used internally for path lookups, etc.
		using Models = std::vector<ModelRef>; // util::small_vector<ModelRef, ...>;

		using ShaderRef = std::shared_ptr<graphics::Shader>;
		using WeakShaderRef = std::weak_ptr<graphics::Shader>;

		using TextureData = std::shared_ptr<graphics::Texture>;

		using FactoryKey = EntityFactoryKey;

		// Resource collections:
		mutable std::unordered_map<std::string, ModelData> loaded_models; // Models // std::map // ModelRef
		mutable std::unordered_map<std::string, ShaderRef> loaded_shaders; // std::map

		mutable std::map<const WeakModelRef, CollisionData, std::owner_less<>> collision_data; //std::unordered_map<WeakModelRef, CollisionData, std::hash<WeakModelRef>, std::owner_less<>> collision_data;

		mutable std::map<const WeakModelRef, std::shared_ptr<AnimationData>, std::owner_less<>> animation_data;

		//std::unordered_map<std::string, TextureData> texture_data;

		// Maps file paths to factory objects able to generate entities of a given specification.
		mutable std::unordered_map<FactoryKey, std::weak_ptr<EntityFactoryData>> entity_factories; // , std::owner_less<> // std::map
	};

	class ResourceManager : protected Resources
	{
		public:
			friend class World;
			friend class WorldRenderer;
			friend struct RenderScene;

			// TODO: Look into removing this. (Needed for construction of `RenderScene` object in `Game`)
			friend class game::Game;

			ResourceManager(const std::shared_ptr<graphics::Context>& context, const std::shared_ptr<graphics::Shader>& default_shader={}, const std::shared_ptr<graphics::Shader>& default_animated_shader={});
			~ResourceManager();

			inline const std::shared_ptr<graphics::Context>& get_context() const { return context; }

			inline const std::shared_ptr<graphics::Shader>& get_default_shader() const { return default_shader; }
			inline const std::shared_ptr<graphics::Shader>& get_default_animated_shader() const { return default_animated_shader; }

			inline void set_default_shader(const std::shared_ptr<graphics::Shader>& shader)
			{
				default_shader = shader;

				loaded_shaders["DEFAULT"] = shader;
			}

			inline void set_default_animated_shader(const std::shared_ptr<graphics::Shader>& shader)
			{
				default_animated_shader = shader;

				loaded_shaders["DEFAULT_ANIMATED"] = shader;
			}

			const ModelData& load_model
			(
				const std::string& path,
				bool load_collision=false,
				
				const std::shared_ptr<graphics::Shader>& shader={},

				bool optimize_collision=true,
				bool force_reload=false,
				bool cache_result=true
			) const;

			ShaderRef get_shader(const std::string& vertex_path, const std::string& fragment_path, bool force_reload=false, bool cache_result=true) const; // const graphics::ShaderSource& shader_source
			
			// Optionally returns a pointer to a 'CollisionData' object for the 'model' specified.
			//const CollisionData* get_collision(WeakModelRef model);

			const CollisionData* get_collision(const WeakModelRef model) const;

			std::shared_ptr<const AnimationData> get_animation_data(const WeakModelRef model) const;
			const AnimationData* peek_animation_data(const WeakModelRef model) const;

			std::shared_ptr<const EntityFactoryData> get_existing_factory(const std::string& path) const; // std::string_view
			const EntityDescriptor* get_existing_descriptor(const std::string& path) const;
			std::shared_ptr<const EntityFactoryData> get_factory(const EntityFactoryContext& context) const;

			Entity generate_entity(const EntityFactoryContext& factory_context, const EntityConstructionContext& entity_context, bool handle_children=true) const;

			MetaParsingContext set_type_resolution_context(MetaTypeResolutionContext&& context);
			//MetaParsingContext set_variable_context(MetaVariableContext&& context);

			MetaParsingContext get_parsing_context() const;
			MetaParsingContext get_existing_parsing_context() const;

			// Links events from `world` to this resource manager instance.
			void subscribe(World& world);
		protected:
			//inline static std::string resolve_path(const std::string& path) { return path; }

			static std::string resolve_path(const std::string& path);

			mutable std::shared_ptr<graphics::Context> context;

			mutable std::shared_ptr<graphics::Shader> default_shader;
			mutable std::shared_ptr<graphics::Shader> default_animated_shader;

			mutable std::optional<MetaTypeResolutionContext> type_resolution_context = std::nullopt;
			//mutable std::optional<MetaVariableContext> variable_context = std::nullopt;
	};
}