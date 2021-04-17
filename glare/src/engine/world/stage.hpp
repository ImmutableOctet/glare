#pragma once

#include "entity.hpp"
#include "player.hpp"

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

namespace engine
{
	class World;

	class Stage
	{
		public:
			using ObjectIndex = std::uint16_t; // std::string; // PlayerIndex;

			using ObjectMap       = std::unordered_map<ObjectIndex, Entity>;
			using PlayerObjectMap = std::unordered_map<PlayerIndex, Entity>;
			
			//using ObjectCreationFunction = Entity(*)(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const util::json& data);
			using ObjectCreationFunction = std::function<Entity(World& world, Entity parent, util::Logger&, const filesystem::path&, const PlayerObjectMap&, const ObjectMap&, const util::json&)>;

			using ObjectCreationMap = std::unordered_map<std::string, ObjectCreationFunction>;

			static Entity Load(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const util::json& data);

		protected:
			static const ObjectCreationMap ObjectRoutines;

			static Entity CreateCamera(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data);
			static Entity CreateLight(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data);
			static Entity CreateFollowSphere(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data);
			static Entity CreateBillboard(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data);
			static Entity CreatePlatform(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data);

			static void resolve_parent(World& world, Entity entity, Entity stage, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data);

			static Entity resolve_object_reference(const std::string& query, World& world, const PlayerObjectMap& player_objects, const ObjectMap& objects); // std::tuple<std::string, std::string>

			static inline math::TransformVectors get_transform_data(util::Logger& dbg, const util::json& cfg)
			{
				auto tform = util::get_transform(cfg);

				auto [position, rotation, scale] = tform;

				dbg->info("Transform:");

				dbg->info("Position: {}", position);
				dbg->info("Rotation: {}", rotation);
				dbg->info("Scale: {}\n", scale);

				return tform;
			};

			static Transform apply_transform(World& world, util::Logger& dbg, Entity entity, const util::json& cfg);
			static std::optional<graphics::ColorRGBA> apply_color(World& world, util::Logger& dbg, Entity entity, const util::json& cfg);

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
	};
}