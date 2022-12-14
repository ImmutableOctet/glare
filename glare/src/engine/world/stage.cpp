#include "stage.hpp"

#include "world.hpp"
#include "world_events.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "player/player.hpp"
#include "graphics_entity.hpp"

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/entity/entity_factory.hpp>

#include "physics/components/collision_component.hpp"

// Behaviors supported in stage format:
#include "behaviors/spin_behavior.hpp"
#include "behaviors/billboard_behavior.hpp"
#include "behaviors/simple_follow_behavior.hpp"

// TODO: Revisit whether debug functionality should be included here.
#include "debug/debug.hpp"
#include "behaviors/debug_move_behavior.hpp"

#include <engine/config.hpp>
#include <engine/components/name_component.hpp>
#include <engine/components/player_component.hpp>
#include <engine/components/model_component.hpp>

#include <util/json.hpp>
#include <util/log.hpp>
#include <util/string.hpp>

#include <string>
#include <regex>

// Debugging related:
#include <engine/entity/entity_descriptor.hpp>
#include "behaviors/rave_behavior.hpp"

// Not sure if this is actually going to be a standard include or not.
#include "behaviors/target_behavior.hpp"

namespace engine
{
	// Stage:
	const Stage::ObjectCreationMap Stage::ObjectRoutines =
	{
		{ "camera",        Stage::CreateCamera       },
		{ "follow_sphere", Stage::CreateFollowSphere },
		{ "billboard",     Stage::CreateBillboard    },
		{ "platform",      Stage::CreatePlatform     },
		{ "light",         Stage::CreateLight        },
		{ "scenery",       Stage::CreateScenery      }
	};

    Entity Stage::Load(World& world, const filesystem::path& root_path, const util::json& data, Entity parent, const Stage::Loader::Config& cfg)
    {
		auto state = Stage::Loader(world, root_path, data);

		return state.load(cfg, parent);
    }

