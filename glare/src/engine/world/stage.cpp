#include "stage.hpp"

#include "world.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "player.hpp"

#include "target_component.hpp"
#include "follow_component.hpp"
#include "billboard_behavior.hpp"
#include "rave_component.hpp"
#include "spin_component.hpp"

#include "debug/debug.hpp"

#include <engine/name_component.hpp>
#include <engine/model_component.hpp>

#include <util/json.hpp>
#include <util/log.hpp>
#include <util/string.hpp>

#include <string>
#include <regex>

// Debugging related:
#include <iostream>

namespace engine
{
	const Stage::ObjectCreationMap Stage::ObjectRoutines
	=
	{
		{ "camera", Stage::CreateCamera },
		{ "follow_sphere", Stage::CreateFollowSphere },
		{ "billboard", Stage::CreateBillboard },
		{ "platform", Stage::CreatePlatform }
	};

    Entity Stage::Load(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const util::json& data)
    {
		constexpr PlayerIndex NoPlayer = PlayerState::NoPlayer;

		PlayerObjectMap player_objects;
		ObjectMap       objects;

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

			//parent = create_pivot(world, parent);

			stage = create_pivot(world, position, rotation, scale, parent);

			registry.emplace<NameComponent>(stage, util::get_value<std::string>(data, "title", util::get_value<std::string>(data, "name", "Unknown Stage")));
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
			auto player_parent = stage; // null;

			auto player = create_player(world, player_tform, player_char, player_name, player_parent, player_idx);

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

		ObjectIndex obj_idx_counter = 1; // std::uint16_t

		ForEach(data["objects"], [&](const auto& obj_cfg)
		{
			auto obj_type = util::get_value<std::string>(obj_cfg, "type", "");

			Entity obj = null;

			if (obj_type.empty())
			{
				auto tform = get_transform_data(dbg, obj_cfg);

				obj = create_pivot(world, tform, stage);
			}
			else
			{
				auto o = ObjectRoutines.find(obj_type);

				if (o == ObjectRoutines.end())
				{
					dbg->warn("Unkown object detected: \"{}\"", obj_type);
				}
				else
				{
					auto obj_fn = o->second;

					obj = obj_fn(world, stage, dbg, root_path, player_objects, objects, obj_cfg);

					if (obj != null)
					{
						apply_transform(world, dbg, obj, obj_cfg);
						apply_color(world, dbg, obj, obj_cfg);
					}
				}
			}

			if (obj != null)
			{
				auto obj_idx = util::get_value<ObjectIndex>(obj_cfg, "index", obj_idx_counter);
				auto obj_name = util::get_value<std::string>(obj_cfg, "name", "");

				if (!obj_name.empty())
				{
					registry.emplace<NameComponent>(obj, obj_name);
				}

				objects[obj_idx] = obj;

				obj_idx_counter = std::max((obj_idx_counter + 1), (obj_idx + 1));
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

	Entity Stage::CreateCamera(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& camera_cfg)
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

		auto camera_parent = parent; // < --TODO: look into weird bug where camera goes flying when a parent is assigned.

		auto camera = create_camera(world, params, camera_parent, make_active);

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
					registry.emplace<SimpleFollowComponent>(camera, player, 25.0f, 0.7f, 100.0f, true);
				}
			}
		}

		return camera;
	}

	Entity Stage::CreateFollowSphere(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto& registry = world.get_registry();

		auto obj = load_model(world, "assets/objects/follow_sphere/follow_sphere.b3d", parent);

		auto target_query = data["target"].get<std::string>();

		auto target = resolve_object_reference(target_query, world, player_objects, objects);

		ASSERT(target != null);

		auto following_distance = util::get_value(data, "following_distance", SimpleFollowComponent::DEFAULT_FOLLOWING_DISTANCE);
		auto follow_speed = util::get_value(data, "follow_speed", SimpleFollowComponent::DEFAULT_FOLLOW_SPEED);
		auto max_distance = util::get_value(data, "max_distance", SimpleFollowComponent::DEFAULT_MAX_DISTANCE);
		auto force_catch_up = util::get_value(data, "force_catch_up", SimpleFollowComponent::DEFAULT_FORCE_CATCH_UP);

		registry.emplace<SimpleFollowComponent>(obj, target, following_distance, follow_speed, max_distance, force_catch_up);

		registry.emplace<BillboardBehavior>(obj);
		registry.emplace<RaveComponent>(obj);

		return obj;
	}

	Entity Stage::CreateBillboard(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto& registry = world.get_registry();
		auto obj = load_model(world, "assets/objects/billboard/billboard.b3d", parent);

		auto mode = BillboardBehavior::resolve_mode(util::get_value<std::string>(data, "mode"));

		auto interp = util::get_value(data, "speed", 0.5f);

		if (mode == BillboardBehavior::Mode::Yaw) // Pitch, Roll
		{
			interp = math::radians(interp);
		}

		bool allow_roll = util::get_value(data, "allow_roll", true); // false

		auto bb = BillboardBehavior { { null, interp, mode, allow_roll } };

		registry.emplace<BillboardBehavior>(obj, bb);

		return obj;
	}

	Entity Stage::CreatePlatform(World& world, Entity parent, util::Logger& dbg, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto& registry = world.get_registry();
		auto obj = load_model(world, "assets/objects/platform/platform.b3d", parent);

		auto mode = util::get_value<std::string>(data, "mode", "static");

		if (mode == "static")
		{
			// Nothing so far.
		}
		else if (mode == "rotate")
		{
			auto direction = util::get_vector(data, "direction", {0.0f, 0.05f, 0.0f});

			registry.emplace<SpinBehavior>(obj, direction);
		}

		return obj;
	}

	Entity Stage::resolve_object_reference(const std::string& query, World& world, const PlayerObjectMap& player_objects, const ObjectMap& objects)
	{
		static const auto re = std::regex("(object|player)?(\\@|\\#)\"?([\\w\\s\\d\\-]+)\"?"); // constexpr

		auto results = util::get_regex_groups(query, re);

		std::string target_pool = results[1];

		if (target_pool == "player")
		{
			std::string player_name = results[3];
			auto idx = static_cast<PlayerIndex>(std::stoul(player_name));

			auto p = player_objects.find(idx);

			if (p == player_objects.end())
			{
				return world.get_by_name(player_name);
			}
			else
			{
				return p->second;
			}
		}
		else if ((target_pool.empty()) || (target_pool == "object"))
		{
			auto ref_type = results[2];

			if (ref_type == "#")
			{
				auto idx = static_cast<ObjectIndex>(std::stoul(results[3]));

				auto o = objects.find(idx);

				if (o != objects.end())
				{
					return o->second;
				}
			}
			else // "@"
			{
				std::string obj_name = results[3];

				return world.get_by_name(obj_name);
			}
		}

		return null;
	}

	Transform Stage::apply_transform(World& world, util::Logger& dbg, Entity entity, const util::json& cfg)
	{
		return world.apply_transform(entity, get_transform_data(dbg, cfg));
	}

	std::optional<graphics::ColorRGBA> Stage::apply_color(World& world, util::Logger& dbg, Entity entity, const util::json& cfg)
	{
		//auto color = util::get_color(cfg, "color");

		if (!cfg.contains("color"))
		{
			return std::nullopt;
		}

		auto& registry = world.get_registry();

		auto* model = registry.try_get<ModelComponent>(entity);

		if (model)
		{
			auto color = util::to_color(cfg["color"], 1.0f);

			model->color = color;

			return color;
		}

		return std::nullopt;
	}
}