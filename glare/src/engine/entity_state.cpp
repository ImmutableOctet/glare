#include "entity_state.hpp"

#include "meta/meta.hpp"

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	void EntityState::update(Registry& registry, Entity entity, const EntityState* previous) const
	{
		if (previous)
		{
			previous->decay(registry, entity, &components.persist);
		}

		remove(registry, entity);
		add(registry, entity);
		persist(registry, entity);
	}

	void EntityState::build_removals(const util::json& removal_list, bool cross_reference_persist)
	{
		util::json_for_each(removal_list, [this, cross_reference_persist](const util::json& removal_entry)
		{
			if (!removal_entry.is_string())
			{
				return;
			}

			const auto component_name = removal_entry.get<std::string>();
			const auto component_type_hash = hash(component_name);

			auto component_type = resolve(component_type_hash);

			if (!component_type)
			{
				print_warn("Unable to resolve component type: {} (#{})", component_name, component_type_hash);

				return;
			}

			if (cross_reference_persist)
			{
				if (components.persist.get_definition(component_type))
				{
					print_warn("State removal-entry ignored due to overlapping persistent entry. ({}, #{})", component_name, component_type_hash);

					return;
				}
			}

			components.remove.emplace_back(component_type_hash); // std::move(component_type)
		});
	}

	bool EntityState::remove_component(Registry& registry, Entity entity, const MetaType& type) const
	{
		using namespace entt::literals;

		auto remove_fn = type.func("remove_component"_hs);

		if (!remove_fn)
		{
			print_warn("Unable to resolve component removal function for: #{}", type.id());

			return false;
		}

		auto result = remove_fn.invoke
		(
			{},
			entt::forward_as_meta(registry),
			entt::forward_as_meta(entity)
		);

		if (!result)
		{
			print_warn("Failed to remove component: #{}", type.id());

			return false;
		}

		return true;
	}

	bool EntityState::remove_component(Registry& registry, Entity entity, const MetaTypeDescriptor& component) const
	{
		return remove_component(registry, entity, component.type);
	}

	bool EntityState::has_component(Registry& registry, Entity entity, const MetaType& type) const
	{
		using namespace entt::literals;

		auto get_fn = type.func("get_component"_hs);

		if (!get_fn)
		{
			print_warn("Unable to resolve component-get function from: #{}", type.id());

			return false;
		}

		auto result = get_fn.invoke(registry, entity);

		//return static_cast<bool>(*result);

		return (*result).cast<bool>();
	}

	bool EntityState::add_component(Registry& registry, Entity entity, const MetaTypeDescriptor& component) const
	{
		using namespace entt::literals;

		auto& type = component.type;

		auto emplace_fn = type.func("emplace_meta_component"_hs);

		if (!emplace_fn)
		{
			print_warn("Unable to resolve component-addition function for: #{}", type.id());

			return false;
		}

		auto instance = component.instance();

		if (!instance)
		{
			print_warn("Failed to create instance of component type: #{}", type.id());

			return false;
		}

		emplace_fn.invoke
		(
			{},

			entt::forward_as_meta(registry),
			entt::forward_as_meta(entity),
			entt::forward_as_meta(std::move(instance))
		);

		return true;
	}

	bool EntityState::update_component_fields(Registry& registry, Entity entity, const MetaTypeDescriptor& component, bool value_assignment) const
	{
		using namespace entt::literals;

		auto& type = component.type;

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
			if (value_assignment)
			{
				if (component.size() > 0)
				{
					return (component.apply_fields(instance) > 0);
				}
			}

			return true;
		}

		return false;
	}

	void EntityState::decay(Registry& registry, Entity entity, const MetaDescription* next_state_persist) const
	{
		for (const auto& component : components.add.type_definitions)
		{
			auto& type = component.type;

			if (next_state_persist)
			{
				if (next_state_persist->get_definition(type))
				{
					continue;
				}
			}

			if (components.persist.get_definition(type))
			{
				continue;
			}

			remove_component(registry, entity, type);
		}

		// Ensure that persistent components are accounted for,
		// but do not alter any existing state.
		persist(registry, entity, false);
	}

	void EntityState::add(Registry& registry, Entity entity) const
	{
		for (const auto& component : components.add.type_definitions)
		{
			const auto& type = component.type;

			if (components.persist.get_definition(type))
			{
				update_component_fields(registry, entity, component);
			}
			else
			{
				add_component(registry, entity, component);
			}
		}
	}

	void EntityState::remove(Registry& registry, Entity entity) const
	{
		for (const auto& component_type_id : components.remove)
		{
			auto type = resolve(component_type_id);

			if (!type)
			{
				print_warn("Unable to resolve type for: #{}", component_type_id);

				continue;
			}

			remove_component(registry, entity, type);
		}
	}

	void EntityState::persist(Registry& registry, Entity entity, bool value_assignment) const
	{
		for (const auto& component : components.persist.type_definitions)
		{
			if (!update_component_fields(registry, entity, component, value_assignment))
			{
				add_component(registry, entity, component);
			}
		}
	}
}