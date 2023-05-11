#include "scene.hpp"

#include "world.hpp"
#include "world_events.hpp"
#include "entity.hpp"
#include "graphics_entity.hpp"

#include "camera/components/camera_component.hpp"
#include "physics/components/collision_component.hpp"
#include "behaviors/simple_follow_behavior.hpp"

// TODO: Revisit whether debug functionality should be included here.
#include "behaviors/debug_move_behavior.hpp"

#include <engine/config.hpp>

#include <engine/meta/hash.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/component.hpp>
#include <engine/meta/meta_evaluation_context.hpp>
#include <engine/meta/meta_variable.hpp>

#include <engine/resource_manager/resource_manager.hpp>

#include <engine/entity/serial.hpp>
#include <engine/entity/entity_target.hpp>
#include <engine/entity/entity_factory.hpp>
#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/entity_construction_context.hpp>

#include <engine/entity/components/entity_thread_component.hpp>
#include <engine/entity/components/instance_component.hpp>

#include <engine/entity/commands/state_change_command.hpp>

#include <engine/components/name_component.hpp>
#include <engine/components/model_component.hpp>
#include <engine/components/player_component.hpp>
#include <engine/components/player_target_component.hpp>

#include <engine/input/components/input_component.hpp>

#include <math/math.hpp>

#include <util/json.hpp>
#include <util/log.hpp>
#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/format.hpp>
#include <util/algorithm.hpp>

#include <string>
#include <regex>

namespace engine
{
	// Scene:
	template <typename ...IgnoredKeys>
	void Scene::process_data_entries
	(
		Registry& registry, Entity entity, const util::json& object_data,
		const MetaParsingContext& parsing_context,
		Service* opt_service, SystemManagerInterface* opt_system_manager,
		IgnoredKeys&&... ignored_keys
	)
	{
		util::enumerate_map_filtered_ex
		(
			object_data.items(),

			[](auto&& value) { return hash(std::forward<decltype(value)>(value)); },

			[&registry, entity, &parsing_context, &opt_service, &opt_system_manager](const auto& key, const auto& value)
			{
				auto [component_name, allow_entry_update, constructor_arg_count] = parse_component_declaration(key, true);

				if (auto component = process_component(component_name, &value, constructor_arg_count, {}, parsing_context))
				{
					auto evaluation_context = MetaEvaluationContext
					{
						.variable_context = {},
						.service          = opt_service,
						.system_manager   = opt_system_manager
					};

					if (!allow_entry_update || !update_component_fields(registry, entity, *component, true, true, &evaluation_context))
					{
						emplace_component(registry, entity, *component, &evaluation_context);
					}
				}
				else
				{
					// Ensure that this is an entity that can take advantage of thread variables.
					if (registry.try_get<InstanceComponent>(entity))
					{
						auto& thread_component = registry.get_or_emplace<EntityThreadComponent>(entity);

						if (auto* global_variables = thread_component.get_global_variables())
						{
							global_variables->set
							(
								MetaVariable
								{
									// NOTE: Global variables always use the direct
									// hash of the variable name as their identifier.
									key, value,

									MetaParsingInstructions
									{
										.context                          = parsing_context,

										.fallback_to_string               = true, // false,
										
										.fallback_to_component_reference  = true, // false,
										.fallback_to_entity_reference     = false, // true,

										.allow_member_references          = true,
										.allow_entity_indirection         = true,

										.allow_remote_variable_references = true
									}
								}
							);
						}
					}
				}
			},

			// Treat every object entry (excluding these keys) as component declarations:

			// Transformation attributes:
			"position", "rotation", "scale",

			// General:
			"type", "player", "index", "parent", "color", // "target",

			"state", "default_state",

			std::forward<IgnoredKeys>(ignored_keys)...
		);
	}

    Entity Scene::Load
	(
		World& world,
		const filesystem::path& root_path,
		const util::json& data,
		Entity parent,
		SystemManagerInterface* opt_system_manager,
		const Scene::Loader::Config& cfg
	)
    {
		auto state = Scene::Loader(world, root_path, data, null, opt_system_manager);

		return state.load(cfg, parent);
    }

	math::TransformVectors Scene::get_transform_data(const util::json& cfg)
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

	void Scene::apply_transform(World& world, Entity entity, const util::json& cfg)
	{
		auto tform_data = get_transform_data(cfg);

		//world.apply_transform(entity, tform_data);
		world.apply_transform_and_reset_collision(entity, tform_data);
	}

	bool Scene::apply_state(World& world, Entity entity, const util::json& cfg)
	{
		if (const auto& state = util::find_any(cfg, "state", "default_state"); state != cfg.end())
		{
			const auto& state_name = state.value().get<std::string>();
			const auto state_id = hash(state_name).value();

			world.queue_event<StateChangeCommand>(entity, entity, state_id); // event

			return true;
		}

		return false;
	}

	std::optional<graphics::ColorRGBA> Scene::apply_color(World& world, Entity entity, const util::json& cfg)
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

	// Scene::Loader:
	Entity Scene::Loader::make_scene_pivot(World& world, Entity parent)
	{
		//parent = create_pivot(world, parent);
		auto scene = create_pivot(world, parent);

		return scene;
	}

