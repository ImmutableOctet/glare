#include "stage.hpp"

#include "world.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "player.hpp"

#include "target_component.hpp"
#include "follow_component.hpp"
#include "billboard_behavior.hpp"
#include "rave_component.hpp"
#include "spin_component.hpp"

#include "debug/debug.hpp"

#include <engine/config.hpp>
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
		{ "camera",        Stage::CreateCamera       },
		{ "follow_sphere", Stage::CreateFollowSphere },
		{ "billboard",     Stage::CreateBillboard    },
		{ "platform",      Stage::CreatePlatform     },
		{ "light",         Stage::CreateLight        },
		{ "scenery",       Stage::CreateScenery      }
	};

    Entity Stage::Load(World& world, Entity parent, const filesystem::path& root_path, const util::json& data, const Stage::Config& cfg)
    {
		constexpr PlayerIndex NoPlayer = PlayerState::NoPlayer;

		PlayerObjectMap player_objects;
		ObjectMap       objects;

		auto& registry = world.get_registry();

		// Global settings:

		// Stage pivot:
		print("Creating scene pivot...");

		//parent = create_pivot(world, parent);

		//Entity stage = null;
		auto stage = create_pivot(world, parent);

		registry.emplace<NameComponent>(stage, util::get_value<std::string>(data, "title", util::get_value<std::string>(data, "name", "Unknown Stage")));

		print("Initializing stage properties...");
		world.properties = {data};

		//auto& resource_manager = world.get_resource_manager();

		// Stage geometry:
		if (cfg.geometry)
		{
			print("Loading scene geometry...");

			ForEach(data["models"], [&](const auto& model_cfg)
			{
				auto model_path = (root_path / model_cfg["path"].get<std::string>()).string();

				bool collision_enabled = util::get_value(model_cfg, "collision", true);

				print("Loading geometry from \"{}\"...\n", model_path);

				auto model = load_model(world, model_path, stage, EntityType::Geometry, true, collision_enabled);

				print("Applying transformation to stage geometry...");

				apply_transform(world, model, model_cfg);
			});
		}

		// Players:
		if (cfg.players)
		{
			print("Loading players...");

			PlayerIndex player_idx_counter = 1;

			ForEach(data["players"], [&](const auto& player_cfg)
			{
				auto& registry = world.get_registry();

				auto player_tform  = get_transform_data(player_cfg);
				auto player_char   = get_character(util::get_value<std::string>(player_cfg, "character", "default"));
				auto player_name   = util::get_value<std::string>(player_cfg, "name", DEFAULT_PLAYER_NAME);
				auto player_idx    = util::get_value<PlayerIndex>(player_cfg, "index", player_idx_counter);
				auto player_parent = stage; // null;

				print("Player #{}", player_idx);
				print("Name: {}", player_name);
				print("Character: {}", static_cast<std::uint8_t>(player_char));

				auto player = create_player(world, player_tform, player_char, player_name, player_parent, player_idx);

				print("Entity: {}", player);
				print("Parent: {}", player_parent);

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
		}

		// Objects:
		if (cfg.objects)
		{
			print("Loading objects...");

			ObjectIndex obj_idx_counter = 1; // std::uint16_t

			ForEach(data["objects"], [&](const auto& obj_cfg)
			{
				auto obj_type = util::get_value<std::string>(obj_cfg, "type", "");

				Entity obj = null;

				if (obj_type.empty())
				{
					auto tform = get_transform_data(obj_cfg);

					obj = create_pivot(world, tform, stage);
				}
				else
				{
					auto o = ObjectRoutines.find(obj_type);

					if (o == ObjectRoutines.end())
					{
						print_warn("Unkown object detected: \"{}\"", obj_type);
					}
					else
					{
						auto obj_fn = o->second;

						print("Creating object of type \"{}\"...", obj_type);

						obj = obj_fn(world, stage, root_path, player_objects, objects, obj_cfg);

						if (obj != null)
						{
							apply_transform(world, obj, obj_cfg);
							apply_color(world, obj, obj_cfg);

							resolve_parent(world, obj, stage, root_path, player_objects, objects, obj_cfg);

							print("\"{}\" object created.", obj_type);
						}
					}
				}

				if (obj != null)
				{
					auto obj_idx = util::get_value<ObjectIndex>(obj_cfg, "index", obj_idx_counter);
					auto obj_name = util::get_value<std::string>(obj_cfg, "name", "");

					print("Object name: {}", obj_name);

					if (!obj_name.empty())
					{
						registry.emplace<NameComponent>(obj, obj_name);
					}

					objects[obj_idx] = obj;

					obj_idx_counter = std::max((obj_idx_counter + 1), (obj_idx + 1));
				}
			});
		}

		// Apply stage transform, etc.
		if (cfg.apply_transform)
		{
			auto tform = util::get_transform(data);

			auto [position, rotation, scale] = tform;

			print("Stage Transform:");

			print("Position: {}", position);
			print("Rotation: {}", rotation);
			print("Scale: {}\n", scale);

			world.apply_transform(stage, tform);
		}

		return stage;
    }

	Entity Stage::CreateCamera(World& world, Entity parent, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& camera_cfg)
	{
		auto& registry = world.get_registry();

		auto params = CameraParameters(camera_cfg);

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

	Entity Stage::CreateLight(World& world, Entity parent, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto debug_mode = util::get_value(data, "debug", false); // true

		auto type = LightComponent::resolve_light_mode(util::get_value<std::string>(data, "mode", "directional"));

		Entity light = null;

		auto properties = LightProperties
		{
			.ambient  { util::get_color_rgb(data, "ambient", { 0.0f, 0.0f, 0.0f }) },
			.diffuse  { util::get_color_rgb(data, "color", util::get_color_rgb(data, "diffuse", { 1.0f, 1.0f, 1.0f })) },
			.specular { util::get_color_rgb(data, "specular", { 0.1f, 0.1f, 0.1f }) }
		};

		switch (type)
		{
			case LightType::Directional:
			{
				auto advanced_properties = LightProperties::Directional
				{
					/*
						.direction { util::get_vector(data, "direction") }
					*/

					.use_position { util::get_value(data, "use_position", false) }
				};

				light = create_directional_light(world, {}, properties, advanced_properties, parent, debug_mode);

				break;
			}
			case LightType::Point:
			{
				auto advanced_properties = LightProperties::Point
				{
					.linear { util::get_value(data, "linear", LightProperties::Point::DEFAULT_LINEAR) },
					.quadratic { util::get_value(data, "quadratic", LightProperties::Point::DEFAULT_QUADRATIC) }
				};

				light = create_point_light(world, {}, properties, advanced_properties, parent, debug_mode);

				break;
			}

			case LightType::Spotlight:
			{
				auto advanced_properties = LightProperties::Spot
				{
					.cutoff { util::get_value(data, "cutoff", LightProperties::Spot::DEFAULT_CUTOFF()) },
					.outer_cutoff { util::get_value(data, "outer_cutoff", LightProperties::Spot::DEFAULT_OUTER_CUTOFF()) },

					.constant { util::get_value(data, "constant", LightProperties::Spot::DEFAULT_CONSTANT) },
					.linear { util::get_value(data, "linear", LightProperties::Spot::DEFAULT_LINEAR) },
					.quadratic { util::get_value(data, "quadratic", LightProperties::Spot::DEFAULT_QUADRATIC) }
				};

				light = create_spot_light(world, {}, properties, advanced_properties, parent, debug_mode);

				break;
			}

			//default:
			//	// Unsupported.
			//	ASSERT(false);

			//	break;
		}

		if (light == null)
		{
			return null;
		}

		auto shadows_enabled = util::get_value(data, "shadows", false);

		if (shadows_enabled)
		{
			const auto& cfg = world.get_config();

			auto shadow_light_type_str = util::get_value<std::string>(data, "shadow_type", {});

			auto shadow_light_type = ((shadow_light_type_str.empty()) ? std::nullopt : std::optional<LightType> { LightComponent::resolve_light_mode(shadow_light_type_str) }); // std::optional<LightType> {}

			auto shadow_resolution = util::get_vec2i
			(
				data,
				"shadow_resolution",

				(shadow_light_type == LightType::Point)
				?
				cfg.graphics.shadow.cubemap_resolution
				:
				cfg.graphics.shadow.resolution
			);

			auto shadow_range = util::get_value(data, "shadow_range", 1000.0f);

			attach_shadows(world, light, shadow_resolution, shadow_range, shadow_light_type);
		}

		return light;
	}

	Entity Stage::CreateFollowSphere(World& world, Entity parent, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto& registry = world.get_registry();

		auto obj = load_model(world, "assets/objects/follow_sphere/follow_sphere.b3d", parent, EntityType::Object);

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

	Entity Stage::CreateBillboard(World& world, Entity parent, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto& registry = world.get_registry();
		auto obj = load_model(world, "assets/objects/billboard/billboard.b3d", parent, EntityType::Object);

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

	Entity Stage::CreatePlatform(World& world, Entity parent, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto& registry = world.get_registry();
		auto obj = load_model(world, "assets/objects/platform/platform.b3d", parent, EntityType::Object);

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

	Entity Stage::CreateScenery(World& world, Entity parent, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto& registry = world.get_registry();

		auto model_path = util::get_value<std::string>(data, "path");

		if (model_path.empty())
		{
			model_path = "assets/geometry/cube.b3d"; // <-- Debugging related.
		}
		else
		{
			model_path = (root_path / model_path).string();
		}

		auto solid = util::get_value<bool>(data, "solid", false);

		auto obj = load_model(world, model_path, parent, EntityType::Scenery, true, solid, 0.0f);

		return obj;
	}

	void Stage::resolve_parent(World& world, Entity entity, Entity stage, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& data)
	{
		auto parent_query = util::get_value<std::string>(data, "parent"); // data["parent"].get<std::string>();

		auto parent = resolve_object_reference(parent_query, world, player_objects, objects);

		if (parent == null)
		{
			parent = stage;
		}

		if (parent != null)
		{
			world.set_parent(entity, parent);
		}
	}

	Entity Stage::resolve_object_reference(const std::string& query, World& world, const PlayerObjectMap& player_objects, const ObjectMap& objects)
	{
		static const auto re = std::regex("(object|player)?(\\@|\\#)\"?([\\w\\s\\d\\-]+)\"?"); // constexpr

		if (query.empty())
		{
			return null;
		}

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

	Transform Stage::apply_transform(World& world, Entity entity, const util::json& cfg)
	{
		return world.apply_transform(entity, get_transform_data(cfg));
	}

	std::optional<graphics::ColorRGBA> Stage::apply_color(World& world, Entity entity, const util::json& cfg)
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