	Entity Stage::CreateCamera(World& world, Entity parent, const filesystem::path& root_path, const PlayerObjectMap& player_objects, const ObjectMap& objects, const util::json& camera_cfg)
	{
		auto& registry = world.get_registry();

		auto params = CameraParameters(camera_cfg);

		bool make_active = util::get_value(camera_cfg, "make_active", false);
		bool collision_enabled = util::get_value(camera_cfg, "collision", false);

		auto camera_parent = parent; // < --TODO: look into weird bug where camera goes flying when a parent is assigned.

		auto camera = create_camera(world, params, camera_parent, make_active, collision_enabled);

		bool is_debug_camera = util::get_value(camera_cfg, "debug", false);

		if (is_debug_camera)
		{
			attach_debug_camera_features(world, camera);
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
					registry.emplace<TargetBehavior>(camera, player, 0.5f); // ((is_debug_camera) ? 0.0f : 0.5f)
					registry.emplace<SimpleFollowBehavior>(camera, player, 25.0f, 0.7f, 100.0f, true);
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
			//	assert(false);

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

		assert(target != null);

		auto following_distance = util::get_value(data, "following_distance", SimpleFollowBehavior::DEFAULT_FOLLOWING_DISTANCE);
		auto follow_speed = util::get_value(data, "follow_speed", SimpleFollowBehavior::DEFAULT_FOLLOW_SPEED);
		auto max_distance = util::get_value(data, "max_distance", SimpleFollowBehavior::DEFAULT_MAX_DISTANCE);
		auto force_catch_up = util::get_value(data, "force_catch_up", SimpleFollowBehavior::DEFAULT_FORCE_CATCH_UP);

		registry.emplace<SimpleFollowBehavior>(obj, target, following_distance, follow_speed, max_distance, force_catch_up);

		registry.emplace<BillboardBehavior>(obj);

		// Debugging related:
		registry.emplace<RaveBehavior>(obj);

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

		auto type = EntityType::Scenery;
		auto solid = util::get_value<bool>(data, "solid", false);

		auto obj = load_model(world, model_path, parent, type, true, CollisionConfig(type, solid), 0.0f);

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

	void Stage::apply_transform(World& world, Entity entity, const util::json& cfg)
	{
		auto tform_data = get_transform_data(cfg);

		//world.apply_transform(entity, tform_data);
		world.apply_transform_and_reset_collision(entity, tform_data);
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

	// Stage::Loader:
	Entity Stage::Loader::make_stage_pivot(World& world, Entity parent)
	{
		//parent = create_pivot(world, parent);
		auto stage = create_pivot(world, parent);

		return stage;
	}

	Stage::Loader::Loader(World& world, const filesystem::path& root_path, const util::json& data, Entity stage):
		world(world),
		root_path(root_path),
		data(data),
		stage(stage)
	{}

	bool Stage::Loader::ensure_stage(Entity parent)
	{
		if (stage != null)
		{
			return false;
		}

		// Automatically generated scene/stage pivot:
		print("Creating scene pivot...");

		stage = make_stage_pivot(world, parent);

		return true;
	}

	Entity Stage::Loader::load(const Stage::Loader::Config& cfg, Entity parent)
	{
		auto stage = this->stage;

		if (stage == null)
		{
			stage = make_stage_pivot(world, parent);
		}

		return load(stage, cfg, parent);
	}

	Entity Stage::Loader::load(Entity stage, const Stage::Loader::Config& cfg, Entity parent, bool load_title)
	{
		assert(stage != null);

		this->stage = stage;

		load_properties((stage == this->stage));

		// Stage geometry:
		if (cfg.geometry)
		{
			load_geometry();
		}

		// Players:
		if (cfg.players)
		{
			load_players();
		}

		// Objects:
		if (cfg.objects)
		{
			load_objects();
		}

		// Apply stage transform, etc.
		if (cfg.apply_transform)
		{
			print("Applying stage transform...");
			apply_transform(world, stage, data);
		}

		return stage;
	}

	void Stage::Loader::load_properties(bool load_title, const std::string& default_title)
	{
		if ((load_title) && (stage != null))
		{
			world.set_name(stage, util::get_value<std::string>(data, "title", util::get_value<std::string>(data, "name", default_title)));
		}

		print("Initializing stage properties...");

		world.set_properties(WorldProperties::from_json(data));
	}

	void Stage::Loader::load_geometry()
	{
		ensure_stage();

		print("Loading scene geometry...");

		ForEach(data["models"], [&](const auto& model_cfg)
		{
			auto model_path = (root_path / model_cfg["path"].get<std::string>()).string();

			bool collision_enabled = util::get_value(model_cfg, "collision", true);

			print("Loading geometry from \"{}\"...\n", model_path);

			auto type = EntityType::Geometry;
			auto model = load_model(world, model_path, stage, type, true, CollisionConfig(type, collision_enabled));

			print("Applying transformation to stage geometry...");

			apply_transform(world, model, model_cfg);
		});
	}

	void Stage::Loader::load_players()
	{
		ensure_stage();

		print("Loading players...");

		auto& resource_manager = get_resource_manager();

		ForEach(data["players"], [&](const auto& player_cfg)
		{
			auto& registry = world.get_registry();

			auto& player_idx_counter = indices.players.player_idx_counter;
			auto& player_objects = indices.players.player_objects;

			auto player_character = util::get_value<std::string>(player_cfg, "character", DEFAULT_CHARACTER);
			auto player_name      = util::get_value<std::string>(player_cfg, "name", DEFAULT_PLAYER_NAME);
			auto player_idx       = util::get_value<PlayerIndex>(player_cfg, "index", player_idx_counter);
			auto player_parent    = stage; // null;

			print("Player #{}", player_idx);
			print("Name: {}", player_name);
			print("Character: {}", player_character);

			auto character_directory = (std::filesystem::path("assets/characters") / player_character);
			auto character_path = (character_directory / "instance.json");

			//auto character_data = load_character_data(player_character);
			//auto player = create_player(world, character_data, player_name, player_parent, player_idx);

			// [INSERT LOOKUP INTO RESOURCE MANAGER HERE]
			auto player = resource_manager.generate_entity
			(
				{
					.paths =
					{
						.instance_path               = character_path,
						.instance_directory          = character_directory,
						.service_archetype_root_path = "archetypes/world"
					}
				},

				{
					//world,

					.registry = world.get_registry(), // registry,
					.resource_manager = world.get_resource_manager(),

					.parent = player_parent
				}
			);

			print("Entity: {}", player);
			print("Parent: {}", player_parent);

			if (player == null)
			{
				return;
			}

			if (player_idx != NoPlayer)
			{
				player_objects[player_idx] = player;
			}

			registry.emplace_or_replace<PlayerComponent>(player, player_idx);

			apply_transform(world, player, player_cfg);

			world.event<OnPlayerLoaded>(player, character_path);

			/*
			if (util::get_value(player_cfg, "debug", false))
			{
				// TODO.
			}
			*/

			player_idx_counter = std::max((player_idx_counter+1), (player_idx+1));
		});
	}

	void Stage::Loader::load_objects()
	{
		auto& registry = world.get_registry();
		auto& obj_idx_counter = indices.objects.obj_idx_counter;
		auto& objects = indices.objects.objects;
		auto& player_objects = indices.players.player_objects;

		ensure_stage();

		print("Loading objects...");

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
						apply_color(world, obj, obj_cfg);
						apply_transform(world, obj, obj_cfg);

						// TODO: Ensure `tform` doesn't get invalidated by call to `resolve_parent`.
						// (May actually make sense to call `get_matrix` before `resolve_parent` anyway)
						resolve_parent(world, obj, stage, root_path, player_objects, objects, obj_cfg);

						print("\"{}\" object created.", obj_type);
					}
				}
			}

			if (obj != null)
			{
				auto obj_idx = util::get_value<ObjectIndex>(obj_cfg, "index", obj_idx_counter);
				auto obj_name = util::get_value<std::string>(obj_cfg, "name", "");

				if (!obj_name.empty())
				{
					print("Object name: {}", obj_name);

					registry.emplace<NameComponent>(obj, obj_name);
				}

				objects[obj_idx] = obj;

				obj_idx_counter = std::max((obj_idx_counter + 1), (obj_idx + 1));
			}
		});
	}

	ResourceManager& Stage::Loader::get_resource_manager() const
	{
		return world.get_resource_manager();
	}
}