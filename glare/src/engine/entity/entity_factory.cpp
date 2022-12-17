#include "entity_factory.hpp"
#include "entity_target.hpp"
#include "event_trigger_condition.hpp"

#include <engine/components/relationship_component.hpp>
#include <engine/components/instance_component.hpp>
#include <engine/components/name_component.hpp>

#include <engine/state/components/state_component.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/meta_data_member.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>

#include <util/algorithm.hpp>
#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/variant.hpp>

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <type_traits>
#include <tuple>
#include <array>

// TODO: Change to better regular expression library.
#include <regex>

// Debugging related:
#include <util/format.hpp>
#include <util/log.hpp>
#include <engine/components/type_component.hpp>

namespace engine
{
	std::filesystem::path EntityFactory::resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		auto get_surface_path = [this, &base_path](const auto& path) -> std::filesystem::path
		{
			if (!base_path.empty())
			{
				if (auto projected_path = (base_path / path); std::filesystem::exists(projected_path))
				{
					return projected_path;
				}
			}

			if (auto projected_path = (paths.instance_directory / path); std::filesystem::exists(projected_path))
			{
				return projected_path;
			}

			if (!paths.service_archetype_root_path.empty())
			{
				if (auto projected_path = (paths.service_archetype_root_path / path); std::filesystem::exists(projected_path))
				{
					return projected_path;
				}
			}

			if (auto projected_path = (paths.archetype_root_path / path); std::filesystem::exists(projected_path))
			{
				return projected_path;
			}

			return {};
		};

		auto module_path = get_surface_path(path);

		if (module_path.empty())
		{
			auto direct_path = path; direct_path.replace_extension("json");

			module_path = get_surface_path(direct_path);

			if (module_path.empty())
			{
				return {};
			}
		}
		else if (std::filesystem::is_directory(module_path))
		{
			const auto module_name = module_path.filename();

			return (module_path / module_name).replace_extension("json");
		}

