#include "component_storage.hpp"

#include "meta.hpp"

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	std::size_t ComponentStorage::store(Registry& registry, Entity entity, const MetaStorageDescription& component_details, bool store_as_copy, bool skip_existing)
	{
		using namespace entt::literals;

		if (component_details.empty())
		{
			return 0;
		}

		std::size_t count = 0;

		for (const auto& component_entry : component_details)
		{
			if (skip_existing)
			{
				bool already_exists = false;

				for (const auto& existing : components)
				{
					if (existing.type().id() == component_entry) // (existing.type() == component_type)
					{
						already_exists = true;

						break;
					}
				}

				if (already_exists)
				{
					continue;
				}
			}

			auto component_type = resolve(component_entry);

			if (!component_type)
			{
				print_warn("Unable to resolve component #{} during storage operation.", component_type.id()); // component_entry

				continue;
			}

			auto store_fn = component_type.func
			(
				(store_as_copy)
				? "copy_meta_component"_hs
				: "store_meta_component"_hs
			);

			if (!store_fn)
			{
				print_warn("Unable to resolve storage-function for component: #{}", component_type.id()); // component_entry

				continue;
			}

			auto instance = store_fn.invoke
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (!instance)
			{
				print_warn("Unable to store instance of component #{} during storage operation. (maybe try `persist` instead of `add`?)", component_type.id()); // component_entry

				continue;
			}

			components.emplace_back(std::move(instance));

			count++;
		}

		return count;
	}

	std::size_t ComponentStorage::retrieve(Registry& registry, Entity entity)
	{
		using namespace entt::literals;

		if (empty())
		{
			return 0;
		}

		std::size_t count = 0;

		for (auto& instance : components)
		{
			if (!instance)
			{
				print_warn("Invalid instance detected in state-storage.");

				continue;
			}

			auto component_type = instance.type();

			if (!component_type)
			{
				print_warn("Unable to resolve meta-type during storage-retrieval operation.");

				continue;
			}

			auto restore_fn = component_type.func("emplace_meta_component"_hs);

			if (!restore_fn)
			{
				print_warn("Unable to resolve component restoration function during storage-retrieval operation.");

				continue;
			}

			auto result = restore_fn.invoke
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity),
				entt::forward_as_meta(std::move(instance))
			);

			if (!result)
			{
				print_warn("Unexpected failure during component restoration.");

				continue;
			}

			count++;
		}

		clear();

		return count;
	}

	bool ComponentStorage::empty() const
	{
		return components.empty();
	}

	void ComponentStorage::clear()
	{
		components.clear();
	}
}