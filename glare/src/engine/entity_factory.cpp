#include "entity_factory.hpp"
#include "event_trigger_condition.hpp"

#include "components/relationship_component.hpp"
#include "components/instance_component.hpp"

#include "state/components/state_component.hpp"

#include "meta/meta.hpp"
#include "meta/serial.hpp"

#include <util/algorithm.hpp>
#include <util/string.hpp>

#include <algorithm>
#include <chrono>

// TODO: Change to better regular expression library.
#include <regex>

// Debugging related:
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

	std::tuple<std::string_view, std::string_view, std::string_view, std::string_view>
	EntityFactory::parse_trigger_condition(const std::string& trigger_condition) // std::string_view
	{
		const auto trigger_rgx = std::regex("([^\\s\\:\\.\\-\\>]+)\\s*(\\:\\:|\\.|\\-\\>)?\\s*([^\\s]+)?\\s*(==|===|!=|!==|\\|)\\s*(.+)");

		if (std::smatch rgx_match; std::regex_search(trigger_condition.begin(), trigger_condition.end(), rgx_match, trigger_rgx))
		{
			auto type_name           = util::match_view(trigger_condition, rgx_match, 1);
			//auto access_operator   = util::match_view(trigger_condition, rgx_match, 2);
			auto member_name         = util::match_view(trigger_condition, rgx_match, 3);
			auto comparison_operator = util::match_view(trigger_condition, rgx_match, 4);
			auto compared_value      = util::match_view(trigger_condition, rgx_match, 5);

			return { type_name, member_name, comparison_operator, compared_value };
		}

		return {};
	}

	std::tuple<std::string_view, std::string_view, bool>
	EntityFactory::parse_single_argument_command(const std::string& command) // std::string_view
	{
		const auto command_rgx = std::regex("([^\\s\\(]+)\\(\\s*(\\\")?([^\\\"]*)(\\\")?\\s*\\)\\s*");

		if (std::smatch rgx_match; std::regex_search(command.begin(), command.end(), rgx_match, command_rgx))
		{
			auto command_name    = util::match_view(command, rgx_match, 1);
			auto command_content = util::match_view(command, rgx_match, 3);

			bool is_string_content = (rgx_match[2].matched && rgx_match[4].matched);

			return { command_name, command_content, is_string_content };
		}

		return { {}, {}, false };
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

	// TODO: Move to a different source file.
	EntityStateTarget::TargetType EntityFactory::process_rule_target(const util::json& target_data)
	{
		using namespace entt::literals;

		using Target = EntityStateTarget;

		switch (target_data.type())
		{
			case util::json::value_t::string:
			{
				auto raw_value = target_data.get<std::string>();

				if ((raw_value == "self") || (raw_value == "this"))
				{
					return Target::SelfTarget {};
				}

				if (raw_value == "parent")
				{
					return Target::ParentTarget {};
				}

				if (auto [command_name, command_content, is_string_content] = parse_single_argument_command(raw_value); !command_name.empty())
				{
					switch (hash(command_name))
					{
						case "entity"_hs:
							if (!is_string_content)
							{
								if (auto entity_id_raw = util::from_string<entt::id_type>(raw_value))
								{
									return Target::EntityTarget { static_cast<Entity>(*entity_id_raw) };
								}
							}

							break;

						case "child"_hs:
						{
							if (command_content.contains(','))
							{
								auto parameter_idx = 0;

								std::string_view child_name;
								bool recursive = true;

								util::split(command_content, ",", [&parameter_idx, &child_name, &recursive](std::string_view argument, bool is_last_argument=false) -> bool
								{
									argument = util::trim(argument);

									switch (parameter_idx++)
									{
										// First argument: `child_name`
										case 0:
											child_name = argument;

											break;

										// Second argument: `recursive`
										case 1:
										{
											switch (hash(argument))
											{
												// Recursion is enabled by default; no
												// need to check for positive values:
												/*
												case "true"_hs:
												case "1"_hs:
													recursive = true;

													break;
												*/

												//case "f"_hs:
												case "0"_hs:
												case "false"_hs:
													recursive = false;

													break;
											}

											return false;
										}
									}

									return true;
								});

								return Target::ChildTarget { hash(child_name), recursive };
							}
							else
							{
								return Target::ChildTarget { hash(command_content) };
							}
						}

						case "player"_hs:
							if (auto player_index = util::from_string<PlayerIndex>(command_content))
							{
								return Target::PlayerTarget { *player_index };
							}

							break;

						case "parent"_hs:
							return Target::ParentTarget {};

						case "self"_hs:
							return Target::SelfTarget {};
					}

					return Target::EntityNameTarget { hash(command_content) };
				}

				break;
			}

			case util::json::value_t::number_integer:
			case util::json::value_t::number_unsigned:
				return Target::EntityTarget { static_cast<Entity>(target_data.get<entt::id_type>()) };
		}

		return Target::SelfTarget {};
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
			auto instance = component.instance();

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
		bool allow_inplace_changes,
		std::optional<SmallSize> constructor_arg_count
	)
	{
		//auto meta = entt::resolve();
		auto meta_type = meta_type_from_name(component_name);

		if (!meta_type)
		{
			print("Failed to resolve reflection for symbol: \"{}\"", component_name);

			return false;
		}

		auto make_type_descriptor = [&data, &meta_type, &constructor_arg_count]()
		{
			if (data)
			{
				if (const auto& component_content = *data; !component_content.empty())
				{
					return MetaTypeDescriptor(meta_type, component_content, constructor_arg_count);
				}
			}

			return MetaTypeDescriptor(meta_type, constructor_arg_count);
		};

		auto component = make_type_descriptor();

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
			loaded_components.emplace_back(std::move(component));
		}
		else
		{
			if (allow_inplace_changes)
			{
				it->set_variables(std::move(component));
			}
			else
			{
				*it = std::move(component);
			}
		}

		return true;
	}

	std::size_t EntityFactory::process_component_list
	(
		EntityDescriptor::TypeInfo& components_out,
		const util::json& components
	)
	{
		std::size_t count = 0;

		auto as_component = [this, &components_out, &count](const auto& component_declaration, const util::json* component_content=nullptr)
		{
			auto [component_name, allow_inplace_changes, constructor_arg_count] = parse_component_declaration(component_declaration);

			if (process_component(components_out, component_name, component_content, allow_inplace_changes, constructor_arg_count))
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

	const EntityState* EntityFactory::resolve_state
	(
		EntityDescriptor::StateCollection& states_out,
		std::string_view state_path_raw, // const std::string&
		const std::filesystem::path& base_path
	)
	{
		const auto state_path = resolve_reference(std::filesystem::path(state_path_raw), base_path);

		// TODO: Optimize.
		auto state_data = util::load_json(state_path);

		std::string state_name;

		// Check for a top-level `name` field in the imported state:
		if (auto name_it = state_data.find("name"); name_it != state_data.end())
		{
			// Use the embedded name.
			state_name = name_it->get<std::string>();
		}
		else
		{
			auto filename = state_path.filename(); filename.replace_extension();

			// An embedded name couldn't be found; use the state's filename instead.
			state_name = filename.string();
		}

		// Since this is an imported state, we'll need to retrieve a new base-path.
		const auto state_base_path = state_path.parent_path();

		if (const auto state = process_state(states_out, state_data, state_name, state_base_path))
		{
			assert(state->name.has_value());

			return state;
		}

		return nullptr;
	}

	const EntityState* EntityFactory::process_state
	(
		EntityDescriptor::StateCollection& states_out,
		const util::json& data,
		std::string_view state_name,
		const std::filesystem::path& base_path
	)
	{
		using namespace entt::literals;

		// Handle embedded imports of other states:
		if (auto imports = util::find_any(data, "import", "imports", "state", "states"); imports != data.end())
		{
			// NOTE: Recursion.
			process_state_list(states_out, *imports, base_path);
		}

		EntityState state;

		if (!state_name.empty())
		{
			state.name = hash(state_name);
		}

		if (auto persist = util::find_any(data, "persist", "share", "shared", "mutate", "modify", "change", "="); persist != data.end())
		{
			process_component_list(state.components.persist, *persist);
		}

		if (auto add = util::find_any(data, "add", "+"); add != data.end())
		{
			process_component_list(state.components.add, *add);
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

		if (auto isolated = util::find_any(data, "isolate", "isolated", "local", "local_storage"); isolated != data.end())
		{
			process_state_isolated_components(state, *isolated);
		}

		if (auto time_data = util::find_any(data, "timer", "wait", "delay"); time_data != data.end())
		{
			state.activation_delay = parse_time_duration(*time_data);
		}

		if (auto rules = util::find_any(data, "rules", "triggers"); rules != data.end())
		{
			process_state_rules(state, *rules, &states_out, &base_path);
		}

		if (state.name)
		{
			if (auto existing = descriptor.get_state(*state.name))
			{
				print_warn("Overriding definition of state: \"{}\"", state_name);

				*existing = std::move(state);

				return existing;
			}
		}

		//descriptor.set_state(std::move(state));

		//descriptor.states.emplace_back(std::move(state));
		const auto& result = descriptor.states.emplace_back(std::make_unique<EntityState>(std::move(state)));

		return result.get();
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

					if (resolve_state(states_out, state_path_raw, base_path))
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
		process_component_list(state.components.add, isolated);

		std::size_t count = 0;

		auto process_isolated = [&state, &count](std::string_view component_name)
		{
			if (!state.process_type_list_entry(state.components.freeze, component_name, true)) // false
			{
				print_warn("Failed to process embedded `freeze` entry from isolation data.");

				return;
			}

			if (!state.process_type_list_entry(state.components.store, component_name, true)) // false
			{
				print_warn("Failed to process embedded `store` entry from isolation data.");

				return;
			}

			count++;
		};

		const auto container_type = isolated.type();

		switch (container_type)
		{
			case util::json::value_t::object:
			{
				for (const auto& proxy : isolated.items())
				{
					const auto& component_declaration = proxy.key();

					// Since component declarations can have additional symbols,
					// we'll need to extract only the 'name' substring.
					// 
					// TODO: Optimize by rolling `process_component_list` into this routine. (Parses declarations twice)
					auto component_decl_info = parse_component_declaration(component_declaration);
					const auto& component_name = std::get<0>(component_decl_info);

					process_isolated(component_name);
				}

				break;
			}
			case util::json::value_t::array:
			{
				for (const auto& proxy : isolated.items())
				{
					const auto component_name = proxy.value().get<std::string>();

					process_isolated(std::string_view(component_name));
				}

				break;
			}
			default:
			{
				print_warn("Unknown type identified in place of isolation data.");

				break;
			}
		}

		// See above for accumulation.
		return count;
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
		using namespace entt::literals;

		std::size_t count = 0;
		
		auto process_rule = [this, &state, &count, &opt_states_out, &opt_base_path, allow_inline_import](const std::string& trigger_condition_expr, const util::json& content) -> bool // std::string_view // [this, ...]
		{
			auto [type_name, member_name, comparison_operator, compared_value_raw] = parse_trigger_condition(trigger_condition_expr);

			if (type_name.empty())
			{
				// If we weren't able to parse the input as an expression/condition,
				// fall back to assuming the string provided is a type.
				type_name = trigger_condition_expr;
			}

			auto type_name_id = hash(type_name).value();
			auto type = resolve(type_name_id);

			if (!type)
			{
				print_warn("Unable to resolve trigger/event type: \"{}\" (#{})", type_name, type_name_id);

				return false;
			}

			auto [member_id, member] = (member_name.empty())
				? resolve_data_member(type, true, "entity", "target", "button", "self")
				: resolve_data_member(type, true, member_name)
			;

			if (!member && (!member_name.empty()))
			{
				print_warn("Unable to resolve target member for rule-trigger: \"{}\"", trigger_condition_expr); // type_name

				return false;
			}

			std::optional<EventTriggerCondition> condition = std::nullopt;

			if (member)
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
					compared_value = static_cast<Entity>(null); // null;
				}
				else if (resolve_native_value)
				{
					compared_value = meta_any_from_string(compared_value_raw, true, false);

					if (!compared_value)
					{
						print_warn("Unable to resolve comparison value for rule/trigger.");

						return false;
					}
				}
				else
				{
					compared_value = std::string(compared_value_raw);
				}

				if (compared_value)
				{
					auto comparison_method = (comparison_operator.empty())
						? EventTriggerCondition::ComparisonMethod::Equal
						: EventTriggerCondition::get_comparison_method(comparison_operator)
					;

					condition = EventTriggerCondition
					{
						//.event_type = type,
						.event_type_member = member_id,

						.comparison_value = std::move(compared_value),
						.comparison_method = comparison_method
					};
				}
			}

			EntityStateTarget::TargetType target = EntityStateTarget::SelfTarget {};
			std::optional<Timer::Duration> delay = std::nullopt;

			auto process_transition_rule = [&](const auto& next_state_raw)
			{
				if (next_state_raw.empty())
				{
					return false;
				}

				const auto inline_import = process_state_inline_import(opt_states_out, next_state_raw, opt_base_path);

				StringHash next_state_id = (inline_import) // EntityStateHash
					? *inline_import->name
					: hash(next_state_raw)
				;

				auto& rules_out = state.rules[type_name_id];

				rules_out.emplace_back
				(
					std::move(condition),
					delay,

					EntityStateTransitionRule
					{
						.target     = std::move(target),
						.state_name = next_state_id
					}
				);

				count++;

				return true;
			};

			switch (content.type())
			{
				case util::json::value_t::string:
					return process_transition_rule(content.get<std::string>());
				
				case util::json::value_t::object:
				{
					if (auto target_data = util::find_any(content, "target", "target_entity"); target_data != content.end())
					{
						target = process_rule_target(*target_data);
					}

					if (auto time_data = util::find_any(content, "timer", "wait", "delay"); time_data != content.end())
					{
						delay = parse_time_duration(*time_data);
					}

					if (auto next_state = util::find_any(content, "state", "next", "next_state"); next_state != content.end())
					{
						return process_transition_rule(next_state->get<std::string>());
					}

					if (auto command = util::find_any(content, "command", "generate"); command != content.end())
					{
						// TODO: Implement commands.
						assert(false);

						//return false;
					}

					break;
				}
			}

			print_warn("Unable to process rule for: {}", trigger_condition_expr);

			return false;
		};

		const auto has_named_members = (rules.is_object());

		for (const auto& proxy : rules.items())
		{
			const auto& declaration = proxy.key();
			const auto& content = proxy.value();

			if ((!has_named_members) || declaration.empty())
			{
				// Embedded (i.e. regular) trigger condition.
				const auto trigger_condition_expr = content["trigger"].get<std::string>();

				process_rule(trigger_condition_expr, content);
			}
			else
			{
				// In-place trigger condition.
				const auto& trigger_condition_expr = declaration;

				process_rule(trigger_condition_expr, content);
			}
		}

		// See above subroutine.
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

		auto [inline_command, state_path_raw, is_string_content] = parse_single_argument_command(command);

		if (state_path_raw.empty())
		{
			return nullptr;
		}

		switch (hash(inline_command))
		{
			case "import"_hs:
				// NOTE: Recursion.
				return resolve_state(*states_out, state_path_raw, *base_path);
		}

		return nullptr;
	}

	void EntityFactory::process_archetype(const util::json& data, const std::filesystem::path& base_path, bool resolve_external_modules)
	{
		if (resolve_external_modules)
		{
			resolve_archetypes(data, base_path);
		}

		if (auto components = data.find("components"); components != data.end())
		{
			process_component_list(descriptor.components, *components);
		}

		if (auto states = data.find("states"); states != data.end())
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