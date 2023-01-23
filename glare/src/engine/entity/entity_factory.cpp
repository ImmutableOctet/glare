#include "entity_factory.hpp"
#include "entity_target.hpp"
#include "entity_state.hpp"
#include "entity_state_rule.hpp"
#include "entity_thread_description.hpp"
#include "entity_thread_builder.hpp"
#include "event_trigger_condition.hpp"

#include "serial_impl.hpp"

#include "components/instance_component.hpp"
#include "components/state_component.hpp"
#include "components/entity_thread_component.hpp"

#include <engine/components/relationship_component.hpp>
#include <engine/components/name_component.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/meta_variable.hpp>
#include <engine/meta/meta_data_member.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>
#include <engine/meta/parsing_context.hpp>

#include <util/algorithm.hpp>
#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/variant.hpp>
#include <util/io.hpp>

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <type_traits>
#include <tuple>
#include <array>
#include <memory>

// TODO: Change to better regular expression library.
#include <regex>

// Debugging related:
#include <util/format.hpp>
#include <util/log.hpp>

#include <engine/components/type_component.hpp>
#include <engine/world/physics/components/collision_component.hpp>

namespace engine
{
	MetaAny EntityFactory::emplace_component
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component
	)
	{
		using namespace entt::literals;

		auto instance = component.instance(true, registry, entity);

		//assert(instance);

		MetaAny result;

		if (instance)
		{
			const auto type = component.get_type();

			if (auto emplace_fn = type.func("emplace_meta_component"_hs))
			{
				result = emplace_fn.invoke
				(
					{},

					entt::forward_as_meta(registry),
					entt::forward_as_meta(entity),
					entt::forward_as_meta(std::move(instance)) // entt::forward_as_meta(instance)
				);
			}

			if (!result)
			{
				print_warn("Failed to attach component: #{} to Entity #{}", component.get_type_id(), entity);
			}
		}
		else
		{
			print_warn("Failed to instantiate component: #{} for Entity #{}", component.get_type_id(), entity);
		}

		return result;
	}

	bool EntityFactory::update_component_fields
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,
		
		bool value_assignment, bool direct_modify
	)
	{
		using namespace entt::literals;

		auto type = component.get_type();

		auto patch_fn = type.func("patch_meta_component"_hs);

		if (!patch_fn)
		{
			print_warn("Unable to resolve patch function for: #{}", type.id());

			return false;
		}

		if (value_assignment && (!direct_modify))
		{
			if (component.size() > 0)
			{
				auto result = patch_fn.invoke
				(
					{},
					entt::forward_as_meta(registry),
					entt::forward_as_meta(entity),
					entt::forward_as_meta(component),
					entt::forward_as_meta(component.size()),
					entt::forward_as_meta(0)
				);

				if (result)
				{
					return (result.cast<std::size_t>() > 0);
				}
			}
		}
		else
		{
			auto get_fn = type.func("get_component"_hs);

			if (!get_fn)
			{
				print_warn("Unable to resolve component-get function from: #{}", type.id());

				return false;
			}

			auto result = get_fn.invoke
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (auto instance = *result) // ; (*result).cast<bool>()
			{
				if (value_assignment) // (&& direct_modify) <-- Already implied due to prior clause.
				{
					if (component.size() > 0)
					{
						return (component.apply_fields(instance) > 0);
					}
				}

				return true;
			}
		}

		return false;
	}

	Entity EntityFactory::create(const EntityConstructionContext& context) const
	{
		auto entity = context.opt_entity_out;

		if (entity == null)
		{
			entity = context.registry.create();
		}

		for (const auto& component : descriptor.components.type_definitions)
		{
			emplace_component(context.registry, entity, component);
		}

		if (context.parent != null)
		{
			RelationshipComponent::set_parent(context.registry, entity, context.parent, true);
		}

		context.registry.emplace_or_replace<InstanceComponent>(entity, paths.instance_path.string());

		if (!context.registry.try_get<NameComponent>(entity))
		{
			auto instance_filename = paths.instance_path.filename(); instance_filename.replace_extension();

			context.registry.emplace<NameComponent>(entity, instance_filename.string());
		}

		if (default_state_index)
		{
			auto result = descriptor.set_state_by_index(context.registry, entity, std::nullopt, *default_state_index);

			if (!result)
			{
				print_warn("Unable to assign default state: Index #{}", *default_state_index);
			}
		}

		if (!descriptor.immediate_threads.empty())
		{
			auto& thread_component = context.registry.get_or_emplace<EntityThreadComponent>(entity);

			for (const auto& thread_range : descriptor.immediate_threads)
			{
				thread_component.start_threads(thread_range);
			}

			// Trigger listeners looking for `EntityThreadComponent` patches.
			context.registry.patch<EntityThreadComponent>(entity);
		}

		return entity;
	}

	std::tuple<std::filesystem::path, std::filesystem::path, util::json>
	EntityFactory::load_state_data(std::string_view state_path_raw, const std::filesystem::path& base_path)
	{
		auto state_path = resolve_reference(std::filesystem::path(state_path_raw), base_path);
		auto state_base_path = state_path.parent_path();

		// TODO: Optimize.
		auto state_data = util::load_json(state_path);

		return { std::move(state_base_path), std::move(state_path), std::move(state_data) };
	}

	std::string EntityFactory::get_embedded_name(const util::json& data)
	{
		if (auto name_it = data.find("name"); name_it != data.end())
		{
			return name_it->get<std::string>();
		}

		return {};
	}

	std::string EntityFactory::default_state_name_from_path(const std::filesystem::path& state_path)
	{
		auto filename = state_path.filename(); filename.replace_extension();

		// An embedded name couldn't be found; use the state's filename instead.
		return filename.string();
	}

	std::string EntityFactory::resolve_state_name(const util::json& state_data, const std::filesystem::path& state_path)
	{
		// Check for a top-level `name` field in the imported state:
		if (auto name = get_embedded_name(state_data); !name.empty())
		{
			// Use the embedded name.
			return name;
		}

		return default_state_name_from_path(state_path);
	}

	const EntityState* EntityFactory::process_state
	(
		EntityDescriptor::StateCollection& states_out,
		std::string_view state_path_raw, // const std::string&
		const std::filesystem::path& base_path,
		const ParsingContext* opt_parsing_context
	)
	{
		auto [state_base_path, state_path, state_data] = load_state_data(state_path_raw, base_path);

		auto state_name = resolve_state_name(state_data, state_path);

		if (const auto state = process_state(states_out, state_data, state_name, state_base_path, opt_parsing_context))
		{
			assert(state->name.has_value());

			return state;
		}

		return nullptr;
	}

	std::size_t EntityFactory::merge_state_list
	(
		EntityDescriptor::StateCollection& states_out,
		EntityState& state,

		const util::json& data,
		const std::filesystem::path& base_path,
		const ParsingContext* opt_parsing_context
	)
	{
		std::size_t count = 0;

		util::json_for_each
		(
			data,

			[this, &states_out, &state, &base_path, opt_parsing_context, &count](const util::json& content)
			{
				switch (content.type())
				{
					case util::json::value_t::string:
					{
						const auto state_path_raw = content.get<std::string>();

						auto [state_base_path, state_path, state_data] = load_state_data(state_path_raw, base_path);

						if (!state_data.empty())
						{
							if (!state.name)
							{
								const auto default_state_name = default_state_name_from_path(state_path);

								state.name = hash(default_state_name);
							}

							if (process_state(states_out, state, state_data, state_base_path, opt_parsing_context))
							{
								count++;
							}
						}

						break;
					}

					default:
						if (process_state(states_out, state, content, base_path, opt_parsing_context))
						{
							count++;
						}

						break;
				}
			}
		);

		return count;
	}

	const EntityState* EntityFactory::process_state
	(
		EntityDescriptor::StateCollection& states_out,
		const util::json& data,
		std::string_view state_name,
		const std::filesystem::path& base_path,
		const ParsingContext* opt_parsing_context
	)
	{
		std::optional<EntityStateID> state_id = std::nullopt;

		if (!state_name.empty())
		{
			state_id = hash(state_name);

			if (auto existing = descriptor.get_state(*state_id))
			{
				print_warn("Overriding existing definition of state: \"{}\"", state_name);

				// Initialize `existing` back to its default state.
				*existing = {};

				// Re-assign the name/ID of this state.
				existing->name = state_id;

				// Execute the main processing routine.
				auto result = process_state(states_out, *existing, data, base_path, opt_parsing_context);

				assert(result);

				return existing;
			}
		}

		// NOTE: State objects are currently heap allocated to ensure a consistent memory location.
		// Likewise, storing multiple `EntityState` objects within a scope may be too bloated for stack usage.
		auto state = std::make_unique<EntityState>();

		// Assign the state's name to the ID we resolved.
		state->name = state_id;

		if (!process_state(states_out, *state, data, base_path, opt_parsing_context))
		{
			return {};
		}

		// Check if `state`'s name has been changed from `state_id`.
		// 
		// NOTE: Control paths from the path-based overload of `process_state`
		// should always provide the state's name ahead of time, so that shouldn't trigger this.
		if (state->name != state_id)
		{
			if (auto existing = descriptor.get_state(state_id))
			{
				// If this additional check for an existing state is triggered, it means a
				// `merge` operation caused the name of a state to be defined,
				// rather than during the initial declaration.
				print_warn("Existing state named \"{}\" (#{}) detected after initial safety checks. -- Perhaps you merged an unnamed state with a named one? (Continuing anyway; replacing existing)", state_name, *state_id);

				*existing = std::move(*state);

				return existing;
			}
			else
			{
				// This path is only reached if the state's name has changed,
				// but there's no existing state with the new name. (No conflict)

				if (state->name && state_id)
				{
					print_warn("State name (#{}) differs from initially resolved value (#{}: \"{}\") -- Maybe caused by a merge operation? (Continuing anyway; no known name conflicts detected.)", *state->name, *state_id, state_name);
				}
				else
				{
					print_warn("State name inherited from initial merge operation. (#{}) (Continuing anyway; no known name conflicts detected)", *state->name);
				}
			}
		}

		//descriptor.set_state(std::move(state));
		//descriptor.states.emplace_back(std::move(state));

		// Store `state` inside of `states_out`.
		const auto& result = states_out.emplace_back(std::move(state));

		// From the stored smart pointer, retrieve a non-owning pointer.
		return result.get();
	}

	bool EntityFactory::process_state
	(
		EntityDescriptor::StateCollection& states_out,
		EntityState& state,
		const util::json& data,
		const std::filesystem::path& base_path,
		const ParsingContext* opt_parsing_context
	)
	{
		using namespace entt::literals;

		if (!state.name)
		{
			if (auto state_name = get_embedded_name(data); !state_name.empty())
			{
				state.name = hash(state_name);
			}
			else
			{
				// NOTE: Because merge operations can take place, we'll refrain
				// from spamming the user, but this warning is still valid.
				//print_warn("Missing name detected while processing state. Nameless states may cause conflicts.");
			}
		}

		// Handle merge operations first.
		// (Allows the contents of `data` to take priority over merged-in definitions):
		if (auto merge = util::find_any(data, "merge", "merge_state", "merge_states", "using"); merge != data.end())
		{
			// NOTE: Recursion by proxy.
			merge_state_list(states_out, state, *merge, base_path, opt_parsing_context);
		}

		// Handle embedded imports of other states before any further operations.
		// (Allows imports in this scope to take priority over imports-within-imports):
		if (auto imports = util::find_any(data, "import", "imports", "state", "states"); imports != data.end())
		{
			// NOTE: Recursion by proxy.
			process_state_list(states_out, *imports, base_path, opt_parsing_context);
		}

		if (auto persist = util::find_any(data, "persist", "share", "shared", "modify", "="); persist != data.end())
		{
			process_component_list(state.components.persist, *persist, {}, opt_parsing_context, true, true, true);
		}

		if (auto add = util::find_any(data, "add", "+"); add != data.end())
		{
			process_component_list(state.components.add, *add, {}, opt_parsing_context, true, true, true);
		}

		if (auto removal_list = util::find_any(data, "remove", "-", "~"); removal_list != data.end())
		{
			state.build_removals(*removal_list, opt_parsing_context);
		}
		
		if (auto frozen_list = util::find_any(data, "frozen", "freeze", "exclude", "%", "^"); frozen_list != data.end())
		{
			state.build_frozen(*frozen_list, opt_parsing_context);
		}

		if (auto storage_list = util::find_any(data, "store", "storage", "include", "temp", "temporary", "#"); storage_list != data.end())
		{
			state.build_storage(*storage_list, opt_parsing_context);
		}

		if (auto isolated = util::find_any(data, "local", "local_storage", "isolate", "isolated"); isolated != data.end())
		{
			process_state_isolated_components(state, *isolated, opt_parsing_context);
		}

		if (auto local_copy = util::find_any(data, "local_copy", "copy"); local_copy != data.end())
		{
			process_state_local_copy_components(state, *local_copy, opt_parsing_context);
		}

		if (auto init_copy = util::find_any(data, "init_copy", "local_modify", "copy_once", "clone"); init_copy != data.end())
		{
			process_state_init_copy_components(state, *init_copy, opt_parsing_context);
		}

		if (auto time_data = util::find_any(data, "timer", "wait", "delay"); time_data != data.end())
		{
			state.activation_delay = parse_time_duration(*time_data);
		}

		if (auto threads = util::find_any(data, "do", "threads", "execute"); threads != data.end())
		{
			if (auto [initial_thread_index, thread_count] = process_thread_list(*threads, &base_path, opt_parsing_context); (thread_count > 0))
			{
				state.immediate_threads.emplace_back(initial_thread_index, thread_count);
			}
		}

		if (auto rules = util::find_any(data, "rule", "rules", "trigger", "triggers"); rules != data.end())
		{
			process_state_rule_list(state, *rules, &states_out, &base_path, opt_parsing_context);
		}

		return true;
	}

	std::size_t EntityFactory::process_state_list
	(
		EntityDescriptor::StateCollection& states_out,
		const util::json& data,
		const std::filesystem::path& base_path,
		const ParsingContext* opt_parsing_context
	)
	{
		std::size_t count = 0;

		util::json_for_each(data, [this, &base_path, &states_out, &opt_parsing_context, &count](const util::json& state_entry)
		{
			switch (state_entry.type())
			{
				case util::json::value_t::object:
				{
					// NOTE: Embedded state definitions must have a `name` field.
					const auto state_name = util::get_value<std::string>(state_entry, "name");

					if (process_state(states_out, state_entry, state_name, base_path, opt_parsing_context))
					{
						count++;
					}

					break;
				}
				case util::json::value_t::string:
				{
					const auto state_path_raw = state_entry.get<std::string>();

					if (process_state(states_out, state_path_raw, base_path, opt_parsing_context))
					{
						count++;
					}

					break;
				}
			}
		});

		return count;
	}

	std::size_t EntityFactory::process_state_isolated_components(EntityState& state, const util::json& isolated, const ParsingContext* opt_parsing_context)
	{
		return process_and_inspect_component_list
		(
			state.components.add,

			isolated,

			[&state, &opt_parsing_context](std::string_view component_name)
			{
				if (!state.process_type_list_entry(state.components.freeze, component_name, true, opt_parsing_context)) // false
				{
					print_warn("Failed to process embedded `freeze` entry from isolation data.");

					return false;
				}

				if (!state.process_type_list_entry(state.components.store, component_name, true, opt_parsing_context)) // false
				{
					print_warn("Failed to process embedded `store` entry from isolation data.");

					return false;
				}

				return true;
			},

			false,
			{},
			opt_parsing_context
		);
	}

	std::size_t EntityFactory::process_state_local_copy_components(EntityState& state, const util::json& local_copy, const ParsingContext* opt_parsing_context)
	{
		const auto build_result = state.build_local_copy(local_copy, opt_parsing_context);

		const auto components_processed = process_component_list
		(
			state.components.add, local_copy,
			{
				.allow_default_construction             = false,
				.allow_forwarding_fields_to_constructor = false,
				.force_field_assignment                 = true
			},

			opt_parsing_context,

			true, false
		);

		assert(components_processed == build_result);

		return build_result;
	}

	std::size_t EntityFactory::process_state_init_copy_components(EntityState& state, const util::json& init_copy, const ParsingContext* opt_parsing_context)
	{
		const auto build_result = state.build_init_copy(init_copy, opt_parsing_context);

		const auto components_processed = process_component_list
		(
			state.components.add, init_copy,
			{
				.allow_default_construction             = false,
				.allow_forwarding_fields_to_constructor = false,
				.force_field_assignment                 = true
			},

			opt_parsing_context,

			true, false
		);

		assert(components_processed == build_result);

		return build_result;
	}

	std::size_t EntityFactory::process_state_rule
	(
		EntityState& state,
		MetaTypeID type_name_id,
		const util::json& content,
		
		std::optional<EventTriggerCondition> condition,

		EntityDescriptor::StateCollection* opt_states_out,
		const std::filesystem::path* opt_base_path,
		const ParsingContext* opt_parsing_context,

		bool allow_inline_import
	)
	{
		std::size_t count = 0;

		EntityTarget::TargetType target = EntityTarget::SelfTarget {};
		std::optional<Timer::Duration> delay = std::nullopt;

		// NOTE: Modified below, used in `process_transition_rule` and co.
		bool forward_condition = true;

		if (condition)
		{
			// Handle special-case scenarios: direct usage of `EventTriggerTrueCondition` and `EventTriggerFalseCondition`.
			switch (condition->index())
			{
				case util::variant_index<EventTriggerCondition, EventTriggerTrueCondition>():
					//print("Pure 'true-condition' detected; generating rule without condition.");

					// Don't bother forwarding this condition since it's guaranteed to always report true.
					forward_condition = false;

					break;
				case util::variant_index<EventTriggerCondition, EventTriggerFalseCondition>():
					print_warn("Pure 'false-condition' found outside of compound condition; ignoring rule generation.");

					// Exit early; trigger condition will never be met.
					return count; // 0;
			}
		}

		auto& rules_out = state.rules[type_name_id];

		auto process_rule = [&count, &rules_out, forward_condition, &condition, &delay, &target](auto&& action)
		{
			auto& rule = rules_out.emplace_back
			(
				(forward_condition)
				? condition // std::move(condition)
				: std::optional<EventTriggerCondition>(std::nullopt),

				delay, // std::move(delay),
				EntityTarget(target), // std::move(target)

				EntityStateAction(std::move(action))
			);

			count++;

			return &rule;
		};

		auto process_transition_rule = [&](const auto& next_state_raw) -> EntityStateRule*
		{
			if (next_state_raw.empty())
			{
				return nullptr;
			}

			// TODO: Look into tracking imports/inline-imports.
			const auto inline_import = process_state_inline_import(opt_states_out, next_state_raw, opt_base_path, opt_parsing_context);

			StringHash next_state_id = (inline_import) // EntityStateHash
				? *inline_import->name
				: hash(next_state_raw)
			;

			return process_rule
			(
				EntityStateTransitionAction
				{
					.state_name = next_state_id
				}
			);
		};

		auto process_command = [this, &process_rule](CommandContent& content) // CommandContent&& content
		{
			return process_rule
			(
				EntityStateCommandAction
				{
					descriptor.shared_storage.get_index_unsafe(content) // std::move(content)
				}
			);
		};

		auto init_empty_command = [this, opt_parsing_context](std::string_view command_name) -> CommandContent*
		{
			if (command_name.empty())
			{
				return {};
			}

			const auto command_name_id = hash(command_name);
			const auto command_type = resolve(command_name_id);

			if (command_type)
			{
				return &(descriptor.generate_empty_command(command_type));
			}
			else
			{
				if (opt_parsing_context)
				{
					if (auto resolved_command_type = opt_parsing_context->get_command_type_from_alias(command_name))
					{
						return &(descriptor.generate_empty_command(resolved_command_type));
					}
				}

				print_warn("Unable to resolve command name: {} (#{})", command_name, command_name_id.value());
			}

			return {};
		};

		auto process_command_rule_from_csv = [&init_empty_command, &process_command](std::string_view command_name, std::string_view args, bool resolve_values=true) -> EntityStateRule*
		{
			if (args.empty())
			{
				return {};
			}

			if (auto command_content = init_empty_command(command_name))
			{
				command_content->set_variables
				(
					args,
					
					{
						.resolve_symbol=resolve_values,
						.resolve_component_member_references=resolve_values
					},
					
					",", command_arg_offset
				);

				return process_command(*command_content); // std::move(*command_content)
			}

			return {};
		};

		auto process_command_rule_from_expr = [&process_command_rule_from_csv](const auto& command_expr) -> EntityStateRule*
		{
			auto [command_name, command_content, trailing_expr, is_string_content] = util::parse_single_argument_command(command_expr);

			return process_command_rule_from_csv(command_name, command_content, !is_string_content);
		};

		auto process_command_rule_from_object = [&opt_parsing_context, &init_empty_command, &process_command](std::string_view command_name, const util::json& command_data) -> EntityStateRule*
		{
			if (auto command_content = init_empty_command(command_name))
			{
				command_content->set_variables
				(
					command_data,
					{ .resolve_component_member_references = true },
					command_arg_offset,
					opt_parsing_context
				);

				return process_command(*command_content); // std::move(*command_content)
			}

			return {};
		};

		auto for_each_command = [&process_command_rule_from_object, &process_command_rule_from_expr, &process_command_rule_from_csv](const util::json& command)
		{
			util::json_for_each<util::json::value_t::string, util::json::value_t::object>
			(
				command,

				[&process_command_rule_from_object, &process_command_rule_from_expr, &process_command_rule_from_csv](const util::json& command_content)
				{
					switch (command_content.type())
					{
						case util::json::value_t::object:
						{
							if (auto command_name_it = util::find_any(command_content, "command", "name", "type"); command_name_it != command_content.end())
							{
								const auto command_name = command_name_it->get<std::string>();

								process_command_rule_from_object(command_name, command_content);
							}
							else
							{
								// TODO: Refactor to share code with `enumerate_map_filtered_ex` section.
								for (const auto& [command_name, embedded_content] : command_content.items())
								{
									switch (embedded_content.type())
									{
										case util::json::value_t::object:
										{
											process_command_rule_from_object(command_name, embedded_content);

											break;
										}

										case util::json::value_t::string:
										{
											auto string_data = embedded_content.get<std::string>();

											process_command_rule_from_csv(command_name, string_data);

											break;
										}
									}
								}
							}

							break;
						}

						case util::json::value_t::string:
							process_command_rule_from_expr(command_content.get<std::string>());

							break;
					}
				}
			);
		};

		switch (content.type())
		{
			case util::json::value_t::string:
			{
				process_transition_rule(content.get<std::string>());

				break;
			}

			case util::json::value_t::array:
			{
				for_each_command(content);

				break;
			}

			case util::json::value_t::object:
			{
				if (auto target_data = util::find_any(content, "target"); target_data != content.end())
				{
					target = parse_target_type(*target_data);
				}

				if (auto time_data = util::find_any(content, "timer", "wait", "delay"); time_data != content.end())
				{
					delay = parse_time_duration(*time_data);
				}

				if (auto next_state = util::find_any(content, "state", "next", "next_state"); next_state != content.end())
				{
					process_transition_rule(next_state->get<std::string>());
				}

				if (auto update = util::find_any(content, "update", "updates", "components", "change"); update != content.end())
				{
					util::json_for_each
					(
						*update,

						[this, &opt_parsing_context, &process_rule, &target](const util::json& update_entry)
						{
							auto action = EntityStateUpdateAction
							{
								std::make_unique<EntityStateUpdateAction::ComponentStore>(),
								target
							};

							process_update_action(action, update_entry, opt_parsing_context);

							process_rule(std::move(action));
						}
					);
				}

				if (auto threads = util::find_any(content, "do", "threads", "execute"); threads != content.end())
				{
					auto [initial_thread_index, thread_count] = process_thread_list(*threads, opt_base_path, opt_parsing_context);

					process_rule
					(
						EntityThreadSpawnAction
						{
							EntityThreadRange
							{
								initial_thread_index,
								thread_count
							},
							
							false
						}
					);
				}

				// TODO: Implement sections for thread control-flow actions.

				if (auto command = util::find_any(content, "command", "commands", "generate"); command != content.end())
				{
					for_each_command(*command);

					break;
				}
				else
				{
					util::enumerate_map_filtered_ex
					(
						content.items(),
						hash,

						[&process_command_rule_from_object, &process_command_rule_from_csv](const auto& command_name, const auto& content)
						{
							switch (content.type())
							{
								case util::json::value_t::array:
								case util::json::value_t::object:
								{
									process_command_rule_from_object(command_name, content);

									break;
								}

								case util::json::value_t::string:
								{
									auto string_data = content.get<std::string>();

									process_command_rule_from_csv(command_name, string_data);

									break;
								}
							}
						},

						// Ignore these keys (see above):
						"trigger", "triggers", "condition", "conditions",

						"target",
						"timer", "wait", "delay",
						"state", "next", "next_state",
						"update", "updates", "components", "change",
						"do", "threads", "execute",
						"command", "commands", "generate"
					);
				}

				break;
			}
		}

		return count;
	}

	std::size_t EntityFactory::process_trigger_expression
	(
		EntityState& state,

		const std::string& trigger_condition_expr, // std::string_view
		const util::json& content,

		EntityDescriptor::StateCollection* opt_states_out,
		const std::filesystem::path* opt_base_path,
		const ParsingContext* opt_parsing_context,
		bool allow_inline_import
	)
	{
		std::size_t number_processed = 0;

		engine::process_trigger_expression
		(
			trigger_condition_expr,

			[this, &state, &content, opt_states_out, opt_base_path, opt_parsing_context, allow_inline_import, &number_processed]
			(
				MetaTypeID type_name_id,
				std::optional<EventTriggerCondition> condition
			)
			{
				const auto result = this->process_state_rule
				(
					state,
					type_name_id, content, condition,
					opt_states_out, opt_base_path, opt_parsing_context,
					allow_inline_import
				);
				
				number_processed += result;

				return result;
			},

			false, // true,
			opt_parsing_context
		);

		// NOTE: Updated automatically in `process_rule` subroutine.
		return number_processed;
	}

	std::size_t EntityFactory::process_state_rule_list
	(
		EntityState& state,
		const util::json& rules,

		EntityDescriptor::StateCollection* opt_states_out,
		const std::filesystem::path* opt_base_path,
		const ParsingContext* opt_parsing_context,
		bool allow_inline_import
	)
	{
		std::size_t count = 0;

		const auto collection_has_named_members = (rules.is_object());

		constexpr auto trigger_symbols = std::array { "trigger", "triggers", "condition", "conditions" };

		auto get_trigger_object = [&](const util::json& content)
		{
			return std::apply
			(
				[&](auto&&... values)
				{
					return util::find_any(content, std::forward<decltype(values)>(values)...);
				},

				trigger_symbols
			);
		};

		// Handles the "rules": [{}, {}, {}] (object array) syntax.
		auto from_json_object = [&](const util::json& content)
		{
			if (const auto trigger_decl = get_trigger_object(content); trigger_decl != content.end())
			{
				util::json_for_each(*trigger_decl, [&](const util::json& decl)
				{
					// Embedded (i.e. regular) trigger condition.
					const auto trigger_condition_expr = decl.get<std::string>();

					count += process_trigger_expression
					(
						state, trigger_condition_expr, content,
						opt_states_out, opt_base_path, opt_parsing_context,
						allow_inline_import
					);
				});
			}
		};

		// Handles the '"TriggerCondition": { actions }' syntax.
		auto from_simplified = [&](const auto& trigger_condition_expr, const util::json& content)
		{
			count += process_trigger_expression
			(
				state, trigger_condition_expr, content,
				opt_states_out, opt_base_path, opt_parsing_context,
				allow_inline_import
			);
		};

		auto from_data = [&](const auto& declaration, const auto& content)
		{
			if ((!collection_has_named_members) || declaration.empty())
			{
				from_json_object(content);
			}
			else
			{
				// In-place trigger condition.
				const auto& trigger_condition_expr = declaration;

				from_simplified(trigger_condition_expr, content);
			}
		};

		auto for_each = [&](const util::json& rule_container)
		{
			for (const auto& proxy : rule_container.items())
			{
				const auto& declaration = proxy.key();
				const auto& content = proxy.value();

				from_data(declaration, content);
			}
		};

		// Same as `for_each`, but ignores the `trigger` field.
		auto for_each_ignore_trigger = [&](const util::json& rule_container)
		{
			return std::apply
			(
				[&](auto&&... values)
				{
					util::enumerate_map_filtered(rule_container.items(), from_data, std::forward<decltype(values)>(values)...);
				},

				trigger_symbols
			);
		};

		switch (rules.type())
		{
			case util::json::value_t::object:
			{
				// Check if a trigger was specified.
				if (auto trigger = get_trigger_object(rules); trigger != rules.end())
				{
					switch (trigger->type())
					{
						case util::json::value_t::string:
						{
							// Because we have a `trigger` field in the top-level definition,
							// and that trigger is a conditional expression, we can assume
							// that the user wanted a single rule, rather than multiple.
							const auto trigger_condition_expr = rules.get<std::string>();

							// Assume the content for this trigger is stored in `rules`.
							from_simplified(trigger_condition_expr, rules);

							break;
						}
						default:
						{
							// Similar to the above, we have a top-level `trigger` field.
							// Unlike that path, however, we do have explicitly defined content available.
							if (auto embedded_trigger = get_trigger_object(*trigger); embedded_trigger != trigger->end())
							{
								switch (embedded_trigger->type())
								{
									case util::json::value_t::string:
										from_simplified(embedded_trigger->get<std::string>(), *trigger);

										break;
									default:
										for_each(*embedded_trigger);

										break;
								}
							}

							// Since we're looking at an encapsulated `trigger` object, the rest of
							// the top-level entries (if any) should be fair game.
							for_each_ignore_trigger(rules);

							break;
						}
					}
				}
				else
				{
					for_each(rules);
				}

				break;
			}
			default:
			{
				for_each(rules);

				break;
			}
		}

		return count;
	}

	const EntityState* EntityFactory::process_state_inline_import
	(
		EntityDescriptor::StateCollection* states_out,
		const std::string& command, // std::string_view
		const std::filesystem::path* base_path,
		const ParsingContext* opt_parsing_context,
		bool allow_inline_import
	)
	{
		using namespace entt::literals;

		if (!allow_inline_import)
		{
			return nullptr;
		}

		if (!states_out)
		{
			return nullptr;
		}

		if (!base_path)
		{
			return nullptr;
		}

		auto [inline_command, state_path_raw, trailing_expr, is_string_content] = util::parse_single_argument_command(command);

		if (state_path_raw.empty())
		{
			return nullptr;
		}

		switch (hash(inline_command))
		{
			case "import"_hs:
				// NOTE: Recursion.
				return process_state(*states_out, state_path_raw, *base_path, opt_parsing_context);
		}

		return nullptr;
	}

	void EntityFactory::process_archetype
	(
		const util::json& data,
		const std::filesystem::path& base_path,
		const ParsingContext* opt_parsing_context,
		bool resolve_external_modules
	)
	{
		if (resolve_external_modules)
		{
			// Handles the following: "archetypes", "import", "imports", "modules"
			resolve_archetypes(data, base_path, opt_parsing_context);
		}

		if (auto components = util::find_any(data, "component", "components"); components != data.end())
		{
			process_component_list(descriptor.components, *components, {}, opt_parsing_context);
		}

		if (auto states = util::find_any(data, "state", "states"); states != data.end())
		{
			process_state_list(descriptor.states, *states, base_path, opt_parsing_context);
		}

		if (auto threads = util::find_any(data, "do", "threads", "execute"); threads != data.end())
		{
			if (auto [initial_thread_index, thread_count] = process_thread_list(*threads, &base_path, opt_parsing_context); (thread_count > 0))
			{
				descriptor.immediate_threads.emplace_back(initial_thread_index, thread_count);
			}
		}

		if (auto default_state = data.find("default_state"); default_state != data.end())
		{
			switch (default_state->type())
			{
				case util::json::value_t::string:
				{
					auto state_name = default_state->get<std::string>();
					auto state_name_hash = hash(state_name).value();

					if (auto index = descriptor.get_state_index(state_name_hash))
					{
						default_state_index = *index;
					}

					break;
				}

				case util::json::value_t::number_integer:
				case util::json::value_t::number_unsigned:
				{
					auto index = default_state->get<EntityStateIndex>();

					if (index < descriptor.states.size())
					{
						default_state_index = index;
					}

					break;
				}
			}
		}

		// Handle every other key-value pair as a component:
		util::enumerate_map_filtered_ex
		(
			data.items(),
			hash,

			[this, &opt_parsing_context](const auto& component_declaration, const auto& component_content)
			{
				process_component
				(
					descriptor.components,

					component_declaration,
					&component_content,
					{},
					opt_parsing_context
				);
			},

			// Ignore these keys:

			// Handled in `resolve_archetypes` routine.
			"archetypes", "import", "imports", "modules",

			// Handled in this function (see above):
			"component", "components",
			"state", "states",
			"do", "threads", "execute",
			"default_state",

			// Handled in callback-based implementation of `process_archetype`.
			"children"
		);
	}

	//std::size_t
	std::tuple<EntityThreadIndex, EntityThreadCount>
	EntityFactory::process_thread_list
	(
		const util::json& content,
		const std::filesystem::path* opt_base_path,
		const ParsingContext* opt_parsing_context
	)
	{
		const auto initial_thread_index = descriptor.get_next_thread_index();

		auto process_thread = [this, opt_base_path, opt_parsing_context]
		(
			const util::json& content,
			std::string_view opt_thread_name
		)
		{
			auto thread_builder = EntityThreadBuilder
			{
				descriptor,
				
				opt_thread_name,

				this,
				opt_base_path,
				opt_parsing_context
			};

			thread_builder << content;

			//return thread_builder;
		};

		switch (content.type())
		{
			case util::json::value_t::array:
			{
				util::json_for_each
				(
					content,

					[&process_thread](const util::json& thread_content)
					{
						process_thread(thread_content, {});
					}
				);

				break;
			}

			case util::json::value_t::object:
			{
				for (const auto& proxy : content.items())
				{
					const auto& thread_name = proxy.key();
					const auto& thread_content = proxy.value();

					process_thread(thread_content, thread_name);
				}

				break;
			}

			default:
			{
				process_thread(content, {});

				break;
			}
		}

		const auto processed_count = static_cast<EntityThreadCount>(descriptor.get_next_thread_index() - initial_thread_index);

		return { initial_thread_index, processed_count };
	}
}