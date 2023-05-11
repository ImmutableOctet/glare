#include "entity_state.hpp"
#include "entity_descriptor.hpp"
#include "entity_factory.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/component.hpp>
//#include <engine/meta/function.hpp>

#include <engine/meta/meta_type_resolution_context.hpp>

#include "components/state_component.hpp"
#include "components/state_storage_component.hpp"
#include "components/frozen_state_component.hpp"
#include "components/entity_thread_component.hpp"

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
		//assert(result == intended_count);

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

	void EntityState::update(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index, const EntityState* previous, std::optional<EntityStateIndex> prev_index, bool decay_prev_state, bool update_state_component) const
	{
		if (previous && decay_prev_state)
		{
			assert(prev_index);

			previous->decay(descriptor, registry, entity, *prev_index, &components.persist);
		}

		activate(descriptor, registry, entity, self_index, prev_index, update_state_component);
	}

	void EntityState::decay(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index, const MetaDescription* next_state_persist) const
	{
		decay_threads(descriptor, registry, entity, self_index);

		// Store state-local component instances.
		store(registry, entity, self_index);

		// Determine if we need to remove the components we added:
		if (decay_policy.remove_add_components)
		{
			// Enumerate components added by this state, cross-referencing the 'persistent components' lists
			// from both this state and the next state, to determine if removal is necessary:
			for (const auto& component_entry : components.add.type_definitions)
			{
				const auto& component = component_entry.get(descriptor);

				const auto type = component.get_type();

				if (next_state_persist)
				{
					if (next_state_persist->get_definition(descriptor, type))
					{
						continue;
					}
				}

				if (components.persist.get_definition(descriptor, type))
				{
					continue;
				}

				if (decay_policy.keep_modified_add_components && component.forces_field_assignment())
				{
					continue;
				}

				remove_component(registry, entity, type);
			}
		}

		// Unfreeze components we didn't want during this state's execution.
		unfreeze(registry, entity, self_index);

		// Ensure that persistent components are accounted for, but do not alter any existing state.
		// (This step is necessary due to the possibility of the next state being dependent on a now-removed component)
		persist(descriptor, registry, entity, false);
	}

	void EntityState::activate(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index, std::optional<EntityStateIndex> prev_index, bool update_state_component) const
	{
		copy(registry, entity, self_index);
		freeze(registry, entity, self_index);
		retrieve(registry, entity, self_index);
		remove(registry, entity);
		add(descriptor, registry, entity);
		persist(descriptor, registry, entity);

		if (update_state_component)
		{
			force_update_component(registry, entity, self_index, prev_index);
		}

		activate_threads(descriptor, registry, entity, self_index);
	}

	void EntityState::force_update_component(Registry& registry, Entity entity, EntityStateIndex self_index, std::optional<EntityStateIndex> prev_index) const
	{
		registry.emplace_or_replace<StateComponent>(entity, self_index, prev_index.value_or(self_index));
	}

	std::size_t EntityState::build_removals(const EntityDescriptor& descriptor, const util::json& removal_list, const MetaParsingContext& opt_parsing_context, bool cross_reference_persist)
	{
		return build_type_list(descriptor, removal_list, components.remove, cross_reference_persist, opt_parsing_context);
	}

	std::size_t EntityState::build_frozen(const EntityDescriptor& descriptor, const util::json& frozen_list, const MetaParsingContext& opt_parsing_context, bool cross_reference_persist)
	{
		return build_type_list(descriptor, frozen_list, components.freeze, cross_reference_persist, opt_parsing_context);
	}

	std::size_t EntityState::build_storage(const EntityDescriptor& descriptor, const util::json& storage_list, const MetaParsingContext& opt_parsing_context, bool cross_reference_persist)
	{
		return build_type_list(descriptor, storage_list, components.store, cross_reference_persist, opt_parsing_context);
	}

	std::size_t EntityState::build_local_copy(const EntityDescriptor& descriptor, const util::json& local_copy_list, const MetaParsingContext& opt_parsing_context, bool cross_reference_persist)
	{
		const auto local_copy_result    = build_type_list(descriptor, local_copy_list, components.local_copy, cross_reference_persist, opt_parsing_context);
		const auto freeze_append_result = build_type_list(descriptor, local_copy_list, components.freeze, cross_reference_persist, opt_parsing_context);

		assert(local_copy_result == freeze_append_result);

		return local_copy_result;
	}

	std::size_t EntityState::build_init_copy(const EntityDescriptor& descriptor, const util::json& init_copy_list, const MetaParsingContext& opt_parsing_context, bool cross_reference_persist)
	{
		const auto init_copy_result     = build_type_list(descriptor, init_copy_list, components.init_copy, cross_reference_persist, opt_parsing_context);
		const auto freeze_append_result = build_type_list(descriptor, init_copy_list, components.freeze, cross_reference_persist, opt_parsing_context);
		const auto store_append_result  = build_type_list(descriptor, init_copy_list, components.store, cross_reference_persist, opt_parsing_context);

		assert(freeze_append_result == init_copy_result);
		assert(store_append_result == init_copy_result);

		return init_copy_result;
	}

	bool EntityState::process_type_list_entry(const EntityDescriptor& descriptor, MetaIDStorage& types_out, const util::json& list_entry, bool cross_reference_persist, const MetaParsingContext& opt_parsing_context)
	{
		if (!list_entry.is_string())
		{
			return false;
		}

		const auto component_name = list_entry.get<std::string>();

		return process_type_list_entry(descriptor, types_out, std::string_view(component_name), cross_reference_persist, opt_parsing_context);
	}

	bool EntityState::process_type_list_entry(const EntityDescriptor& descriptor, MetaIDStorage& types_out, std::string_view component_name, bool cross_reference_persist, const MetaParsingContext& opt_parsing_context)
	{
		MetaType component_type;
		MetaTypeID component_type_id = {};

		if (const auto opt_type_context = opt_parsing_context.get_type_context())
		{
			component_type = opt_type_context->get_component_type(component_name);

			if (component_type)
			{
				component_type_id = component_type.id();
			}
		}
		else
		{
			component_type_id = hash(component_name).value();
			component_type = resolve(component_type_id);
		}

		if (!component_type)
		{
			print_warn("Unable to resolve component type: {} (#{})", component_name, component_type_id);

			return false;
		}

		if (cross_reference_persist)
		{
			if (components.persist.get_definition(descriptor, component_type))
			{
				print_warn("Component entry ignored due to overlapping persistent entry. ({}, #{})", component_name, component_type_id);

				return false;
			}
		}

		types_out.emplace_back(component_type_id); // std::move(component_type)

		return true;
	}

	std::size_t EntityState::build_type_list(const EntityDescriptor& descriptor, const util::json& type_names, MetaIDStorage& types_out, bool cross_reference_persist, const MetaParsingContext& opt_parsing_context)
	{
		std::size_t count = 0;

		util::json_for_each(type_names, [this, &descriptor, &types_out, &count, cross_reference_persist, &opt_parsing_context](const util::json& list_entry)
		{
			switch (list_entry.type())
			{
				case util::json::value_t::object:
					for (const auto& proxy : list_entry.items())
					{
						const auto& component_name = proxy.key();

						if (process_type_list_entry(descriptor, types_out, std::string_view(component_name), cross_reference_persist, opt_parsing_context))
						{
							count++;
						}
					}

					break;
				default:
					if (process_type_list_entry(descriptor, types_out, list_entry, cross_reference_persist, opt_parsing_context))
					{
						count++;
					}

					break;
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
		using namespace engine::literals;

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
		return remove_component(registry, entity, component.get_type());
	}

	bool EntityState::has_component(Registry& registry, Entity entity, const MetaType& type) const
	{
		using namespace engine::literals;

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

	void EntityState::add(const EntityDescriptor& descriptor, Registry& registry, Entity entity) const
	{
		for (const auto& component_entry : components.add.type_definitions)
		{
			const auto& component = component_entry.get(descriptor);

			const auto type_id = component.get_type_id();

			if (component.forces_field_assignment() || components.persist.get_definition(descriptor, type_id))
			{
				update_component_fields(registry, entity, component);
			}
			else
			{
				emplace_component(registry, entity, component);
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

	void EntityState::persist(const EntityDescriptor& descriptor, Registry& registry, Entity entity, bool value_assignment) const
	{
		for (const auto& component_entry : components.persist.type_definitions)
		{
			const auto& component = component_entry.get(descriptor);

			if (!update_component_fields(registry, entity, component, value_assignment))
			{
				emplace_component(registry, entity, component);
			}
		}
	}

	void EntityState::activate_threads(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index) const
	{
		if (immediate_threads.empty())
		{
			return;
		}

		auto& thread_component = registry.get_or_emplace<EntityThreadComponent>(entity);

		// TODO: Change to use of `EntityThreadSpawnCommand` instead of direct call.
		thread_component.start_threads(descriptor, *this, self_index);
	}

	void EntityState::decay_threads(const EntityDescriptor& descriptor, Registry& registry, Entity entity, EntityStateIndex self_index) const
	{
		auto* thread_component = registry.try_get<EntityThreadComponent>(entity);

		if (!thread_component)
		{
			return;
		}

		thread_component->stop_threads(*this, self_index); // descriptor
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

	StateStorageComponent& EntityState::copy(Registry& registry, Entity entity, EntityStateIndex self_index) const
	{
		auto& container = registry.get_or_emplace<StateStorageComponent>(entity);
		auto& storage = container.get_storage(self_index);

		// NOTE: To implement local copies, the items in `local_copy` are also
		// added to `freeze` during the state's generation phase prior.
		storage.store(registry, entity, components.local_copy, true, false);

		storage.store(registry, entity, components.init_copy, true, true);

		return container;
	}
}