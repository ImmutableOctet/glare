#include "component.hpp"

#include "hash.hpp"
//#include "function.hpp"

#include "meta_type_descriptor.hpp"
#include "meta_evaluation_context.hpp"

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	namespace impl
	{
		static bool history_component_action(MetaAny& history_component, MetaSymbolID function_id, auto&&... args)
		{
			if (!history_component)
			{
				return false;
			}

			const auto history_component_type = history_component.type();

			if (!history_component_type)
			{
				return false;
			}

			auto store_fn = history_component_type.func(function_id);

			while (store_fn)
			{
				auto result = store_fn.invoke // invoke_any_overload
				(
					history_component,

					entt::forward_as_meta(args)...
				);

				if (result)
				{
					if (const auto result_as_bool = result.try_cast<bool>())
					{
						return *result_as_bool;
					}
				}

				store_fn = store_fn.next();
			}

			return false;
		}
	}

	MetaAny get_component_ref(Registry& registry, Entity entity, const MetaType& component_type)
	{
		using namespace engine::literals;

		if (!component_type)
		{
			return {};
		}

		if (entity == null)
		{
			return {};
		}

		auto get_component_fn = component_type.func("get_component"_hs);

		while (get_component_fn)
		{
			auto component_ptr = get_component_fn.invoke // invoke_any_overload
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (component_ptr)
			{
				return *component_ptr;
			}

			get_component_fn = get_component_fn.next();
		}

		return {};
	}

	MetaAny get_component_ref(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return get_component_ref(registry, entity, resolve(component_type_id));
	}

	MetaAny get_or_emplace_component(Registry& registry, Entity entity, const MetaType& component_type)
	{
		using namespace engine::literals;

		/*
		// Alternative implementation:
		if (auto existing_component = get_component_ref(registry, entity, component_type))
		{
			return existing_component;
		}

		return emplace_default_component(registry, entity, component_type);
		*/

		if (!component_type)
		{
			return {};
		}

		if (entity == null)
		{
			return {};
		}

		auto emplace_component_fn = component_type.func("get_or_default_construct_component"_hs);

		while (emplace_component_fn)
		{
			auto component_ptr = emplace_component_fn.invoke // invoke_any_overload
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (component_ptr)
			{
				return *component_ptr;
			}

			emplace_component_fn = emplace_component_fn.next();
		}

		return {};
	}

	MetaAny get_or_emplace_component(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return get_or_emplace_component(registry, entity, resolve(component_type_id));
	}

	// Attempts to emplace a default-constructed instance of `component_type` to `entity`.
	MetaAny emplace_default_component(Registry& registry, Entity entity, const MetaType& component_type)
	{
		using namespace engine::literals;

		if (!component_type)
		{
			return {};
		}

		if (entity == null)
		{
			return {};
		}

		auto default_component_fn = component_type.func("emplace_default_component"_hs);

		while (default_component_fn)
		{
			auto component_ptr = default_component_fn.invoke // invoke_any_overload
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (component_ptr)
			{
				return *component_ptr;
			}

			default_component_fn = default_component_fn.next();
		}

		return {};
	}

	MetaAny emplace_default_component(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return emplace_default_component(registry, entity, resolve(component_type_id));
	}

	MetaAny emplace_component
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,
		const MetaEvaluationContext* opt_evaluation_context
	)
	{
		using namespace engine::literals;

		auto instance = MetaAny {};

		if (opt_evaluation_context)
		{
			instance = component.instance(true, registry, entity, *opt_evaluation_context);
		}
		else
		{
			instance = component.instance(true, registry, entity);
		}

		//assert(instance);

		auto result = MetaAny {};

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

	bool update_component_fields
	(
		Registry& registry, Entity entity,
		const MetaTypeDescriptor& component,
		
		bool value_assignment, bool direct_modify,
		const MetaEvaluationContext* opt_evaluation_context
	)
	{
		using namespace engine::literals;

		auto type = component.get_type();

		if (value_assignment && (!direct_modify))
		{
			auto patch_fn = type.func("indirect_patch_meta_component"_hs);

			if (!patch_fn)
			{
				print_warn("Unable to resolve patch function for: #{}", type.id());

				return false;
			}

			if (component.size() > 0)
			{
				auto result = patch_fn.invoke
				(
					{},
					
					entt::forward_as_meta(registry),
					entt::forward_as_meta(entity),
					entt::forward_as_meta(component),
					entt::forward_as_meta(component.size()),
					entt::forward_as_meta(0),
					entt::forward_as_meta(Entity(null)),
					entt::forward_as_meta(opt_evaluation_context),
					entt::forward_as_meta(MetaAny {})
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
						const auto result = component.apply_fields(instance);

						//assert(result == component.size());

						if (result != component.size())
						{
							print_warn("Unable to apply all of the specified fields while updating component #{}", type.id());

							if (result == 0)
							{
								return false;
							}
						}

						if (auto notify_patch_fn = type.func("mark_component_as_patched"_hs))
						{
							auto notify_result = notify_patch_fn.invoke
							(
								{},

								entt::forward_as_meta(registry),
								entt::forward_as_meta(entity)
							);

							if (!notify_result)
							{
								print_warn("Failed to notify listeners of patch for component #{}", type.id());
							}
						}
						else
						{
							print_warn("Unable to resolve component-patch notification function from: #{}", type.id());
						}
					}
				}

				return true;
			}
		}

		return false;
	}

	// Attempts to mark a component of type `component_type` as patched (i.e. updated).
	// The return value of this function indicates if the operation was performed successfully.
	bool mark_component_as_patched(Registry& registry, Entity entity, const MetaType& component_type)
	{
		using namespace engine::literals;

		if (!component_type)
		{
			return false;
		}

		auto mark_fn = component_type.func("mark_component_as_patched"_hs);

		while (mark_fn)
		{
			auto result = mark_fn.invoke
			(
				{},

				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (result)
			{
				const auto result_raw = result.try_cast<bool>();

				return (*result_raw);
			}

			mark_fn = mark_fn.next();
		}

		return false;
	}

	bool mark_component_as_patched(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return mark_component_as_patched(registry, entity, resolve(component_type_id));
	}

	// Retrieves a `MetaTypeID` referencing a specialization of `HistoryComponent`, based on `component_type`.
	MetaTypeID get_history_component_type_id(const MetaType& component_type)
	{
		using namespace engine::literals;

		if (!component_type)
		{
			return {};
		}

		if (auto lookup_fn = component_type.func("get_history_component_type_id"_hs))
		{
			if (auto result = lookup_fn.invoke({}))
			{
				if (const auto as_type_id = result.try_cast<MetaTypeID>())
				{
					return (*as_type_id);
				}
			}
		}
		
		return {};
	}

	// Retrieves a `MetaTypeID` referencing a specialization of `HistoryComponent`,
	// based on the type identified by `component_type_id`.
	MetaTypeID get_history_component_type_id(MetaTypeID component_type_id)
	{
		return get_history_component_type_id(resolve(component_type_id));
	}

	MetaType get_history_component_type(const MetaType& component_type)
	{
		if (!component_type)
		{
			return {};
		}

		if (auto history_component_type_id = get_history_component_type_id(component_type))
		{
			return resolve(history_component_type_id);
		}

		return {};
	}

	MetaType get_history_component_type(MetaTypeID component_type_id)
	{
		return get_history_component_type(resolve(component_type_id));
	}

	MetaAny get_or_emplace_history_component(Registry& registry, Entity entity, const MetaType& component_type)
	{
		if (!component_type)
		{
			return {};
		}

		if (const auto history_component_type = get_history_component_type(component_type))
		{
			return get_or_emplace_component(registry, entity, history_component_type);
		}
		
		return {};
	}

	MetaAny get_or_emplace_history_component(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		/*
		// Alternative implementation:
		if (const auto history_component_type_id = get_history_component_type_id(component_type_id))
		{
			return get_or_emplace_component(registry, entity, history_component_type_id);
		}
		*/

		return get_or_emplace_history_component(registry, entity, resolve(component_type_id));
	}

	MetaAny get_history_component_ref(Registry& registry, Entity entity, const MetaType& component_type)
	{
		if (!component_type)
		{
			return {};
		}

		if (const auto history_component_type = get_history_component_type(component_type))
		{
			return get_component_ref(registry, entity, history_component_type);
		}

		return {};
	}

	MetaAny get_history_component_ref(Registry& registry, Entity entity, MetaTypeID component_type_id)
	{
		return get_history_component_ref(registry, entity, resolve(component_type_id));
	}

	bool undo_component_change(Registry& registry, Entity entity, MetaAny& history_component)
	{
		using namespace engine::literals;

		return impl::history_component_action(history_component, "undo"_hs, registry, entity);
	}

	bool redo_component_change(Registry& registry, Entity entity, MetaAny& history_component)
	{
		using namespace engine::literals;

		return impl::history_component_action(history_component, "redo"_hs, registry, entity);
	}

	bool store_component_history(Registry& registry, Entity entity, MetaAny& history_component)
	{
		using namespace engine::literals;

		return impl::history_component_action(history_component, "store"_hs, registry, entity);
	}

	bool truncate_component_history(MetaAny& history_component)
	{
		using namespace engine::literals;

		return impl::history_component_action(history_component, "truncate"_hs);
	}

	bool component_history_store_back(Registry& registry, Entity entity, MetaAny& history_component)
	{
		using namespace engine::literals;

		return impl::history_component_action(history_component, "store_back"_hs, registry, entity);
	}

	bool component_history_truncate_back(Registry& registry, Entity entity, MetaAny& history_component, bool update_cursor)
	{
		using namespace engine::literals;

		return impl::history_component_action(history_component, "truncate_back"_hs, update_cursor);
	}

	bool component_history_truncate_back(Registry& registry, Entity entity, MetaAny& history_component)
	{
		using namespace engine::literals;

		return impl::history_component_action(history_component, "truncate_back"_hs);
	}
}