#include "stage.hpp"

#include "world.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "player.hpp"

#include "target_component.hpp"
#include "follow_component.hpp"

#include "debug/debug.hpp"

#include <engine/name_component.hpp>

#include <util/json.hpp>
#include <util/log.hpp>

#include <string>

// Debugging related:
#include <iostream>

namespace engine
{
	const Stage::ObjectCreationMap Stage::ObjectRoutines
	=
	{
		{ "camera", &Stage::CreateCamera }
	};

    Entity Stage::Load(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const util::json& data)
    {
		constexpr PlayerIndex NoPlayer = PlayerState::NoPlayer;

		PlayerObjectMap player_objects;

		auto& registry = world.get_registry();

		// Global settings:

		// Stage pivot:
		Entity stage = null;

		{
			auto [position, rotation, scale] = util::get_transform(data);

			dbg->info("Transform:");

			dbg->info("Position: {}", position);
			dbg->info("Rotation: {}", rotation);
			dbg->info("Scale: {}\n", scale);

			dbg->info("Creating pivot...");

			stage = create_pivot(world, position, rotation, scale, parent);
		}

		// Players:
		dbg->info("Loading players...");

		PlayerIndex player_idx_counter = 1;

		ForEach(data["players"], [&](const auto& player_cfg)
		{
			auto& registry = world.get_registry();

			auto player_tform = get_transform_data(dbg, player_cfg);
			auto player_char  = get_character(util::get_value<std::string>(player_cfg, "character", "default"));
			auto player_name  = util::get_value<std::string>(player_cfg, "name", DEFAULT_PLAYER_NAME);
			auto player_idx   = util::get_value<PlayerIndex>(player_cfg, "index", player_idx_counter);

			auto player = create_player(world, player_tform, player_char, player_name, stage, player_idx);

			if (player_idx != NoPlayer)
			{
				player_objects[player_idx] = player;
			}

			/*
			if (util::get_value(player_cfg, "debug", false))
			{
				// TODO.
			}
			*/

			player_idx_counter = std::max((player_idx_counter+1), (player_idx+1));
		});

		// Cameras:
		dbg->info("Loading objects...");

		ForEach(data["objects"], [&](const auto& obj_cfg)
		{
			auto obj_type = obj_cfg["type"].get<std::string>();

			auto o = ObjectRoutines.find(obj_type);

			if (o == ObjectRoutines.end())
			{
				dbg->warn("Unkown object detected: \"{}\"", obj_type);
			}
			else
			{
				auto obj_fn = o->second;

				auto obj = obj_fn(world, stage, dbg, root_path, player_objects, obj_cfg);

				if (obj != null)
				{
					apply_transform(world, dbg, obj, obj_cfg);
				}
			}
		});

		// Models:
		dbg->info("Loading models...");

		//auto& resource_manager = world.get_resource_manager();

		ForEach(data["models"], [&](const auto& model_cfg)
		{
			auto model_path = (root_path / model_cfg["path"].get<std::string>()).string();

			dbg->info("Loading model from \"{}\"...\n", model_path);

			auto model = load_model(world, model_path, stage);

			apply_transform(world, dbg, model, model_cfg);
		});

		return stage;
    }

	Entity Stage::CreateCamera(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const util::json& camera_cfg)
	{
		auto& registry = world.get_registry();

		auto params = CameraParameters
		(
			util::get_value(camera_cfg, "fov", CameraParameters::DEFAULT_FOV),
			util::get_value(camera_cfg, "near", CameraParameters::NEAR_PLANE),
			util::get_value(camera_cfg, "far", CameraParameters::FAR_PLANE),
			util::get_value(camera_cfg, "aspect_ratio", CameraParameters::ASPECT)
		);

		bool make_active = util::get_value(camera_cfg, "make_active", false);

		auto camera = create_camera(world, params, null, make_active); // stage <-- TODO: look into weird bug where camera goes flying when a parent is assigned.

		auto camera_name = util::get_value<std::string>(camera_cfg, "name", "");

		if (!camera_name.empty())
		{
			registry.emplace<NameComponent>(camera, camera_name);
		}

		bool is_debug_camera = util::get_value(camera_cfg, "debug", false);

		if (is_debug_camera)
		{
			debug::attach_debug_camera_features(world, camera);
		}

		auto target_player = util::get_value<PlayerIndex>(camera_cfg, "player", PlayerState::NoPlayer);

		if (target_player != PlayerState::NoPlayer)
		{
			auto p = player_objects.find(target_player);

			if (p != player_objects.end())
			{
				auto player = p->second;

				if ((!is_debug_camera))
				{
					registry.emplace<TargetComponent>(camera, player, 0.5f); // ((is_debug_camera) ? 0.0f : 0.5f)
					registry.emplace<SimpleFollowComponent>(camera, player, 20.0f, 0.8f, 200.0f, true);
				}
			}
		}

		return camera;
	}

	Transform Stage::apply_transform(World& world, util::Logger& dbg, Entity entity, const util::json& cfg)
	{
		return world.apply_transform(entity, get_transform_data(dbg, cfg));
	};
}