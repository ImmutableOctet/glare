#include "event_trigger_condition.hpp"

#include "entity_target.hpp"
#include "entity_descriptor.hpp"

#include "components/instance_component.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/string.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/data_member.hpp>

#include <engine/meta/meta_data_member.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>
#include <engine/meta/apply_operation.hpp>

#include <optional>
#include <utility>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	// EventTriggerConditionType:

	// Internal subroutine; implementation of `compare`.
	// Indirection is not explicitly resolved for `current_value` and `comparison_value`, but may happen during boolean type conversion.
	template <typename ...Args>
	static bool compare_impl(const MetaAny& current_value, const MetaAny& comparison_value, EventTriggerConditionType::ComparisonMethod comparison_method, Args&&... args)
	{
		using ComparisonMethod = EventTriggerConditionType::ComparisonMethod;

		switch (comparison_method)
		{
			case ComparisonMethod::Equal:
			{
				if (current_value == comparison_value)
				{
					return true;
				}

				if (meta_any_string_compare(current_value, comparison_value))
				{
					return true;
				}

				// Fallback to `apply_operation` control-path.
				break;
			}

			case ComparisonMethod::NotEqual:
			{
				/*
				if (current_value != comparison_value)
				{
					return !meta_any_string_compare(current_value, comparison_value);
				}
				*/

				// NOTE: Recursion.
				return !compare_impl(current_value, comparison_value, ComparisonMethod::Equal, args...);
			}

			case ComparisonMethod::LessThan:
			case ComparisonMethod::LessThanOrEqual:
			case ComparisonMethod::GreaterThan:
			case ComparisonMethod::GreaterThanOrEqual:
				// Immediately fallback to `apply_operation` control-path.
				break;

			/*
			default:
				print_error("Unsupported comparison method detected: {}", static_cast<std::uint32_t>(comparison_method));

				break; // return false;
			*/
		}

		if (auto result = apply_operation(current_value, comparison_value, EventTriggerConditionType::operation_from_comparison_method(comparison_method)))
		{
			if (result.allow_cast<bool>())
			{
				return result.cast<bool>();
			}
		}

		return false;
	}

	const EntityDescriptor& EventTriggerConditionType::get_descriptor(Registry& registry, Entity entity)
	{
		auto& instance_comp = registry.get<InstanceComponent>(entity);

		return instance_comp.get_descriptor();
	}

	const EntityDescriptor* EventTriggerConditionType::try_get_descriptor(Registry& registry, Entity entity)
	{
		const auto* instance_comp = registry.try_get<InstanceComponent>(entity);

		if (!instance_comp)
		{
			return {};
		}

		if (!instance_comp->instance)
		{
			return {};
		}

		return &(instance_comp->get_descriptor());
	}

	const EntityDescriptor& EventTriggerConditionType::get_descriptor(Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return get_descriptor(registry, entity);
	}

	const EntityDescriptor* EventTriggerConditionType::try_get_descriptor(Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return try_get_descriptor(registry, entity);
	}

	const EventTriggerCondition& EventTriggerConditionType::from_remote_condition(const EntityDescriptor& descriptor, const RemoteConditionType& condition)
	{
		return condition.get(descriptor);
	}

	const EventTriggerConditionType* EventTriggerConditionType::get_condition_interface(const MetaAny& condition)
	{
		using namespace engine::literals;

		if (!condition)
		{
			return {};
		}

		const auto type = condition.type();

		if (!type)
		{
			return {};
		}

		switch (type.id())
		{
			case "EventTriggerSingleCondition"_hs:
			case "EventTriggerMemberCondition"_hs:
			case "EventTriggerAndCondition"_hs:
			case "EventTriggerOrCondition"_hs:
			case "EventTriggerTrueCondition"_hs:
			case "EventTriggerFalseCondition"_hs:
			case "EventTriggerInverseCondition"_hs:
				return reinterpret_cast<const EventTriggerConditionType*>(condition.data());

			//case "EventTriggerCondition"_hs:
				// ...
		}

		return {};
	}

	const EventTriggerConditionType* EventTriggerConditionType::get_condition_interface(const MetaAny& condition_any, Registry& registry, Entity entity)
	{
		const EventTriggerConditionType* condition_interface = nullptr;

		if (auto underlying = try_get_underlying_value(condition_any, registry, entity))
		{
			condition_interface = get_condition_interface(underlying);
		}

		if (!condition_interface)
		{
			condition_interface = get_condition_interface(condition_any);
		}

		return condition_interface;
	}

	EventTriggerConditionType::ComparisonMethod EventTriggerConditionType::get_comparison_method(std::string_view comparison_operator)
	{
		using namespace engine::literals;

		if (comparison_operator.empty())
		{
			return ComparisonMethod::Equal;
		}

		switch (hash(comparison_operator))
		{
			case "!="_hs:
			case "!=="_hs:
			case "<>"_hs:
				return ComparisonMethod::NotEqual;

			case "<"_hs:
				return ComparisonMethod::LessThan;

			case "<="_hs:
				return ComparisonMethod::LessThanOrEqual;

			case ">"_hs:
				return ComparisonMethod::GreaterThan;

			case ">="_hs:
				return ComparisonMethod::GreaterThanOrEqual;

			/*
			case "="_hs:
			case "|"_hs:
				return ComparisonMethod::Equal;
			*/
		}

		return ComparisonMethod::Equal;
	}

	EventTriggerConditionType::ComparisonMethod EventTriggerConditionType::get_comparison_method(std::string_view comparison_operator, bool apply_inversion)
	{
		const auto comparison_method = get_comparison_method(comparison_operator);

		if (apply_inversion)
		{
			return invert_comparison_method(comparison_method);
		}

		return comparison_method;
	}

	EventTriggerConditionType::ComparisonMethod EventTriggerConditionType::invert_comparison_method(ComparisonMethod comparison_method)
	{
		switch (comparison_method)
		{
			case ComparisonMethod::Equal:
				return ComparisonMethod::NotEqual;
			
			case ComparisonMethod::NotEqual:
				return ComparisonMethod::Equal;

			case ComparisonMethod::LessThan:
				return ComparisonMethod::GreaterThanOrEqual;

			case ComparisonMethod::LessThanOrEqual:
				return ComparisonMethod::GreaterThan;

			case ComparisonMethod::GreaterThan:
				return ComparisonMethod::LessThanOrEqual;

			case ComparisonMethod::GreaterThanOrEqual:
				return ComparisonMethod::LessThan;
		}

		assert(false);

		return ComparisonMethod::NotEqual;
	}

	EventTriggerConditionType::~EventTriggerConditionType() {}

	MetaAny EventTriggerConditionType::get_component(const MetaType& component_type, Registry& registry, Entity entity)
	{
		using namespace engine::literals;

		if (!component_type)
		{
			return {};
		}

		auto get_fn = component_type.func("get_component"_hs);

		if (!get_fn)
		{
			return {};
		}

		auto component_ptr = get_fn.invoke
		(
			{},

			entt::forward_as_meta(registry),
			entt::forward_as_meta(entity)
		);

		if (component_ptr)
		{
			return *component_ptr;
		}

		return {};
	}

	MetaAny EventTriggerConditionType::get_component(const MetaType& component_type, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return get_component(component_type, registry, entity);
	}

	MetaValueOperator EventTriggerConditionType::operation_from_comparison_method(EventTriggerComparisonMethod comparison_method)
	{
		switch (comparison_method)
		{
			case EventTriggerComparisonMethod::Equal:
				return MetaValueOperator::Equal;
			case EventTriggerComparisonMethod::NotEqual:
				return MetaValueOperator::NotEqual;
			case EventTriggerComparisonMethod::LessThan:
				return MetaValueOperator::LessThan;
			case EventTriggerComparisonMethod::LessThanOrEqual:
				return MetaValueOperator::LessThanOrEqual;
			case EventTriggerComparisonMethod::GreaterThan:
				return MetaValueOperator::GreaterThan;
			case EventTriggerComparisonMethod::GreaterThanOrEqual:
				return MetaValueOperator::GreaterThanOrEqual;
		}

		return MetaValueOperator::Equal;
	}

	std::optional<EventTriggerComparisonMethod> EventTriggerConditionType::comparison_method_from_operation(MetaValueOperator operation)
	{
		switch (operation)
		{
			case MetaValueOperator::Equal:
				return EventTriggerComparisonMethod::Equal;
			case MetaValueOperator::NotEqual:
				return EventTriggerComparisonMethod::NotEqual;
			case MetaValueOperator::LessThan:
				return EventTriggerComparisonMethod::LessThan;
			case MetaValueOperator::LessThanOrEqual:
				return EventTriggerComparisonMethod::LessThanOrEqual;
			case MetaValueOperator::GreaterThan:
				return EventTriggerComparisonMethod::GreaterThan;
			case MetaValueOperator::GreaterThanOrEqual:
				return EventTriggerComparisonMethod::GreaterThanOrEqual;
		}

		return std::nullopt;
	}

	bool EventTriggerConditionType::compare(const MetaAny& current_value, const MetaAny& comparison_value, ComparisonMethod comparison_method)
	{
		return compare_impl(current_value, comparison_value, comparison_method);
	}

	template <typename ...Args>
	bool EventTriggerConditionType::compare(const MetaAny& current_value, const MetaAny& comparison_value, ComparisonMethod comparison_method, Args&&... args)
	{
		return compare_impl(current_value, comparison_value, comparison_method, args...);
	}

	/*
	// Currently disabled -- default implementations for `MetaEvaluationContext`-enabled overloads:
	bool EventTriggerConditionType::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return condition_met(event_instance, registry, entity);
	}

	bool EventTriggerConditionType::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return condition_met(event_instance, comparison_value, registry, entity);
	}
	*/

	// EventTriggerSingleCondition:
	EventTriggerSingleCondition::EventTriggerSingleCondition
	(
		MetaAny&& comparison_value,
		ComparisonMethod comparison_method,
		MetaTypeID event_type_id
	) : 
		comparison_value(std::move(comparison_value)),
		comparison_method(comparison_method),
		event_type_id(event_type_id)
	{}

	EventTriggerSingleCondition::EventTriggerSingleCondition
	(
		MetaAny&& comparison_value,
		ComparisonMethod comparison_method,
		const MetaType& event_type
	) :
		EventTriggerSingleCondition
		(
			std::move(comparison_value),
			comparison_method,
			((event_type) ? event_type.id() : MetaTypeID {})
		)
	{}

	EventTriggerSingleCondition::EventTriggerSingleCondition
	(
		MetaSymbolID event_type_member,
		MetaAny&& comparison_value,
		ComparisonMethod comparison_method,

		MetaTypeID event_type_id
	) :
		event_type_member(event_type_member),
		comparison_value(std::move(comparison_value)),
		comparison_method(comparison_method),
		event_type_id(event_type_id)
	{}

	EventTriggerSingleCondition::EventTriggerSingleCondition
	(
		MetaSymbolID event_type_member,
		MetaAny&& comparison_value,
		ComparisonMethod comparison_method,
		const MetaType& event_type
	) :
		EventTriggerSingleCondition
		(
			event_type_member,
			std::move(comparison_value),
			comparison_method,
			((event_type) ? event_type.id() : MetaTypeID {})
		)
	{}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return condition_met(event_instance, this->comparison_value, registry, entity);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return condition_met(event_instance, this->comparison_value, registry, entity, context);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		if (!event_instance)
		{
			return false;
		}

		const auto current_value = get_member_value(event_instance);

		return compare(current_value, comparison_value, comparison_method);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, const MetaEvaluationContext& context) const
	{
		if (!event_instance)
		{
			return false;
		}

		const auto current_value = get_member_value(event_instance, context);

		return compare(current_value, comparison_value, comparison_method);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance) const
	{
		if (auto underlying = try_get_underlying_value(this->comparison_value))
		{
			return condition_met(event_instance, underlying);
		}

		if (auto result = condition_met(event_instance, this->comparison_value))
		{
			return result;
		}

		return standalone_boolean_result();
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaEvaluationContext& context) const
	{
		if (auto underlying = try_get_underlying_value(this->comparison_value, context))
		{
			return condition_met(event_instance, underlying, context);
		}

		if (auto result = condition_met(event_instance, this->comparison_value, context))
		{
			return result;
		}

		return standalone_boolean_result();
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		return condition_met_impl(event_instance, comparison_value, registry, entity);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return condition_met_impl(event_instance, comparison_value, registry, entity, context);
	}

	template <typename ...Args>
	bool EventTriggerSingleCondition::condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const
	{
		// Check against the incoming event instance, if present:
		if (event_instance)
		{
			if ((!this->event_type_id) || (this->event_type_id == event_instance.type().id()))
			{
				const auto current_value = get_member_value(event_instance);

				if (condition_met_comparison_impl(event_instance, comparison_value, args...))
				{
					return true;
				}
			}
		}
		else
		{
			if (standalone_boolean_result(args...))
			{
				return true;
			}
		}

		// Fallback to checking against a component of `entity`:
		if (fallback_to_component)
		{
			// Initially check if we can use an encoded type ID:
			auto component_type_id = this->event_type_id;

			if ((!component_type_id) && event_instance)
			{
				// Try to fallback to the `event_instance` object's type ID.
				component_type_id = event_instance.type().id();
			}

			// Check if we could resolve which type to look for as a component:
			if (component_type_id)
			{
				if (auto component_type = resolve(component_type_id))
				{
					// NOTE: This calls `condition_met_comparison_impl` internally.
					if (condition_met_as_component(component_type, comparison_value, args...))
					{
						return true;
					}
				}	
			}
		}

		return false;
	}

	template <typename ...Args>
	bool EventTriggerSingleCondition::condition_met_as_component(const MetaType& component_type, const MetaAny& comparison_value, Args&&... args) const
	{
		if (const auto component = get_component(component_type, args...))
		{
			return condition_met_comparison_impl(component, comparison_value, args...);
		}

		return false;
	}

	template <typename ...Args>
	bool EventTriggerSingleCondition::condition_met_comparison_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const
	{
		if (!event_instance)
		{
			return false;
		}

		if (auto underlying = try_get_underlying_value(comparison_value, args...))
		{
			// NOTE: Recursion.
			return condition_met_comparison_impl(event_instance, underlying, args...); // condition_met
		}

		const auto current_value = get_member_value(event_instance, args..., true);

		return compare(current_value, comparison_value, comparison_method, args...);
	}

	EventTriggerCompoundMethod EventTriggerSingleCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::None;
	}

	MetaAny EventTriggerSingleCondition::get_member_value(const MetaAny& event_instance, bool resolve_underlying) const
	{
		return get_member_value_basic_impl(event_instance, resolve_underlying);
	}

	MetaAny EventTriggerSingleCondition::get_member_value(const MetaAny& event_instance, const MetaEvaluationContext& context, bool resolve_underlying) const
	{
		return get_member_value_basic_impl(event_instance, resolve_underlying, context);
	}

	MetaAny EventTriggerSingleCondition::get_member_value(const MetaAny& event_instance, Registry& registry, Entity entity, bool resolve_underlying) const
	{
		return get_indirect_member_value_impl(event_instance, resolve_underlying, registry, entity);
	}

	MetaAny EventTriggerSingleCondition::get_member_value(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context, bool resolve_underlying) const
	{
		return get_indirect_member_value_impl(event_instance, resolve_underlying, registry, entity, context);
	}

	template <typename ...Args>
	MetaAny EventTriggerSingleCondition::get_member_value_basic_impl(const MetaAny& event_instance, bool resolve_underlying, Args&&... args) const
	{
		auto type = event_instance.type();

		if (!type)
		{
			return {};
		}

		MetaAny value_out;

		if (event_type_member)
		{
			auto data_member = resolve_data_member_by_id(type, true, event_type_member);

			if (!data_member)
			{
				print_warn("Unable to resolve data member (#{}) in type: #{}", event_type_member, type.id());

				return false;
			}
			
			value_out = data_member.get(event_instance);
		}
		else
		{
			value_out = event_instance.as_ref();
		}

		if (value_out && resolve_underlying)
		{
			if (auto underlying = try_get_underlying_value(value_out, std::forward<Args>(args)...))
			{
				return underlying;
			}
		}

		return value_out;
	}

	template <typename ...Args>
	MetaAny EventTriggerSingleCondition::get_indirect_member_value_impl(const MetaAny& event_instance, bool resolve_underlying, Args&&... args) const
	{
		auto value_out = get_member_value(event_instance, false);

		if (resolve_underlying)
		{
			if (auto underlying = try_get_underlying_value(value_out, std::forward<Args>(args)...))
			{
				return underlying;
			}
		}

		return value_out;
	}

	MetaType EventTriggerSingleCondition::get_type() const
	{
		return resolve(event_type_id);
	}

	template <typename ...Args>
	bool EventTriggerSingleCondition::standalone_boolean_result(Args&&... args) const
	{
		if (fallback_to_boolean_evaluation && comparison_value && !event_type_id) // && !event_type_member
		{
			if constexpr (sizeof...(args) > 0)
			{
				if (const auto comparison_value_underlying = get_indirect_value_or_ref(comparison_value, std::forward<Args>(args)...))
				{
					if (auto underlying_as_boolean = comparison_value_underlying.allow_cast<bool>())
					{
						return underlying_as_boolean.cast<bool>();
					}
				}
			}
			else
			{
				if (auto comparison_value_as_boolean = comparison_value.allow_cast<bool>())
				{
					return comparison_value_as_boolean.cast<bool>();
				}
			}
		}

		return false;
	}

	// EventTriggerMemberCondition:
	EventTriggerMemberCondition::EventTriggerMemberCondition
	(
		IndirectMetaDataMember&& component_member,
		MetaAny&& comparison_value,
		ComparisonMethod comparison_method
	) :
		component_member(std::move(component_member)),
		comparison_value(std::move(comparison_value)),
		comparison_method(comparison_method)
	{}

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return condition_met(event_instance, this->comparison_value, registry, entity);
	}

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return condition_met(event_instance, this->comparison_value, registry, entity, context);
	}

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		// Unsupported operation.
		return false;
	}

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, const MetaEvaluationContext& context) const
	{
		// Unsupported operation.
		return false;
	}

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance) const
	{
		//return condition_met(event_instance, this->comparison_value);
		
		// Unsupported operation.
		return false;
	}

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, const MetaEvaluationContext& context) const
	{
		//return condition_met(event_instance, this->comparison_value, context);
		
		// Unsupported operation.
		return false;
	}

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		return condition_met_impl(event_instance, comparison_value, registry, entity);
	}

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return condition_met_impl(event_instance, comparison_value, registry, entity, context);
	}

	template <typename ...Args>
	bool EventTriggerMemberCondition::condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const
	{
		if (!event_instance)
		{
			return false;
		}

		auto target_value = get_indirect_value_or_ref(entt::forward_as_meta(component_member), args...);

		if (!target_value)
		{
			print_warn("Unable to resolve target value for `EventTriggerMemberCondition`.");

			return false;
		}

		auto resolved_comparison_value = get_indirect_value_or_ref(comparison_value, args...);

		if (!resolved_comparison_value)
		{
			print_warn("Unable to resolve comparison value for `EventTriggerMemberCondition`.");

			return false;
		}

		return compare(target_value, resolved_comparison_value, comparison_method, args...);
	}

	EventTriggerCompoundMethod EventTriggerMemberCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::None;
	}

	// EventTriggerCompoundCondition:
	std::size_t EventTriggerCompoundCondition::add_condition(RemoteConditionType&& condition_in)
	{
		conditions.emplace_back(std::move(condition_in));

		return 1;
	}

	bool EventTriggerCompoundCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		// Unable to implement due to missing storage to resolve from.
		return false;
	}

	bool EventTriggerCompoundCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, const MetaEvaluationContext& context) const
	{
		// Unable to implement due to missing storage to resolve from.
		return false;
	}

	bool EventTriggerCompoundCondition::condition_met(const MetaAny& event_instance) const
	{
		// Unable to implement due to missing storage to resolve from.
		return false;
	}

	bool EventTriggerCompoundCondition::condition_met(const MetaAny& event_instance, const MetaEvaluationContext& context) const
	{
		// Unable to implement due to missing storage to resolve from.
		return false;
	}

	std::size_t EventTriggerCompoundCondition::size() const
	{
		return conditions.size();
	}

	bool EventTriggerCompoundCondition::empty() const
	{
		return (size() > 0);
	}

	// EventTriggerAndCondition:
	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return implicit_condition_met_impl(event_instance, registry, entity);
	}

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return implicit_condition_met_impl(event_instance, registry, entity, context);
	}

	template <typename ...Args>
	bool EventTriggerAndCondition::implicit_condition_met_impl(const MetaAny& event_instance, Args&&... args) const
	{
		const auto& descriptor = get_descriptor(args...);

		for (const auto& remote_condition : conditions)
		{
			auto& condition = from_remote_condition(descriptor, remote_condition);

			const auto* condition_interface = EventTriggerConditionType::get_condition_interface(condition);

			assert(condition_interface);

			if (!condition_interface)
			{
				continue;
			}

			if (!condition_interface->condition_met(event_instance, args...))
			{
				return false;
			}
		}

		return true;
	}

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		return explicit_condition_met_impl(event_instance, comparison_value, registry, entity);
	}

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return explicit_condition_met_impl(event_instance, comparison_value, registry, entity, context);
	}
	
	template <typename ...Args>
	bool EventTriggerAndCondition::explicit_condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const
	{
		const auto& descriptor = get_descriptor(args...);

		for (const auto& remote_condition : conditions)
		{
			const auto& condition = from_remote_condition(descriptor, remote_condition);
			const auto* condition_interface = EventTriggerConditionType::get_condition_interface(condition);

			assert(condition_interface);

			if (!condition_interface)
			{
				continue;
			}

			if (!condition_interface->condition_met(event_instance, comparison_value, args...))
			{
				return false;
			}
		}

		return true;
	}

	EventTriggerCompoundMethod EventTriggerAndCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::And;
	}

	// EventTriggerOrCondition:
	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return implicit_condition_met_impl(event_instance, registry, entity);
	}

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return implicit_condition_met_impl(event_instance, registry, entity, context);
	}

	template <typename ...Args>
	bool EventTriggerOrCondition::implicit_condition_met_impl(const MetaAny& event_instance, Args&&... args) const
	{
		const auto& descriptor = get_descriptor(args...);

		for (const auto& remote_condition : conditions)
		{
			const auto& condition = from_remote_condition(descriptor, remote_condition);
			const auto* condition_interface = EventTriggerConditionType::get_condition_interface(condition);

			assert(condition_interface);

			if (!condition_interface)
			{
				continue;
			}

			if (condition_interface->condition_met(event_instance, args...))
			{
				return true;
			}
		}

		return false;
	}

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		return explicit_condition_met_impl(event_instance, comparison_value, registry, entity);
	}

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return explicit_condition_met_impl(event_instance, comparison_value, registry, entity, context);
	}

	template <typename ...Args>
	bool EventTriggerOrCondition::explicit_condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const
	{
		const auto& descriptor = get_descriptor(args...);

		for (const auto& remote_condition : conditions)
		{
			const auto& condition = from_remote_condition(descriptor, remote_condition);
			const auto* condition_interface = EventTriggerConditionType::get_condition_interface(condition);

			assert(condition_interface);

			if (!condition_interface)
			{
				continue;
			}

			if (condition_interface->condition_met(event_instance, comparison_value, args...))
			{
				return true;
			}
		}

		return false;
	}

	EventTriggerCompoundMethod EventTriggerOrCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::Or;
	}

	// EventTriggerTrueCondition:
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, const MetaEvaluationContext& context) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, const MetaEvaluationContext& context) const { return true; }
	
	EventTriggerCompoundMethod EventTriggerTrueCondition::compound_method() const { return EventTriggerCompoundMethod::None; }

	// EventTriggerFalseCondition:
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, const MetaEvaluationContext& context) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, const MetaEvaluationContext& context) const { return false; }
	
	EventTriggerCompoundMethod EventTriggerFalseCondition::compound_method() const { return EventTriggerCompoundMethod::None; }

	// EventTriggerInverseCondition:
	EventTriggerInverseCondition::EventTriggerInverseCondition(RemoteConditionType&& inv_condition)
		: inv_condition(std::move(inv_condition)) {}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return !get_inverse(registry, entity).condition_met(event_instance, registry, entity);
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return !get_inverse(registry, entity).condition_met(event_instance, registry, entity, context);
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		return !get_inverse(registry, entity).condition_met(event_instance, comparison_value, registry, entity);
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return !get_inverse(registry, entity).condition_met(event_instance, comparison_value, registry, entity, context);
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		//return !get_inverse().condition_met(event_instance, comparison_value);

		return false;
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, const MetaEvaluationContext& context) const
	{
		//return !get_inverse().condition_met(event_instance, comparison_value, context);

		return false;
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance) const
	{
		//return !get_inverse().condition_met(event_instance);

		return false;
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, const MetaEvaluationContext& context) const
	{
		//return !get_inverse().condition_met(event_instance, context);

		return false;
	}
	
	EventTriggerCompoundMethod EventTriggerInverseCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::None;
	}

	const EventTriggerConditionType& EventTriggerInverseCondition::get_inverse(const EntityDescriptor& descriptor) const
	{
		const auto& remote_condition = inv_condition.get(descriptor);
		const auto* condition_interface = get_condition_interface(remote_condition);

		assert(condition_interface);

		return *condition_interface;
	}

	const EventTriggerConditionType& EventTriggerInverseCondition::get_inverse(Registry& registry, Entity entity) const
	{
		return get_inverse(get_descriptor(registry, entity));
	}
}