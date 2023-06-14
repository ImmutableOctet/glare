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
#include <engine/entity/components/static_mutation_component.hpp>

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
	// Internal routine for processing static entity mutations.
	template <typename ...IgnoredKeys>
	static void process_entity_mutations
	(
		Registry& registry, Entity entity, const util::json& object_data,
		const MetaParsingContext& parsing_context,
		Service* opt_service, SystemManagerInterface* opt_system_manager,
		bool capture_static_mutations,
		IgnoredKeys&&... ignored_keys
	)
	{
		StaticMutationComponent* entity_mutation = nullptr;

		auto get_mutation = [&registry, &entity_mutation, entity, capture_static_mutations]() -> StaticMutationComponent&
		{
			assert(capture_static_mutations);

			if (!entity_mutation)
			{
				entity_mutation = &(registry.get_or_emplace<StaticMutationComponent>(entity));
			}

			return *entity_mutation;
		};

		util::enumerate_map_filtered_ex
		(
			object_data.items(),

			[](auto&& value) { return hash(std::forward<decltype(value)>(value)); },

			[&registry, entity, &parsing_context, &opt_service, &opt_system_manager, capture_static_mutations, &get_mutation](const auto& key, const auto& value)
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

					bool component_processed = false;

					if (allow_entry_update)
					{
						component_processed = update_component_fields(registry, entity, *component, true, true, &evaluation_context);
					}

					if (!component_processed)
					{
						component_processed = static_cast<bool>(emplace_component(registry, entity, *component, &evaluation_context));
					}

					if (component_processed)
					{
						if (capture_static_mutations)
						{
							get_mutation().add_component(component->get_type_id());
						}
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
							constexpr auto type_specifier_symbol = std::string_view { ":" };

							const auto variable_declaration = std::string_view { key };
							
							auto variable_name = std::string_view {};

							// NOTE: Global variables always use the direct
							// hash of the variable name as their identifier.
							auto variable_type_name = std::string_view {};

							if (auto variable_type_begin = util::find_singular(variable_declaration, type_specifier_symbol); (variable_type_begin != std::string_view::npos))
							{
								variable_name = util::trim(variable_declaration.substr(0, variable_type_begin));
								variable_type_name = util::trim(variable_declaration.substr(variable_type_begin + type_specifier_symbol.length()));
							}
							else
							{
								variable_name = variable_declaration;
							}

							const auto parsing_instructions = MetaParsingInstructions
							{
								.context                          = parsing_context,

								.fallback_to_string               = true, // false,
										
								.fallback_to_component_reference  = true, // false,
								.fallback_to_entity_reference     = false, // true,

								.allow_member_references          = true,
								.allow_entity_indirection         = true,

								.allow_remote_variable_references = true
							};

							auto variable_out = MetaVariable {};

							if (!variable_type_name.empty())
							{
								const auto type_context = parsing_context.get_type_context();

								auto variable_type = (type_context)
									? type_context->get_type(variable_type_name, parsing_instructions)
									: resolve(hash(variable_type_name).value())
								;

								if (variable_type)
								{
									variable_out = MetaVariable
									{
										variable_name, value,
										variable_type, parsing_instructions
									};
								}
							}

							if (!variable_out)
							{
								variable_out = MetaVariable
								{
									variable_name, value,
									parsing_instructions
								};
							}

							if (variable_out)
							{
								global_variables->set(std::move(variable_out));
							}

							if (capture_static_mutations)
							{
								get_mutation().add_variable
								(
									StaticVariableMutation
									{
										//variable_out.name,
										std::string(variable_name)//,

										//MetaVariableScope::Global
									}
								);
							}
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

	// SceneLoader:
	math::TransformVectors SceneLoader::get_transform_data(const util::json& data)
	{
		auto position = math::Vector {};
		auto rotation = math::Vector {};
		auto scale    = math::Vector { 1.0f, 1.0f, 1.0f };

		//return engine::load<math::TransformVectors>(data);

		if (auto vector_data = data.find("position"); vector_data != data.end())
		{
			engine::load(position, *vector_data);
		}

		if (auto vector_data = data.find("rotation"); vector_data != data.end())
		{
			engine::load(rotation, *vector_data);

			rotation = math::radians(rotation);
		}

		if (auto vector_data = data.find("scale"); vector_data != data.end())
		{
			engine::load(scale, *vector_data);
		}

		return { position, rotation, scale };
	}

	SceneLoader::SceneLoader
	(
		World& world,
		const filesystem::path& root_path,
		Entity scene,
		const Config& cfg,
		SystemManagerInterface* system_manager
	) :
		world(world),
		root_path(root_path),
		scene(scene),
		cfg(cfg),
		system_manager(system_manager)
	{}

	bool SceneLoader::ensure_scene(Entity parent)
	{
		if (scene != null)
		{
			return false;
		}

		if (this->scene == null)
		{
			print("Creating scene pivot...");

			this->scene = create_pivot(world, parent);

			return true;
		}
		
		if (parent != null)
		{
			world.set_parent(this->scene, parent);
		}

		return false;
	}

	Entity SceneLoader::load(const util::json& data, Entity parent)
	{
		ensure_scene(parent);

		return load(data);
	}

	Entity SceneLoader::load(const util::json& data)
	{
		ensure_scene();

		if (cfg.title)
		{
			load_title(data);
		}

		if (cfg.properties)
		{
			if (auto properties = data.find("properties"); properties != data.end())
			{
				load_properties(*properties);
			}
		}

		if (cfg.geometry)
		{
			if (auto models = data.find("models"); models != data.end())
			{
				load_geometry(*models);
			}
		}

		if (cfg.players)
		{
			if (auto players = data.find("players"); players != data.end())
			{
				load_players(*players);
			}
		}

		if (cfg.objects)
		{
			if (auto objects = data.find("objects"); objects != data.end())
			{
				load_objects(*objects);
			}
		}

		// Apply scene transform, etc.
		if (cfg.apply_transform)
		{
			print("Applying scene transform...");

			apply_transform(scene, data);
		}

		return scene;
	}

	void SceneLoader::load_title(const util::json& data)
	{
		ensure_scene();

		if (auto title_entry = data.find("title"); title_entry != data.end())
		{
			if (title_entry->is_string())
			{
				const auto& title = title_entry->get<std::string>();

				world.set_name(scene, std::string_view { title });
			}
		}
	}

	void SceneLoader::load_properties(const util::json& data)
	{
		if (data.empty())
		{
			return;
		}

		//ensure_scene();

		print("Initializing world properties from scene...");

		world.set_properties(engine::load<WorldProperties>(data));
	}

	void SceneLoader::load_geometry(const util::json& data)
	{
		if (data.empty())
		{
			return;
		}

		ensure_scene();

		print("Loading scene geometry...");

		util::json_for_each
		(
			data,
			
			[&](const auto& model_entry)
			{
				const auto model_path = (root_path / model_entry["path"].get<std::string>()).string();

				print("Loading geometry from \"{}\"...\n", model_path);

				const bool collision_enabled = util::get_value(model_entry, "collision", true);

				constexpr auto entity_type = EntityType::Geometry;
				constexpr bool allow_multiple_models = true;

				auto model = load_model
				(
					world, model_path, scene,
					entity_type, allow_multiple_models,
					CollisionConfig(entity_type, collision_enabled)
				);

				print("Applying transformation to scene geometry...");

				apply_transform(model, model_entry);
			}
		);
	}

	void SceneLoader::load_players(const util::json& data)
	{
		if (data.empty())
		{
			return;
		}

		ensure_scene();

		print("Loading players...");

		auto& resource_manager = get_resource_manager();

		auto& registry = world.get_registry();
		const auto& config = world.get_config();

		util::json_for_each
		(
			data,

			[&](const auto& player_cfg)
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

				process_entity_mutations
				(
					registry, player,
					player_cfg,
					resource_manager.get_parsing_context(),
					&world, system_manager,
					cfg.identify_static_mutations,

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

				apply_transform(player, player_cfg);

				apply_state(player, player_cfg);

				player_idx_counter = std::max((player_idx_counter + 1), (player_idx + 1));

				world.event<OnPlayerLoaded>(player, character_path);
			}
		);
	}

	void SceneLoader::load_objects(const util::json& data)
	{
		auto& registry = world.get_registry();
		auto& resource_manager = world.get_resource_manager();

		const auto& config = world.get_config();

		ensure_scene();

		print("Loading objects...");

		util::json_for_each
		(
			data,
			
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

				process_entity_mutations
				(
					registry, entity, obj_cfg,
					resource_manager.get_parsing_context(),
					&world, system_manager,
					cfg.identify_static_mutations,

					"make_active"
				);

				apply_color(entity, obj_cfg);
				apply_transform(entity, obj_cfg);

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

				apply_state(entity, obj_cfg);

				print("\"{}\" object created.", obj_type);
			}
		);
	}

	Entity SceneLoader::entity_reference(std::string_view query) // const
	{
		if (auto target = EntityTarget::parse(query))
		{
			return target->get(world.get_registry(), scene);
		}

		return null;
	}

	Entity SceneLoader::resolve_parent(Entity entity, const util::json& data, bool fallback_to_scene)
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

	ResourceManager& SceneLoader::get_resource_manager() const
	{
		return world.get_resource_manager();
	}

	void SceneLoader::apply_transform(Entity entity, const util::json& cfg)
	{
		auto tform_data = get_transform_data(cfg);

		//world.apply_transform(entity, tform_data);
		world.apply_transform_and_reset_collision(entity, tform_data);
	}

	bool SceneLoader::apply_state(Entity entity, const util::json& data)
	{
		if (const auto& state = util::find_any(data, "state", "default_state"); state != data.end())
		{
			const auto& state_name = state.value().get<std::string>();
			const auto state_id = hash(state_name).value();

			world.queue_event<StateChangeCommand>(entity, entity, state_id); // event

			return true;
		}

		return false;
	}

	std::optional<graphics::ColorRGBA> SceneLoader::apply_color(Entity entity, const util::json& data)
	{
		//auto color = util::get_color(data, "color");

		if (!data.contains("color"))
		{
			return std::nullopt;
		}

		auto& registry = world.get_registry();

		if (auto* model = registry.try_get<ModelComponent>(entity))
		{
			const auto color = util::to_color(data["color"], 1.0f);

			model->color = color;

			return color;
		}

		return std::nullopt;
	}
}