#pragma once

#include "entity.hpp"
#include "scene_loader_config.hpp"

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

	class SceneLoader
	{
		public:
			using Config = SceneLoaderConfig;

			static math::TransformVectors get_transform_data(const util::json& data);

			SceneLoader
			(
				World& world,
				const filesystem::path& root_path,
				Entity scene=null,
				const Config& cfg={},
				SystemManagerInterface* system_manager=nullptr
			);

			Entity load(const util::json& data, Entity parent);
			Entity load(const util::json& data);

			void load_title(const util::json& data);
			void load_properties(const util::json& data);
			void load_geometry(const util::json& data);
			void load_players(const util::json& data);
			void load_objects(const util::json& data);

			void apply_transform(Entity entity, const util::json& data);

			Entity entity_reference(std::string_view query); // const

			ResourceManager& get_resource_manager() const;

			inline operator Entity() const
			{
				//assert(this->scene != null);

				return this->scene;
			}

		protected:
			bool apply_state(Entity entity, const util::json& data);
			std::optional<graphics::ColorRGBA> apply_color(Entity entity, const util::json& data);

			// Some members of this type are reference/const-reference for
			// optimization purposes, but may be changed to value-types later.

			// The target `World` object we're loading the scene into.
			World& world;

			// The root path of this scene. (i.e. where scene-specific resources can be loaded from -- geometry, etc.)
			const filesystem::path& root_path; // filesystem::path

			Entity scene = null;

			Config cfg;

			// Optional non-owning pointer to system-manager.
			SystemManagerInterface* system_manager = nullptr;

			PlayerIndex player_idx_counter = PRIMARY_LOCAL_PLAYER;
			
			bool ensure_scene(Entity parent=null);

			Entity resolve_parent(Entity entity, const util::json& data, bool fallback_to_scene=true);
	};
}