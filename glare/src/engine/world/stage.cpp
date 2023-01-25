#include "stage.hpp"

#include "world.hpp"
#include "world_events.hpp"
#include "entity.hpp"
#include "graphics_entity.hpp"

#include "components/camera_component.hpp"
#include "physics/components/collision_component.hpp"
#include "behaviors/simple_follow_behavior.hpp"

// TODO: Revisit whether debug functionality should be included here.
#include "behaviors/debug_move_behavior.hpp"

#include <engine/config.hpp>
#include <engine/meta/meta.hpp>
#include <engine/meta/serial.hpp>
#include <engine/resource_manager/resource_manager.hpp>
#include <engine/entity/entity_factory.hpp>
#include <engine/entity/serial.hpp>

#include <engine/components/name_component.hpp>
#include <engine/components/model_component.hpp>
#include <engine/components/player_component.hpp>
#include <engine/components/player_target_component.hpp>

#include <engine/input/components/input_component.hpp>

#include <math/math.hpp>

#include <util/json.hpp>
#include <util/log.hpp>
#include <util/string.hpp>
#include <util/format.hpp>
#include <util/algorithm.hpp>

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
	template <typename ...IgnoredKeys>
	void Stage::process_component_entries(Registry& registry, Entity entity, const util::json& object_data, const ParsingContext& parsing_context, IgnoredKeys&&... ignored_keys)
	{
		util::enumerate_map_filtered_ex
		(
			object_data.items(),

			hash,

			[&registry, entity, &parsing_context](const auto& component_declaration, const auto& component_content)
			{
				auto [component_name, allow_entry_update, constructor_arg_count] = parse_component_declaration(component_declaration, true);

				if (auto component = process_component(component_name, &component_content, constructor_arg_count, {}, &parsing_context))
				{
					if (!allow_entry_update || !EntityFactory::update_component_fields(registry, entity, *component, true, true))
					{
						EntityFactory::emplace_component(registry, entity, *component);
					}
				}
			},

			// Treat every object entry (excluding these keys) as component declarations:

			// Transformation attributes:
			"position", "rotation", "scale",

			// General:
			"type", "player", "index", "parent", "color", // "target",

			std::forward<IgnoredKeys>(ignored_keys)...
		);
	}

    Entity Stage::Load(World& world, const filesystem::path& root_path, const util::json& data, Entity parent, const Stage::Loader::Config& cfg)
    {
		auto state = Stage::Loader(world, root_path, data);

		return state.load(cfg, parent);
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
			const auto& ref_type = results[2];

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

	math::TransformVectors Stage::get_transform_data(const util::json& cfg)
	{
		auto position = math::Vector {};
		auto rotation = math::Vector {};
		auto scale    = math::Vector { 1.0f, 1.0f, 1.0f };

		if (auto vector_data = cfg.find("position"); vector_data != cfg.end())
		{
			engine::load(position, *vector_data);
		}

		if (auto vector_data = cfg.find("rotation"); vector_data != cfg.end())
		{
			engine::load(rotation, *vector_data);

			rotation = math::radians(rotation);
		}

		if (auto vector_data = cfg.find("scale"); vector_data != cfg.end())
		{
			engine::load(scale, *vector_data);
		}

		return { position, rotation, scale };
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

		if (auto* model = registry.try_get<ModelComponent>(entity))
		{
			const auto color = util::to_color(cfg["color"], 1.0f);

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

		if (auto properties = data.find("properties"); properties != data.end())
		{
			print("Initializing stage properties...");

			world.set_properties(engine::load<WorldProperties>(*properties));
		}
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

		auto& registry = world.get_registry();
		const auto& config = world.get_config();

		ForEach(data["players"], [&](const auto& player_cfg)
		{
			auto& player_idx_counter = indices.players.player_idx_counter;
			auto& player_objects = indices.players.player_objects;

			auto player_character = util::get_value<std::string>(player_cfg, "character", config.players.default_player.character);
			auto player_parent    = stage; // null;

			auto character_directory = (std::filesystem::path(config.players.character_path) / player_character);
			auto character_path = (character_directory / util::format("{}.json", player_character));

			auto player = resource_manager.generate_entity
			(
				{
					{
						.instance_path               = character_path,
						.instance_directory          = character_directory,
						.shared_directory            = config.players.character_path,
						.service_archetype_root_path = (std::filesystem::path(config.entity.archetype_path) / "world"),
						.archetype_root_path         = config.entity.archetype_path
					}
				},

				{
					//world,

					.registry = registry,
					.resource_manager = resource_manager,

					.parent = player_parent
				}
			);

			//assert(player != null);

			if (player == null)
			{
				return;
			}

			process_component_entries
			(
				registry, player, player_cfg,
				resource_manager.get_parsing_context(),

				"character"
			);

			auto player_idx = util::get_value<PlayerIndex>(player_cfg, "player", util::get_value<PlayerIndex>(player_cfg, "index", player_idx_counter));

			assert(player_idx != NO_PLAYER);

			registry.emplace_or_replace<PlayerComponent>(player, player_idx);
			registry.emplace_or_replace<InputComponent>(player, static_cast<InputStateIndex>(player_idx));

			if (!config.players.default_player.name.empty())
			{
				registry.emplace_or_replace<NameComponent>(player, config.players.default_player.name);
			}

			player_objects[player_idx] = player;

			apply_transform(world, player, player_cfg);

			world.event<OnPlayerLoaded>(player, character_path);

			player_idx_counter = std::max((player_idx_counter+1), (player_idx+1));
		});
	}

	void Stage::Loader::load_objects()
	{
		auto& registry = world.get_registry();
		auto& resource_manager = world.get_resource_manager();

		const auto& config = world.get_config();

		auto& obj_idx_counter = indices.objects.obj_idx_counter;
		auto& objects = indices.objects.objects;
		auto& player_objects = indices.players.player_objects;

		ensure_stage();

		print("Loading objects...");

		ForEach
		(
			data["objects"],
			
			[&](const auto& obj_cfg)
			{
				const auto obj_type = util::get_value<std::string>(obj_cfg, "type", "");
				//const auto obj_type_id = hash(obj_type);

				Entity entity = null;

				if (obj_type.empty())
				{
					auto tform = get_transform_data(obj_cfg);

					entity = create_pivot(world, tform, stage);
				}
				else
				{
					print("Creating object of type \"{}\"...", obj_type);

					entity = resource_manager.generate_entity
					(
						{
							{
								.instance_path               = util::format("{}.json", obj_type),
								.instance_directory          = root_path,
								.shared_directory            = config.objects.object_path,
								.service_archetype_root_path = (std::filesystem::path(config.entity.archetype_path) / "world"),
								.archetype_root_path         = config.entity.archetype_path
							}
						},

						{
							//world,

							.registry = registry,
							.resource_manager = resource_manager,

							.parent = stage
						}
					);
				}

				if (entity != null)
				{
					process_component_entries
					(
						registry, entity, obj_cfg,
						resource_manager.get_parsing_context(),
						"make_active"
					);

					apply_color(world, entity, obj_cfg);
					apply_transform(world, entity, obj_cfg);

					// TODO: Ensure `tform` doesn't get invalidated by call to `resolve_parent`.
					// (May actually make sense to call `get_matrix` before `resolve_parent` anyway)
					resolve_parent(world, entity, stage, root_path, player_objects, objects, obj_cfg);

					if (auto player_index = util::get_optional<PlayerIndex>(obj_cfg, "player"))
					{
						registry.emplace_or_replace<PlayerTargetComponent>(entity, *player_index);
					}

					if (auto make_active = util::get_value<bool>(obj_cfg, "make_active", false))
					{
						if (const auto* camera = registry.try_get<CameraComponent>(entity))
						{
							world.set_camera(entity);
						}
						else
						{
							print_warn("Failed to set active camera to Entity #{}.", entity);
						}
					}

					auto obj_idx = util::get_value<ObjectIndex>(obj_cfg, "index", obj_idx_counter);

					objects[obj_idx] = entity;

					obj_idx_counter = std::max((obj_idx_counter + 1), (obj_idx + 1));

					print("\"{}\" object created.", obj_type);
				}
			}
		);
	}

	ResourceManager& Stage::Loader::get_resource_manager() const
	{
		return world.get_resource_manager();
	}
}