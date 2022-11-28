#include "entity_state.hpp"

#include "meta/meta.hpp"

#include "state/components/state_component.hpp"
#include "state/components/state_storage_component.hpp"
#include "state/components/frozen_state_component.hpp"

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	template <typename StorageComponentType>
	static StorageComponentType& store_components(Registry& registry, Entity entity, EntityStateIndex self_index, const MetaStorageDescription& components)
	{
		auto& container = registry.get_or_emplace<StorageComponentType>(entity);
		auto& storage = container.get_storage(self_index);

		const auto intended_count = components.size();
		const auto result = storage.store(registry, entity, components);

		// Ensure the correct number of components were stored.
		assert(result == intended_count);

		return container;
	}

	template <typename StorageComponentType>
	static StorageComponentType& retrieve_components(Registry& registry, Entity entity, EntityStateIndex self_index)
	{
		auto& container = registry.get_or_emplace<StorageComponentType>(entity); // registry.try_get<StorageComponentType>(entity);

		auto& storage = container.get_storage(self_index);

		const auto intended_count = storage.components.size();
		const auto result = storage.retrieve(registry, entity);

		// Ensure the correct number of components were retrieved.
		assert(result == intended_count);

		return container;
	}

	void EntityState::update(Registry& registry, Entity entity, EntityStateIndex self_index, const EntityState* previous, std::optional<EntityStateIndex> prev_index) const
	{
		if (previous)
		{
			assert(prev_index);

			previous->decay(registry, entity, *prev_index, &components.persist);
		}

		freeze(registry, entity, self_index);
		retrieve(registry, entity, self_index);
		remove(registry, entity);
		add(registry, entity);
		persist(registry, entity);

		registry.emplace_or_replace<StateComponent>(entity, self_index, prev_index.value_or(self_index));
	}

	void EntityState::decay(Registry& registry, Entity entity, EntityStateIndex self_index, const MetaDescription* next_state_persist) const
	{
		store(registry, entity, self_index);

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

		unfreeze(registry, entity, self_index);

		// Ensure that persistent components are accounted for,
		// but do not alter any existing state.
		persist(registry, entity, false);
	}

	std::size_t EntityState::build_removals(const util::json& removal_list, bool cross_reference_persist)
	{
		return build_type_list(removal_list, components.remove, cross_reference_persist);
	}

	std::size_t EntityState::build_frozen(const util::json& frozen_list, bool cross_reference_persist)
	{
		return build_type_list(frozen_list, components.freeze, cross_reference_persist);
	}

	std::size_t EntityState::build_storage(const util::json& storage_list, bool cross_reference_persist)
	{
		return build_type_list(storage_list, components.store, cross_reference_persist);
	}

	bool EntityState::process_type_list_entry(MetaIDStorage& types_out, const util::json& list_entry, bool cross_reference_persist)
	{
		if (!list_entry.is_string())
		{
			return false;
		}

		const auto component_name = list_entry.get<std::string>();

		return process_type_list_entry(types_out, std::string_view(component_name), cross_reference_persist);
	}

	bool EntityState::process_type_list_entry(MetaIDStorage& types_out, std::string_view component_name, bool cross_reference_persist)
	{
		const auto component_type_hash = hash(component_name).value();

		auto component_type = resolve(component_type_hash);

		if (!component_type)
		{
			print_warn("Unable to resolve component type: {} (#{})", component_name, component_type_hash);

			return false;
		}

		if (cross_reference_persist)
		{
			if (components.persist.get_definition(component_type))
			{
				print_warn("Component entry ignored due to overlapping persistent entry. ({}, #{})", component_name, component_type_hash);

				return false;
			}
		}

		types_out.emplace_back(component_type_hash); // std::move(component_type)

		return true;
	}

	std::size_t EntityState::build_type_list(const util::json& type_names, MetaIDStorage& types_out, bool cross_reference_persist)
	{
		std::size_t count = 0;

		util::json_for_each(type_names, [this, &types_out, &count, cross_reference_persist](const util::json& list_entry)
		{
			if (process_type_list_entry(types_out, list_entry, cross_reference_persist))
			{
				count++;
			}
		});

		return count;
	}

	const EntityState::RuleCollection* EntityState::get_rules(MetaTypeID type_id) const
	{
		auto it = rules.find(type_id);

		if (it != rules.end())
		{
			return &it->second;
		}

		return nullptr;
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

		//return true;
		return result.cast<bool>();
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

	bool EntityState::update_component_fields(Registry& registry, Entity entity, const MetaTypeDescriptor& component, bool value_assignment, bool direct_modify) const
	{
		using namespace entt::literals;

		auto& type = component.type;

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

	FrozenStateComponent& EntityState::freeze(Registry& registry, Entity entity, EntityStateIndex self_index) const
	{
		return store_components<FrozenStateComponent>(registry, entity, self_index, components.freeze);
	}

	FrozenStateComponent& EntityState::unfreeze(Registry& registry, Entity entity, EntityStateIndex self_index) const
	{
		return retrieve_components<FrozenStateComponent>(registry, entity, self_index);
	}

	StateStorageComponent& EntityState::store(Registry& registry, Entity entity, EntityStateIndex self_index) const
	{
		return store_components<StateStorageComponent>(registry, entity, self_index, components.store);
	}

	StateStorageComponent& EntityState::retrieve(Registry& registry, Entity entity, EntityStateIndex self_index) const
	{
		return retrieve_components<StateStorageComponent>(registry, entity, self_index);
	}
}