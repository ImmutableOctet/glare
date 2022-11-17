#include "entity_factory.hpp"

#include "components/relationship_component.hpp"
#include "components/instance_component.hpp"

#include "state/components/state_component.hpp"

#include <engine/meta/meta.hpp>

#include <util/algorithm.hpp>

#include <algorithm>

// TODO: Change to better regular expression library.
#include <regex>

// Debugging related:
#include <util/log.hpp>
#include <engine/components/type_component.hpp>

namespace engine
{
	EntityFactory::EntityFactory(const EntityFactoryContext& factory_context)
		: EntityFactoryContext(factory_context)
	{
		auto instance = util::load_json(paths.instance_path);

		process_archetype(instance, paths.instance_directory);
	}

	std::filesystem::path EntityFactory::resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const
	{
		auto get_surface_path = [this](const auto& path, const auto& base_path) -> std::filesystem::path
		{
			if (!base_path.empty())
			{
				if (auto projected_path = (base_path / path); std::filesystem::exists(projected_path))
				{
					return projected_path;
				}
			}

			if (auto projected_path = (paths.archetype_root_path / path); std::filesystem::exists(projected_path))
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

			return {};
		};

		auto module_path = get_surface_path(path, base_path);

		if (module_path.empty())
		{
			auto direct_path = path; direct_path.replace_extension("json");

			module_path = get_surface_path(direct_path, base_path);

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
	EntityFactory::parse_component_declaration(const std::string& component_declaration)
	{
		const auto component_rgx = std::regex("(\\+)?([^\\|]+)(\\|(\\d+))?");

		std::smatch rgx_match;

		std::string_view name_view = {};
		bool inplace_changes = false;
		std::optional<SmallSize> constructor_arg_count = std::nullopt;

		if (std::regex_search(component_declaration.begin(), component_declaration.end(), rgx_match, component_rgx))
		{
			if (rgx_match[1].matched)
			{
				inplace_changes = true;
			}

			name_view =
			{
				(component_declaration.data() + rgx_match.position(2)),
				static_cast<std::size_t>(rgx_match.length(2))
			};

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

	Entity EntityFactory::create(EntityConstructionContext context) const
	{
		using namespace entt::literals;

		if (context.entity == null)
		{
			context.entity = registry.create();
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
					entt::forward_as_meta(registry),
					entt::forward_as_meta(context.entity),
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
			RelationshipComponent::set_parent(registry, context.entity, context.parent, true);
		}

		registry.emplace_or_replace<InstanceComponent>(context.entity, paths.instance_path.string());

		if (default_state_index)
		{
			auto result = descriptor.set_state_by_index(registry, context.entity, std::nullopt, *default_state_index);

			if (!result)
			{
				print_warn("Unable to assign default state: Index #{}", *default_state_index);
			}
		}

		return context.entity;
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

	void EntityFactory::process_component_list
	(
		EntityDescriptor::TypeInfo& components_out,
		const util::json& components
	)
	{
		auto as_component = [this, &components_out](const auto& component_declaration, const util::json* component_content=nullptr)
		{
			auto [component_name, allow_inplace_changes, constructor_arg_count] = parse_component_declaration(component_declaration);

			process_component(components_out, component_name, component_content, allow_inplace_changes, constructor_arg_count);
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
	}

	bool EntityFactory::process_state
	(
		EntityDescriptor::StateCollection& states_out,
		const util::json& data,
		std::string_view state_name
	)
	{
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

		if (auto storage_list = util::find_any(data, "store", "storage", "local", "local_storage", "include", "temp", "temporary", "#"); storage_list != data.end())
		{
			state.build_storage(*storage_list);
		}

		if (state.name)
		{
			auto existing = descriptor.get_state(*state.name);

			if (existing)
			{
				*existing = std::move(state);

				return true;
			}
		}

		//descriptor.set_state(std::move(state));

		descriptor.states.emplace_back(std::move(state));

		return true;
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
			util::json_for_each(*states, [this, &base_path](const util::json& state_entry)
			{
				switch (state_entry.type())
				{
					case util::json::value_t::object:
					{
						const auto state_name = util::get_value<std::string>(state_entry, "name");

						process_state(descriptor.states, state_entry, state_name);

						break;
					}
					case util::json::value_t::string:
					{
						const auto state_path_raw = std::filesystem::path(state_entry.get<std::string>());
						const auto state_path = resolve_reference(state_path_raw, base_path);

						// TODO: Optimize.
						auto state_data = util::load_json(state_path);

						std::string state_name;

						if (auto name_it = state_data.find("name"); name_it != state_data.end())
						{
							state_name = name_it->get<std::string>();
						}
						else
						{
							state_name = state_path_raw.filename().string();
						}

						process_state(descriptor.states, state_data, state_name);

						break;
					}
				}
			});
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

	bool EntityFactory::resolve_archetypes(const util::json& instance, const std::filesystem::path& base_path)
	{
		auto archetypes = util::find_any(instance, "archetypes", "import", "modules"); // instance.find("archetypes");

		if (archetypes == instance.end())
		{
			return false;
		}

		auto elements_processed = util::json_for_each<util::json::value_t::string>
		(
			*archetypes,

			[this, &base_path](const auto& value)
			{
				const auto archetype_path_raw = std::filesystem::path(value.get<std::string>());
				const auto archetype_path = resolve_reference(archetype_path_raw, base_path);

				if (archetype_path.empty())
				{
					return;
				}

				// TODO: Optimize.
				auto archetype = util::load_json(archetype_path);

				process_archetype(archetype, archetype_path.parent_path());
			}
		);

		return (elements_processed > 0);
	}
}