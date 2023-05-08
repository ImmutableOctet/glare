#pragma once

#include "entity.hpp"

#include <engine/types.hpp>
#include <util/log.hpp>
#include <util/json.hpp>

#include <filesystem>
#include <string>
#include <tuple>
#include <utility>

//#define _ENFORCE_MATCHING_ALLOCATORS 0

#include <unordered_map>
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

	// A Stage, sometimes referred to as a Map is an abstract concept for
	// a portion of the scene-graph that's been loaded from an external source.
	// e.g. from disk, network, etc.
	class Stage
	{
		public:
			using ObjectIndex = std::uint16_t; // std::string; // PlayerIndex;

			using ObjectMap       = std::unordered_map<ObjectIndex, Entity>;
			using PlayerObjectMap = std::unordered_map<PlayerIndex, Entity>;
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
			static void resolve_parent(World& world, Entity entity, Entity stage, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data);

			static Entity resolve_object_reference(const std::string& query, World& world, const PlayerObjectMap& player_objects, const ObjectMap& objects); // std::tuple<std::string, std::string>

			static math::TransformVectors get_transform_data(const util::json& cfg);

			static void apply_transform(World& world, Entity entity, const util::json& cfg);
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
			// e.g. the `world` object we're loading into, the `stage` entity acting as parent to the scene loaded, etc.
			class Loader
			{
				protected:
					static Entity make_stage_pivot(World& world, Entity parent=null);
				public:
					// Loader configuration; e.g. should we load objects, players, etc.
					struct Config
					{
						bool geometry = true;
						bool objects = true;
						bool players = true;

						bool apply_transform = true;
					};

					// Some members of this type are reference/const-reference for
					// optimization purposes, but may be changed to value-types later.

					// The target `World` object we're loading the scene into.
					World& world;

					// The root path of this stage. (i.e. where stage-specific resources can be loaded from -- geometry, etc.)
					const filesystem::path& root_path; // filesystem::path

					// Hierarchical input data in the form of a 'dictionary'. (Currently json-based)
					const util::json& data; // util::json

					Entity stage = null;

					// Optional non-owning pointer to system-manager.
					SystemManagerInterface* system_manager = nullptr;
				protected:
					struct
					{
						struct
						{
							PlayerObjectMap player_objects;
							PlayerIndex     player_idx_counter = 1;
						} players;

						struct
						{
							ObjectMap   objects;
							ObjectIndex obj_idx_counter = 1; // std::uint16_t
						} objects;
					} indices;

					bool ensure_stage(Entity parent=null);
				public:
					Loader
					(
						World& world,
						const filesystem::path& root_path,
						const util::json& data,
						Entity stage=null,
						SystemManagerInterface* system_manager=nullptr
					);

					Entity load(Entity stage, const Config& cfg={}, Entity parent=null, bool load_title=false);
					Entity load(const Config& cfg={}, Entity parent=null);

					void load_properties(bool load_title=false, const std::string& default_title="Unknown Stage");
					void load_geometry();
					void load_players();
					void load_objects();

					ResourceManager& get_resource_manager() const;

					inline operator Entity() const
					{
						//assert(stage != null);

						return stage;
					}
			};

			static Entity Load
			(
				World& world,
				const filesystem::path& root_path,
				const util::json& data,
				Entity parent=null,
				SystemManagerInterface* opt_system_manager=nullptr,
				const Stage::Loader::Config& cfg={}
			);
	};
}