		return module_path;
	}

	std::tuple<std::string_view, bool, std::optional<EntityFactory::SmallSize>>
	EntityFactory::parse_component_declaration(const std::string& component_declaration) // std::string_view
	{
		const auto component_rgx = std::regex("(\\+)?([^\\|]+)(\\|(\\d+))?");

		std::string_view name_view = {};
		bool inplace_changes = false;
		std::optional<SmallSize> constructor_arg_count = std::nullopt;

		if (std::smatch rgx_match; std::regex_search(component_declaration.begin(), component_declaration.end(), rgx_match, component_rgx))
		{
			if (rgx_match[1].matched)
			{
				inplace_changes = true;
			}

			name_view = util::match_view(component_declaration, rgx_match, 2);

			if (rgx_match[4].matched)
			{
				SmallSize arg_count = 0;

				auto begin = (component_declaration.data() + rgx_match.position(4));
				auto end = (begin + rgx_match.length(4));

				auto result = std::from_chars(begin, end, arg_count);

				if (result.ec == static_cast<std::errc>(0)) // != std::errc::invalid_argument
				{
					constructor_arg_count = arg_count;
				}
			}
		}

		return { name_view, inplace_changes, constructor_arg_count };
	}

	std::tuple<std::string_view, std::string_view, std::string_view, std::string_view, std::string_view, std::ptrdiff_t>
	EntityFactory::parse_trigger_condition(const std::string& trigger_condition, std::ptrdiff_t offset) // std::string_view
	{
		//const auto trigger_rgx = std::regex("([^\\s\\:\\.\\-\\>\\&\\|\\(\\)]+)\\s*(\\:\\:|\\.|\\-\\>)?\\s*([^\\s\\(\\)\\|\\&]+)?\\s*(==|===|!=|!==|\\|)\\s*([^\\&\\|]+)"); // \\s* // \\(\\)
		const auto trigger_rgx = std::regex("([^\\s\\:\\.\\-\\>\\&\\|\\(\\)]+\\([^\\)]+\\)|self|this)?(\\:\\:|\\.|\\-\\>)?([^\\s\\:\\.\\-\\>\\&\\|\\(\\)]+)\\s*(\\:\\:|\\.|\\-\\>)?\\s*([^\\s\\(\\)\\|\\&]+)?\\s*(==|===|!=|!==|\\|)\\s*([^\\&\\|]+)"); // \\s* // \\(\\)

		if (std::smatch rgx_match; std::regex_search((trigger_condition.begin() + offset), trigger_condition.end(), rgx_match, trigger_rgx))
		{
			auto updated_offset = (rgx_match.suffix().first - trigger_condition.begin());
			
			auto parsed_view = std::string_view
			{
				(trigger_condition.data() + offset),
				static_cast<std::size_t>(updated_offset)
			};

			auto entity_ref               = util::match_view(parsed_view, rgx_match, 1);
			//auto entity_access_operator = util::match_view(parsed_view, rgx_match, 2);
			auto type_name                = util::match_view(parsed_view, rgx_match, 3);
			//auto access_operator        = util::match_view(parsed_view, rgx_match, 4);
			auto member_name              = util::match_view(parsed_view, rgx_match, 5);
			auto comparison_operator      = util::match_view(parsed_view, rgx_match, 6);
			auto compared_value           = util::match_view(parsed_view, rgx_match, 7);

			return { entity_ref, type_name, member_name, comparison_operator, compared_value, updated_offset };
		}

		return {};
	}

	std::tuple<std::string_view, std::ptrdiff_t>
	EntityFactory::parse_event_type(const std::string& event_type, std::ptrdiff_t offset) // std::string_view
	{
		const auto event_type_rgx = std::regex("([^\\s\\:\\.\\-\\>\\&\\|\\(\\)]+)\\s*([^\\s\\&\\|\\(\\)]+)?");

		if (std::smatch rgx_match; std::regex_search((event_type.begin() + offset), event_type.end(), rgx_match, event_type_rgx))
		{
			auto updated_offset = (rgx_match.suffix().first - event_type.begin());

			auto parsed_view = std::string_view
			{
				(event_type.data() + offset),
				static_cast<std::size_t>(updated_offset)
			};

			auto type_name = util::match_view(parsed_view, rgx_match, 1);

			return { type_name, updated_offset };
		}

		return {};
	}

	std::optional<Timer::Duration> EntityFactory::parse_time_duration(const std::string& time_expr) // std::string_view
	{
		using namespace entt::literals;

		const auto time_rgx = std::regex("([\\d\\.]+)\\s*(.*)"); // (d|h|m|s|ms|us)?

		if (std::smatch rgx_match; std::regex_search(time_expr.begin(), time_expr.end(), rgx_match, time_rgx))
		{
			const auto numeric_str = util::match_view(time_expr, rgx_match, 1);

			if (const auto number = util::from_string<Timer::DurationRaw>(numeric_str))
			{
				const auto time_symbol = util::match_view(time_expr, rgx_match, 2);

				return Timer::to_duration(*number, time_symbol);
			}
		}

		return std::nullopt;
	}

	std::optional<Timer::Duration> EntityFactory::parse_time_duration(const util::json& time_data)
	{
		switch (time_data.type())
		{
			case util::json::value_t::string:
				if (const auto timer_expr = time_data.get<std::string>(); !timer_expr.empty())
				{
					return parse_time_duration(timer_expr);
				}

				break;

			case util::json::value_t::number_float:
				return Timer::to_duration(time_data.get<float>()); // double

			case util::json::value_t::number_integer:
			case util::json::value_t::number_unsigned:
				return Timer::Seconds(time_data.get<std::uint32_t>()); // std::uint64_t
		}

		return std::nullopt;
	}

	Entity EntityFactory::create(const EntityConstructionContext& context) const
	{
		using namespace entt::literals;

		auto entity = context.opt_entity_out;

		if (entity == null)
		{
			entity = context.registry.create();
		}

		for (const auto& component : descriptor.components.type_definitions)
		{
			auto instance = component.instance(true, context.registry, entity);

			MetaAny result;

			if (auto emplace_fn = component.type.func("emplace_meta_component"_hs))
			{
				result = emplace_fn.invoke
				(
					{},
					entt::forward_as_meta(context.registry),
					entt::forward_as_meta(entity),
					entt::forward_as_meta(std::move(instance))
				);
			}

			if (!result)
			{
				print_warn("Failed to instantiate component: \"#{}\"", component.type.id());
			}
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

		return entity;
	}

	bool EntityFactory::process_component
	(
		EntityDescriptor::TypeInfo& components_out,
		std::string_view component_name,
		const util::json* data,
		
		std::optional<SmallSize> constructor_arg_count,

		const MetaTypeDescriptorFlags& component_flags,

		bool allow_entry_update,
		bool allow_new_entry,
		bool allow_default_entries,

		bool ignore_special_symbols
	)
	{
		// Ignore special entry names/symbols:
		if (!ignore_special_symbols)
		{
			// Used by states for `update` actions.
			if (component_name == "target")
			{
				return false;
			}
		}

		//auto meta = entt::resolve();
		auto meta_type = meta_type_from_name(component_name);

		if (!meta_type)
		{
			print("Failed to resolve reflection for symbol: \"{}\"", component_name);

			return false;
		}

		auto make_type_descriptor = [&data, &meta_type, &constructor_arg_count, &component_flags]()
		{
			if (data)
			{
				if (const auto& component_content = *data; !component_content.empty())
				{
					return MetaTypeDescriptor
					(
						meta_type, component_content,
						{ .resolve_component_member_references=true },
						constructor_arg_count, component_flags
					);
				}
			}

			return MetaTypeDescriptor(meta_type, constructor_arg_count, component_flags);
		};

		auto component = make_type_descriptor();

		auto check_construction_type = [allow_new_entry, allow_default_entries, &data]() -> bool
		{
			if (!allow_new_entry)
			{
				return false;
			}

			if (!allow_default_entries)
			{
				if (!data)
				{
					return false;
				}
			}

			return true;
		};

		auto& loaded_components = components_out.type_definitions;

		auto it = std::find_if(loaded_components.begin(), loaded_components.end(), [&component](const MetaTypeDescriptor& entry) -> bool
		{
			if (entry.type == component.type)
			{
				return true;
			}

			return false;
		});

		if (it == loaded_components.end())
		{
			if (!check_construction_type())
			{
				return false;
			}

			loaded_components.emplace_back(std::move(component));
		}
		else
		{
			if (allow_entry_update)
			{
				it->set_variables(std::move(component));
			}
			else
			{
				if (!check_construction_type())
				{
					return false;
				}

				*it = std::move(component);
			}
		}

		return true;
	}

	std::size_t EntityFactory::process_component_list
	(
		EntityDescriptor::TypeInfo& components_out,
		const util::json& components,

		const MetaTypeDescriptorFlags& shared_component_flags,

		bool allow_new_entry,
		bool allow_default_entries,
		bool forward_entry_update_condition_to_flags,

		bool ignore_special_symbols
	)
	{
		std::size_t count = 0;

		auto as_component =
		[
			this, &components_out, &shared_component_flags,
			allow_new_entry, allow_default_entries, forward_entry_update_condition_to_flags, ignore_special_symbols,
			&count
		]
		(const auto& component_declaration, const util::json* component_content=nullptr)
		{
			auto [component_name, allow_entry_update, constructor_arg_count] = parse_component_declaration(component_declaration);

			const bool force_entry_update = ((!allow_new_entry) && (!allow_default_entries));

			auto flags = shared_component_flags;

			if (forward_entry_update_condition_to_flags && allow_entry_update)
			{
				flags.force_field_assignment = true;
			}

			auto result = process_component
			(
				components_out,
				component_name, component_content, constructor_arg_count,
				flags, (allow_entry_update || force_entry_update),
				allow_new_entry, allow_default_entries,
				ignore_special_symbols
			);

			if (result)
			{
				count++;
			}
		};

		util::json_for_each(components, [this, &as_component](const util::json& comp)
		{
			switch (comp.type())
			{
				case util::json::value_t::object:
				{
					for (const auto& proxy : comp.items())
					{
						const auto& component_declaration = proxy.key();
						const auto& component_content = proxy.value();

						as_component(component_declaration, &component_content);
					}

					break;
				}
				case util::json::value_t::string:
				{
					const auto component_declaration = comp.get<std::string>();

					as_component(component_declaration);

					break;
				}
			}
		});

		return count;
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
		const std::filesystem::path& base_path
	)
	{
		auto [state_base_path, state_path, state_data] = load_state_data(state_path_raw, base_path);

		auto state_name = resolve_state_name(state_data, state_path);

		if (const auto state = process_state(states_out, state_data, state_name, state_base_path))
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
		const std::filesystem::path& base_path
	)
	{
		std::size_t count = 0;

		util::json_for_each
		(
			data,

			[this, &states_out, &state, &base_path, &count](const util::json& content)
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

							if (process_state(states_out, state, state_data, state_base_path))
							{
								count++;
							}
						}

						break;
					}

					default:
						if (process_state(states_out, state, content, base_path))
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
		const std::filesystem::path& base_path
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
				auto result = process_state(states_out, *existing, data, base_path);

				assert(result);

				return existing;
			}
		}

		// NOTE: State objects are currently heap allocated to ensure a consistent memory location.
		// Likewise, storing multiple `EntityState` objects within a scope may be too bloated for stack usage.
		auto state = std::make_unique<EntityState>();

		// Assign the state's name to the ID we resolved.
		state->name = state_id;

		if (!process_state(states_out, *state, data, base_path))
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
		const std::filesystem::path& base_path
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
			merge_state_list(states_out, state, *merge, base_path);
		}

		// Handle embedded imports of other states before any further operations.
		// (Allows imports in this scope to take priority over imports-within-imports):
		if (auto imports = util::find_any(data, "import", "imports", "state", "states"); imports != data.end())
		{
			// NOTE: Recursion by proxy.
			process_state_list(states_out, *imports, base_path);
		}

		if (auto persist = util::find_any(data, "persist", "share", "shared", "modify", "="); persist != data.end())
		{
			process_component_list(state.components.persist, *persist, {}, true, true, true);
		}

		if (auto add = util::find_any(data, "add", "+"); add != data.end())
		{
			process_component_list(state.components.add, *add, {}, true, true, true);
		}

		if (auto removal_list = util::find_any(data, "remove", "-", "~"); removal_list != data.end())
		{
			state.build_removals(*removal_list);
		}
		
		if (auto frozen_list = util::find_any(data, "frozen", "freeze", "exclude", "%", "^"); frozen_list != data.end())
		{
			state.build_frozen(*frozen_list);
		}

		if (auto storage_list = util::find_any(data, "store", "storage", "include", "temp", "temporary", "#"); storage_list != data.end())
		{
			state.build_storage(*storage_list);
		}

		if (auto isolated = util::find_any(data, "local", "local_storage", "isolate", "isolated"); isolated != data.end())
		{
			process_state_isolated_components(state, *isolated);
		}

		if (auto local_copy = util::find_any(data, "local_copy", "copy"); local_copy != data.end())
		{
			process_state_local_copy_components(state, *local_copy);
		}

		if (auto init_copy = util::find_any(data, "init_copy", "local_modify", "copy_once", "clone"); init_copy != data.end())
		{
			process_state_init_copy_components(state, *init_copy);
		}

		if (auto time_data = util::find_any(data, "timer", "wait", "delay"); time_data != data.end())
		{
			state.activation_delay = parse_time_duration(*time_data);
		}

		if (auto rules = util::find_any(data, "rule", "rules", "trigger", "triggers"); rules != data.end())
		{
			process_state_rules(state, *rules, &states_out, &base_path);
		}

		if (auto threads = util::find_any(data, "do", "threads", "execute"); threads != data.end())
		{
			// ...
		}

		return true;
	}

	std::size_t EntityFactory::process_state_list(EntityDescriptor::StateCollection& states_out, const util::json& data, const std::filesystem::path& base_path)
	{
		std::size_t count = 0;

		util::json_for_each(data, [this, &base_path, &states_out, &count](const util::json& state_entry)
		{
			switch (state_entry.type())
			{
				case util::json::value_t::object:
				{
					// NOTE: Embedded state definitions must have a `name` field.
					const auto state_name = util::get_value<std::string>(state_entry, "name");

					if (process_state(states_out, state_entry, state_name, base_path))
					{
						count++;
					}

					break;
				}
				case util::json::value_t::string:
				{
					const auto state_path_raw = state_entry.get<std::string>();

					if (process_state(states_out, state_path_raw, base_path))
					{
						count++;
					}

					break;
				}
			}
		});

		return count;
	}

	std::size_t EntityFactory::process_state_isolated_components(EntityState& state, const util::json& isolated)
	{
		return process_and_inspect_component_list
		(
			state.components.add,

			isolated,

			[&state](std::string_view component_name)
			{
				if (!state.process_type_list_entry(state.components.freeze, component_name, true)) // false
				{
					print_warn("Failed to process embedded `freeze` entry from isolation data.");

					return false;
				}

				if (!state.process_type_list_entry(state.components.store, component_name, true)) // false
				{
					print_warn("Failed to process embedded `store` entry from isolation data.");

					return false;
				}

				return true;
			},

			false
		);
	}

	std::size_t EntityFactory::process_state_local_copy_components(EntityState& state, const util::json& local_copy)
	{
		const auto build_result = state.build_local_copy(local_copy);

		const auto components_processed = process_component_list
		(
			state.components.add, local_copy,
			{
				.allow_default_construction             = false,
				.allow_forwarding_fields_to_constructor = false,
				.force_field_assignment                 = true
			},

			true, false
		);

		assert(components_processed == build_result);

		return build_result;
	}

	std::size_t EntityFactory::process_state_init_copy_components(EntityState& state, const util::json& init_copy)
	{
		const auto build_result = state.build_init_copy(init_copy);

		const auto components_processed = process_component_list
		(
			state.components.add, init_copy,
			{
				.allow_default_construction             = false,
				.allow_forwarding_fields_to_constructor = false,
				.force_field_assignment                 = true
			},

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
			const auto inline_import = process_state_inline_import(opt_states_out, next_state_raw, opt_base_path);

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

		auto process_command = [&process_rule](CommandContent&& content)
		{
			return process_rule
			(
				EntityStateCommandAction
				{
					std::move(content)
				}
			);
		};

		constexpr std::size_t command_arg_offset = 2;

		auto init_empty_command = [](std::string_view command_name) -> std::optional<CommandContent>
		{
			if (command_name.empty())
			{
				return std::nullopt;
			}

			using namespace entt::literals;

			const auto command_name_id = hash(command_name);
			const auto command_type = resolve(command_name_id);

			if (!command_type)
			{
				print_warn("Unable to resolve command name: {} (#{})", command_name, command_name_id);

				return std::nullopt;
			}

			auto command_content = CommandContent(command_type);

			command_content.set_variable(MetaVariable("source"_hs, Entity(null)));
			command_content.set_variable(MetaVariable("target"_hs, Entity(null)));

			return command_content;
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

				return process_command(std::move(*command_content));
			}

			return {};
		};

		auto process_command_rule_from_expr = [&process_command_rule_from_csv](const auto& command_expr) -> EntityStateRule*
		{
			auto [command_name, command_content, is_string_content] = util::parse_single_argument_command(command_expr);

			return process_command_rule_from_csv(command_name, command_content, !is_string_content);
		};

		auto process_command_rule_from_object = [&init_empty_command, &process_command](std::string_view command_name, const util::json& command_data) -> EntityStateRule*
		{
			if (auto command_content = init_empty_command(command_name))
			{
				command_content->set_variables
				(
					command_data,
					{ .resolve_component_member_references = true },
					command_arg_offset
				);

				return process_command(std::move(*command_content));
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

						[this, &process_rule, &target](const util::json& update_entry)
						{
							EntityStateUpdateAction::Components updated_components;

							process_component_list
							(
								updated_components,
								update_entry, {}, // { .force_field_assignment = true },
								true, true, true,
								false
							);

							EntityTarget update_target = { target };

							if (auto manual_target_entry = util::find_any(update_entry, "target"); manual_target_entry != update_entry.end())
							{
								const auto manual_target_raw = manual_target_entry->get<std::string>();

								if (auto result = EntityTarget::parse(manual_target_raw))
								{
									update_target = *result;
								}
								else
								{
									print_warn("Failed to resolve target while constructing entity update action: \"{}\" -- Using default target instead.", manual_target_raw);
								}
							}

							process_rule
							(
								EntityStateUpdateAction
								{
									std::move(updated_components),
									std::move(update_target)
								}
							);
						}
					);
				}

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
						"command", "commands", "generate"
					);
				}

				break;
			}
		}

		return count;
	}

	MetaAny EntityFactory::process_trigger_condition_value(std::string_view compared_value_raw)
	{
		MetaAny compared_value;

		bool resolve_native_value = true;

		if (util::is_quoted(compared_value_raw, '\"', '\''))
		{
			resolve_native_value = false;

			compared_value_raw = util::unquote(compared_value_raw);
		}
			
		if (compared_value_raw.empty())
		{
			return {};
		}

		if (resolve_native_value)
		{
			compared_value = meta_any_from_string
			(
				compared_value_raw,
				
				{
					.resolve_symbol     = true,
					.strip_quotes       = false,
					.fallback_to_string = false
				}
			);

			if (!compared_value)
			{
				// TODO: Add support for multiple levels of indirection. (e.g. "child(name)::RelationshipComponent::child(name)")
				// Look for (optionally entity-qualified) reference to data member of component:
				if (auto data_member = indirect_meta_data_member_from_string(compared_value_raw))
				{
					compared_value = *data_member;
				}
				else
				{
					// Look for reference to entity target:
					if (auto parse_result = EntityTarget::parse_type(compared_value_raw))
					{
						compared_value = EntityTarget { std::move(std::get<0>(*parse_result)) };
					}
				}
			}
		}
				
		if (!compared_value)
		{
			compared_value = std::string(compared_value_raw);
		}

		return compared_value;
	}

	std::tuple<MetaTypeID, entt::meta_data>
	EntityFactory::resolve_member(const entt::meta_type& type, std::string_view member_name)
	{
		if (member_name.empty())
		{
			return resolve_data_member(type, true, "entity", "target", "button", "self", "value", "name");
		}
		
		return resolve_data_member(type, true, member_name);
	}

	std::optional<EventTriggerSingleCondition> EntityFactory::process_standard_trigger_condition // std::optional<EventTriggerCondition>
	(
		const entt::meta_type& type,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,

		std::string_view trigger_condition_expr,

		bool embed_type_in_condition
	)
	{
		auto [member_id, member] = resolve_member(type, member_name);

		if (!member && (!member_name.empty()))
		{
			print_warn("Unable to resolve target member for rule-trigger: \"{}\"", trigger_condition_expr);

			return std::nullopt;
		}

		if (!member)
		{
			return std::nullopt;
		}
			
		if (compared_value_raw.empty())
		{
			return std::nullopt;

			// TODO: Look into idea of resolving `null` as source-entity.
			//compared_value = static_cast<Entity>(null); // null;
		}

		auto compared_value = process_trigger_condition_value(compared_value_raw);

		if (!compared_value)
		{
			return std::nullopt;
		}

		auto comparison_method = EventTriggerConditionType::get_comparison_method(comparison_operator);

		return EventTriggerSingleCondition
		{
			member_id,
			std::move(compared_value),
			comparison_method,

			((embed_type_in_condition) ? type : MetaType {})
		};
	}

	std::optional<EventTriggerMemberCondition> EntityFactory::process_member_trigger_condition
	(
		const entt::meta_type& type,

		std::string_view entity_ref,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,

		std::string_view trigger_condition_expr
	)
	{
		auto [member_id, member] = resolve_member(type, member_name);

		if (!member && (!member_name.empty()))
		{
			print_warn("Unable to resolve target member for rule-trigger: \"{}\"", trigger_condition_expr);

			return std::nullopt;
		}

		auto comparison_value = process_trigger_condition_value(compared_value_raw);

		if (!comparison_value)
		{
			return std::nullopt;
		}

		auto target = EntityTarget::parse(entity_ref);

		if (!target)
		{
			//target = { EntityTarget::SelfTarget {} };

			return std::nullopt;
		}

		auto comparison_method = EventTriggerConditionType::get_comparison_method(comparison_operator);

		return EventTriggerMemberCondition
		{
			IndirectMetaDataMember
			{
				std::move(*target),

				MetaDataMember
				{
					type.id(),
					member_id,
				}
			},

			std::move(comparison_value),

			comparison_method
		};
	}

	std::size_t EntityFactory::process_trigger_expression
	(
		EntityState& state,

		const std::string& trigger_condition_expr, // std::string_view
		const util::json& content,

		EntityDescriptor::StateCollection* opt_states_out,
		const std::filesystem::path* opt_base_path,
		bool allow_inline_import
	)
	{
		//using namespace entt::literals;
		using Combinator = EventTriggerCompoundMethod;

		std::size_t number_processed = 0;

		// Shorthand for `process_state_rule` + updating `number_processed`.
		auto process_rule = [this, &state, opt_states_out, opt_base_path, allow_inline_import, &number_processed]
		(
			MetaTypeID type_name_id,
			const util::json& content,
			std::optional<EventTriggerCondition> condition = std::nullopt
		)
		{
			const auto result = this->process_state_rule(state, type_name_id, content, condition, opt_states_out, opt_base_path, allow_inline_import);

			number_processed += result;

			return result;
		};

		std::size_t offset = 0; // std::ptrdiff_t

		std::optional<EventTriggerCondition> condition_out;
		std::optional<MetaType> active_type;
		Combinator active_combinator = Combinator::None;

		bool is_first_expr = true;

		// NOTE: Nesting (i.e. parentheses) is not currently supported.
		// Therefore, a 'store' operation simply processes a rule immediately, consuming the condition object.
		auto store_condition = [this, &content, &active_type, &condition_out, &process_rule]()
		{
			if (!condition_out.has_value())
			{
				return;
			}

			if (!active_type)
			{
				return;
			}

			process_rule(active_type->id(), content, std::move(*condition_out));

			// Ensure `std::nullopt` status.
			// (As opposed to 'moved-from' status)
			condition_out = std::nullopt;
		};

		do
		{
			auto [entity_ref, type_name, member_name, comparison_operator, compared_value_raw, updated_offset] = parse_trigger_condition(trigger_condition_expr, static_cast<std::ptrdiff_t>(offset));

			bool is_standalone_event_trigger = false;

			if (type_name.empty())
			{
				auto standalone_event_result = parse_event_type(trigger_condition_expr, static_cast<std::ptrdiff_t>(offset));

				type_name = std::get<0>(standalone_event_result);

				if (type_name.empty())
				{
					// Exit the loop, nothing else can be processed.
					break;
				}
				else
				{
					updated_offset = std::get<1>(standalone_event_result);

					is_standalone_event_trigger = true;
				}
			}

			auto expr_start_idx = static_cast<std::size_t>(((entity_ref.empty()) ? type_name.data() : entity_ref.data()) - trigger_condition_expr.data());
			//auto processed_length = (static_cast<std::size_t>(updated_offset) - offset);

			auto parsed_expr = std::string_view { trigger_condition_expr.data()+expr_start_idx, static_cast<std::size_t>(updated_offset-expr_start_idx) };

			if (!is_first_expr)
			{
				// Space between the old `offset` (i.e. end of previous expression) and the start of current expression.
				auto expr_gap = std::string_view { (trigger_condition_expr.data() + offset), (expr_start_idx - offset) };

				auto local_combinator_symbol_idx = std::string_view::npos;

				// Perform view-local lookup:
				if (local_combinator_symbol_idx = expr_gap.find("&&"); local_combinator_symbol_idx != std::string_view::npos)
				{
					active_combinator = Combinator::And;
				}
				else if (local_combinator_symbol_idx = expr_gap.find("||"); local_combinator_symbol_idx != std::string_view::npos)
				{
					active_combinator = Combinator::Or;
				}
					
				if (local_combinator_symbol_idx == std::string_view::npos)
				{
					print_warn("Missing combinator symbol in state-rule trigger expression; using last known combinator as fallback.");
				}
				/*
				else
				{
					// Convert from view-local index to global index.
					const auto combinator_symbol_idx = static_cast<std::size_t>((expr_gap.data() + local_combinator_symbol_idx) - trigger_condition_expr.data());

					constexpr std::size_t combinator_symbol_size = 2;
					if (trigger_condition_expr.find("(", (combinator_symbol_idx + combinator_symbol_size)) != std::string_view::npos)
					{
						// ...
					}
				}
				*/
			}
				
			// Update the processing offset to the latest location reported by our parsing step.
			offset = static_cast<std::size_t>(updated_offset);

			// Attempt to resolve `type_name`:
			auto type_name_id = hash(type_name).value();

			bool active_type_changed = false;

			if (!active_type.has_value())
			{
				active_type = resolve(type_name_id);

				if (!active_type)
				{
					throw std::runtime_error(format("Unable to resolve trigger/event type: \"{}\" (#{})", type_name, type_name_id));
				}
			}

			MetaType expr_type = *active_type;

			if (active_type->id() != type_name_id)
			{
				if (active_combinator == Combinator::And)
				{
					//throw std::runtime_error(format("Unsupported operation: Unable to build 'AND' compound condition with multiple event types. ({})", parsed_expr));
					//print("NOTE: Use of type other than initial event type will result in component-only qualification.");

					if (auto type = resolve(type_name_id))
					{
						expr_type = type;
					}
				}
				else
				{
					// Since combining event types in trigger-clauses is not supported,
					// we need to process the condition we've already built.
					store_condition();

					// With the previous condition handled, we can switch to the new event-type:
					active_type = resolve(type_name_id);

					if (!active_type)
					{
						throw std::runtime_error(format("Unable to resolve trigger/event type in compound condition: \"{}\" (#{})", type_name, type_name_id));
					}

					active_type_changed = true;
				}
			}

			assert(active_type);

			auto on_condition = [&store_condition](EventTriggerCondition& condition_out, auto&& generated_condition, Combinator combinator)
			{
				// Covers the first time visiting a condition. (Fragment only; no AND/OR container generated)
				// 'Basic condition' scenarios: `EventTriggerSingleCondition`, `EventTriggerTrueCondition`, `EventTriggerFalseCondition` (see below)
				auto basic_condition = [combinator, &store_condition, &condition_out, &generated_condition](auto& condition)
				{
					switch (combinator)
					{
						case Combinator::And:
						{
							// Generate an AND container:
							EventTriggerAndCondition and_condition;

							and_condition.add_condition(std::move(condition));
							and_condition.add_condition(std::move(generated_condition));

							// Store the newly generated container in place of the old condition-fragment.
							condition_out = std::move(and_condition);

							break;
						}
						case Combinator::Or:
						{
							// Generate an OR container:
							EventTriggerOrCondition or_condition;

							if constexpr (std::is_same_v<std::decay_t<decltype(condition)>, EventTriggerTrueCondition>)
							{
								print_warn("Attempting to add 'always-true' condition(s) to 'OR' compound condition.");
							}

							or_condition.add_condition(std::move(condition));
							or_condition.add_condition(std::move(generated_condition));

							// Store the newly generated container in place of the old condition-fragment.
							condition_out = std::move(or_condition);

							break;
						}

						default:
							// In the event that there is no conbinator, process the existing
							// condition/fragment and swap in the newly generated one:
							store_condition();

							condition_out = std::move(generated_condition);

							break;
					}
				};

				util::visit
				(
					condition_out,

					[&basic_condition](EventTriggerSingleCondition& single_condition) { basic_condition(single_condition); },
					[&basic_condition](EventTriggerMemberCondition& member_condition) { basic_condition(member_condition); },
					[&basic_condition](EventTriggerTrueCondition& true_condition)     { basic_condition(true_condition); },
					[&basic_condition](EventTriggerFalseCondition& false_condition)   { basic_condition(false_condition); },

					// Existing AND container:
					[combinator, &store_condition, &condition_out, &generated_condition](EventTriggerAndCondition& and_condition)
					{
						switch (combinator)
						{
							case Combinator::And:
								// Add to this container as usual.
								and_condition.add_condition(std::move(generated_condition));

								break;
							case Combinator::Or:
								// The user has indicated a switch to an OR clause.
								// Process this AND container as-is, then swap in the
								// new condition fragment for further operation.
								store_condition();

								condition_out = std::move(generated_condition);

								break;

							default:
								// Unsupported conbinator; ignore this fragment.

								break;
						}
					},

					// Existing OR container:
					[combinator, &store_condition, &condition_out, &generated_condition](EventTriggerOrCondition& or_condition)
					{
						switch (combinator)
						{
							case Combinator::And:
								// The user has indicated a switch to an AND clause.
								// Process this OR container as-is, then swap in the
								// new condition fragment for further operation.
								store_condition();

								condition_out = std::move(generated_condition);

								break;
							case Combinator::Or:
								// Add to this container as usual.
								or_condition.add_condition(std::move(generated_condition));

								break;

							default:
								// Unsupported conbinator; ignore this fragment.

								break;
						}
					}
				);
			};

			// Workaround for compound AND/OR on 'standalone' (no-condition) of same event type.
			// Without this workaround, an extra rule would be generated.
			// 
			// NOTE: This does not cover duplication through multiple triggers, only duplication within the same trigger expression.
			// TODO: Look into implications of duplicated triggers.
			if ((is_standalone_event_trigger) && (!is_first_expr) && (!active_type_changed))
			{
				//print_warn("Detected conditionless event-trigger of same type as previous conditioned trigger -- generating dummy 'always-true' condition to avoid anomalies.");
				//on_condition(*condition_out, EventTriggerTrueCondition {}, active_combinator);

				print_warn("Detected conditionless event-trigger of same type as previous conditioned trigger -- ignoring to avoid anomalies.");
			}
			else
			{
				auto handle_condition = [&on_condition, &active_combinator, &condition_out](auto&& generated_condition)
				{
					// Attempt to resolve a single condition (i.e. fragment) from the expression we parsed.
					if (generated_condition) // *active_type
					{
						if (condition_out.has_value())
						{
							on_condition(*condition_out, *generated_condition, active_combinator);
						}
						else
						{
							condition_out = std::move(*generated_condition);
						}
					}
					else
					{
						if (condition_out.has_value())
						{
							on_condition(*condition_out, EventTriggerTrueCondition {}, active_combinator);
						}
						else
						{
							condition_out = EventTriggerTrueCondition {};
						}
					}
				};

				if (entity_ref.empty())
				{
					// Standard event triggers:
					handle_condition
					(
						process_standard_trigger_condition
						(
							expr_type, member_name,
							comparison_operator, compared_value_raw,
							parsed_expr,

							(expr_type != *active_type)
						)
					);
				}
				else
				{
					// Fully qualified component-member event triggers:
					handle_condition
					(
						process_member_trigger_condition
						(
							expr_type, entity_ref, member_name,
							comparison_operator, compared_value_raw,
							parsed_expr
						)
					);
				}
			}

			is_first_expr = false;

			// Debugging related:
			/*
			auto remaining_length = (trigger_condition_expr.size() - static_cast<std::size_t>(updated_offset));
			auto remaining = std::string_view{ (trigger_condition_expr.data() + updated_offset), remaining_length };

			print(remaining);
			*/
		} while (offset < trigger_condition_expr.size());

		if (!active_type)
		{
			return number_processed;
		}

		// Process the rule using whatever's left.
		if (condition_out)
		{
			process_rule(active_type->id(), content, std::move(*condition_out));
		}
		else
		{
			process_rule(active_type->id(), content);
		}

		// NOTE: Updated automatically in `process_rule` subroutine.
		return number_processed;
	}

	std::size_t EntityFactory::process_state_rules
	(
		EntityState& state,
		const util::json& rules,

		EntityDescriptor::StateCollection* opt_states_out,
		const std::filesystem::path* opt_base_path,
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
						opt_states_out, opt_base_path, allow_inline_import
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
				opt_states_out, opt_base_path, allow_inline_import
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

		auto [inline_command, state_path_raw, is_string_content] = util::parse_single_argument_command(command);

		if (state_path_raw.empty())
		{
			return nullptr;
		}

		switch (hash(inline_command))
		{
			case "import"_hs:
				// NOTE: Recursion.
				return process_state(*states_out, state_path_raw, *base_path);
		}

		return nullptr;
	}

	void EntityFactory::process_archetype(const util::json& data, const std::filesystem::path& base_path, bool resolve_external_modules)
	{
		if (resolve_external_modules)
		{
			resolve_archetypes(data, base_path);
		}

		if (auto components = util::find_any(data, "component", "components"); components != data.end())
		{
			process_component_list(descriptor.components, *components);
		}

		if (auto states = util::find_any(data, "state", "states"); states != data.end())
		{
			process_state_list(descriptor.states, *states, base_path);
		}

		if (auto default_state = data.find("default_state"); default_state != data.end())
		{
			switch (default_state->type())
			{
				case util::json::value_t::string:
				{
					auto state_name = default_state->get<std::string>();
					auto state_name_hash = hash(state_name);

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
	}
}