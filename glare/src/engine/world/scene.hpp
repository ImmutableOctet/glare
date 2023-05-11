#pragma once

#include "entity.hpp"

#include <engine/types.hpp>
#include <util/log.hpp>
#include <util/json.hpp>

#include <filesystem>
#include <string>
#include <tuple>
#include <utility>
#include <string_view>

//#define _ENFORCE_MATCHING_ALLOCATORS 0

#include <functional>

namespace filesystem = std::filesystem;

namespace graphics
{
	class Context;
}

namespace engine
{
	class Service;
	class World;
	class ResourceManager;
	class MetaParsingContext;
	class SystemManagerInterface;

	// A `Scene`, sometimes referred to as a 'Map' or 'Stage' is an abstract concept for
	// a portion of the scene-graph that's been loaded from an external source.
	// e.g. from disk, network, etc.
	class Scene
	{
		private:
			template <typename ...IgnoredKeys>
			static void process_data_entries
			(
				Registry& registry, Entity entity, const util::json& object_data,
				const MetaParsingContext& parsing_context,
				Service* opt_service, SystemManagerInterface* opt_system_manager,
				IgnoredKeys&&... ignored_keys
			);

		protected:
			static math::TransformVectors get_transform_data(const util::json& cfg);

			static void apply_transform(World& world, Entity entity, const util::json& cfg);
			static bool apply_state(World& world, Entity entity, const util::json& cfg);
			static std::optional<graphics::ColorRGBA> apply_color(World& world, Entity entity, const util::json& cfg);

			template <typename JsonArray, typename Pred>
			static inline JsonArray ForEach(JsonArray&& arr, Pred&& pred)
			{
				for (const auto& item : arr.items())
				{
					const auto& e = item.value();

					pred(e);
				}

				return arr;
			}
		public:
			// Used internally to keep track of objects involved in the loading process.
			// e.g. the `world` object we're loading into, the `scene` entity acting as parent to the scene loaded, etc.
			class Loader
			{
				public:
					// Loader configuration; e.g. should we load objects, players, etc.
					struct Config
					{
						bool geometry = true;
						bool objects = true;
						bool players = true;

						bool apply_transform = true;
					};

				protected:
					static Entity make_scene_pivot(World& world, Entity parent=null);

					// Some members of this type are reference/const-reference for
					// optimization purposes, but may be changed to value-types later.

					// The target `World` object we're loading the scene into.
					World& world;

					// The root path of this scene. (i.e. where scene-specific resources can be loaded from -- geometry, etc.)
					const filesystem::path& root_path; // filesystem::path

					// Hierarchical input data in the form of a 'dictionary'. (Currently json-based)
					const util::json& data; // util::json

					Entity scene = null;

					// Optional non-owning pointer to system-manager.
					SystemManagerInterface* system_manager = nullptr;

					PlayerIndex player_idx_counter = PRIMARY_LOCAL_PLAYER;
					
					bool ensure_scene(Entity parent=null);

					Entity resolve_parent(Entity entity, const util::json& data, bool fallback_to_scene=true);

				public:
					Loader
					(
						World& world,
						const filesystem::path& root_path,
						const util::json& data,
						Entity scene=null,
						SystemManagerInterface* system_manager=nullptr
					);

					Entity load(Entity scene, const Config& cfg={}, Entity parent=null, bool load_title=false);
					Entity load(const Config& cfg={}, Entity parent=null);

					void load_properties(bool load_title=false, const std::string& default_title="Unknown Scene");
					void load_geometry();
					void load_players();
					void load_objects();

					Entity entity_reference(std::string_view query); // const

					ResourceManager& get_resource_manager() const;

					inline operator Entity() const
					{
						//assert(scene != null);

						return scene;
					}
			};

			static Entity Load
			(
				World& world,
				const filesystem::path& root_path,
				const util::json& data,
				Entity parent=null,
				SystemManagerInterface* opt_system_manager=nullptr,
				const Scene::Loader::Config& cfg={}
			);
	};
}