	Scene::Loader::Loader
	(
		World& world,
		const filesystem::path& root_path,
		const util::json& data,
		Entity scene,
		SystemManagerInterface* system_manager
	) :
		world(world),
		root_path(root_path),
		data(data),
		scene(scene),
		system_manager(system_manager)
	{}

	bool Scene::Loader::ensure_scene(Entity parent)
	{
		if (scene != null)
		{
			return false;
		}

		// Automatically generated scene pivot:
		print("Creating scene pivot...");

		scene = make_scene_pivot(world, parent);

		return true;
	}

	Entity Scene::Loader::load(const Scene::Loader::Config& cfg, Entity parent)
	{
		auto scene = this->scene;

		if (scene == null)
		{
			scene = make_scene_pivot(world, parent);
		}

		return load(scene, cfg, parent);
	}

	Entity Scene::Loader::load(Entity scene, const Scene::Loader::Config& cfg, Entity parent, bool load_title)
	{
		assert(scene != null);

		this->scene = scene;

		load_properties((scene == this->scene));

		// Scene geometry:
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

		// Apply scene transform, etc.
		if (cfg.apply_transform)
		{
			print("Applying scene transform...");

			apply_transform(world, scene, data);
		}

		return scene;
	}

	void Scene::Loader::load_properties(bool load_title, const std::string& default_title)
	{
		if ((load_title) && (scene != null))
		{
			world.set_name(scene, util::get_value<std::string>(data, "title", util::get_value<std::string>(data, "name", default_title)));
		}

		if (auto properties = data.find("properties"); properties != data.end())
		{
			print("Initializing scene properties...");

			world.set_properties(engine::load<WorldProperties>(*properties));
		}
	}

	void Scene::Loader::load_geometry()
	{
		ensure_scene();

		print("Loading scene geometry...");

		ForEach(data["models"], [&](const auto& model_cfg)
		{
			auto model_path = (root_path / model_cfg["path"].get<std::string>()).string();

			bool collision_enabled = util::get_value(model_cfg, "collision", true);

			print("Loading geometry from \"{}\"...\n", model_path);

			auto type = EntityType::Geometry;
			
			auto model = load_model(world, model_path, scene, type, true, CollisionConfig(type, collision_enabled));

			print("Applying transformation to scene geometry...");

			apply_transform(world, model, model_cfg);
		});
	}

	void Scene::Loader::load_players()
	{
		ensure_scene();

		print("Loading players...");

		auto& resource_manager = get_resource_manager();

		auto& registry = world.get_registry();
		const auto& config = world.get_config();

		ForEach(data["players"], [&](const auto& player_cfg)
		{
			auto player_character = util::get_value<std::string>(player_cfg, "character", config.players.default_player.character);

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
					.registry = registry,
					.resource_manager = resource_manager,

					.parent = scene,

					.opt_entity_out = null,

					.opt_service = &world,
					.opt_system_manager = system_manager
				}
			);

			resolve_parent(player, player_cfg, false);

			//assert(player != null);

			if (player == null)
			{
				print_warn("Failed to create player object for character: \"{}\"", player_character);

				return;
			}

			process_data_entries
			(
				registry, player,
				player_cfg,
				resource_manager.get_parsing_context(),
				&world, system_manager,

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

			apply_transform(world, player, player_cfg);

			apply_state(world, player, player_cfg);

			player_idx_counter = std::max((player_idx_counter + 1), (player_idx + 1));

			world.event<OnPlayerLoaded>(player, character_path);
		});
	}

	void Scene::Loader::load_objects()
	{
		auto& registry = world.get_registry();
		auto& resource_manager = world.get_resource_manager();

		const auto& config = world.get_config();

		ensure_scene();

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

					entity = create_pivot(world, tform, scene);
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
							.registry = registry,
							.resource_manager = resource_manager,

							.parent = scene,

							.opt_entity_out = null,

							.opt_service = &world,
							.opt_system_manager = system_manager
						}
					);
				}

				if (entity == null)
				{
					print_warn("Failed to create object of type: \"{}\"", obj_type);

					return;
				}

				// TODO: Ensure `tform` doesn't get invalidated by call to `resolve_parent`.
				// (May actually make sense to call `get_matrix` before `resolve_parent` anyway)
				resolve_parent(entity, obj_cfg);

				process_data_entries
				(
					registry, entity, obj_cfg,
					resource_manager.get_parsing_context(),
					&world, system_manager,
					"make_active"
				);

				apply_color(world, entity, obj_cfg);
				apply_transform(world, entity, obj_cfg);

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

				apply_state(world, entity, obj_cfg);

				print("\"{}\" object created.", obj_type);
			}
		);
	}

	Entity Scene::Loader::entity_reference(std::string_view query) // const
	{
		if (auto target = EntityTarget::parse(query))
		{
			return target->get(world.get_registry(), scene);
		}

		return null;
	}

	Entity Scene::Loader::resolve_parent(Entity entity, const util::json& data, bool fallback_to_scene)
	{
		const auto parent_query = util::get_value<std::string>(data, "parent"); // data["parent"].get<std::string>();

		auto parent = entity_reference(parent_query);

		if ((parent == null) && (fallback_to_scene))
		{
			parent = scene;
		}

		if (parent != null)
		{
			world.set_parent(entity, parent);
		}

		return parent;
	}

	ResourceManager& Scene::Loader::get_resource_manager() const
	{
		return world.get_resource_manager();
	}
}