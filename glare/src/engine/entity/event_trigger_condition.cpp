#include "event_trigger_condition.hpp"

#include "entity_target.hpp"

#include <engine/meta/meta.hpp>
#include <engine/meta/meta_data_member.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>

#include <optional>
#include <utility>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	// EventTriggerConditionType:
	EventTriggerConditionType::ComparisonMethod EventTriggerConditionType::get_comparison_method(std::string_view comparison_operator)
	{
		using namespace entt::literals;

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
				return ComparisonMethod::LessThanOrEqualTo;

			case ">"_hs:
				return ComparisonMethod::GreaterThan;

			case ">="_hs:
				return ComparisonMethod::GreaterThanOrEqualTo;

			/*
			case "="_hs:
			case "|"_hs:
				return ComparisonMethod::Equal;
			*/
		}

		return ComparisonMethod::Equal;
	}

	EventTriggerConditionType::~EventTriggerConditionType() {}

	MetaAny EventTriggerConditionType::resolve_value(Registry& registry, Entity entity, const MetaAny& data, bool fallback_to_input_value, bool recursive)
	{
		using namespace entt::literals;

		MetaAny out;

		switch (data.type().id())
		{
			case "MetaDataMember"_hs:
				out = data.cast<MetaDataMember>().get(registry, entity);

				break;

			case "IndirectMetaDataMember"_hs:
				out = data.cast<IndirectMetaDataMember>().get(registry, entity);

				break;

			case "EntityTarget"_hs:
				out = data.cast<EntityTarget>().get(registry, entity);

				break;
		}

		if (out && recursive)
		{
			return resolve_value(registry, entity, out, true, true);
		}

		if (fallback_to_input_value)
		{
			return data;
		}

		return out;
	}

	MetaAny EventTriggerConditionType::get_component(const MetaType& component_type, Registry& registry, Entity entity)
	{
		using namespace entt::literals;

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

	bool EventTriggerConditionType::compare(const MetaAny& current_value, const MetaAny& comparison_value, ComparisonMethod comparison_method)
	{
		switch (comparison_method)
		{
			case ComparisonMethod::Equal:
			{
				if (current_value == comparison_value)
				{
					return true;
				}

				return meta_any_string_compare(current_value, comparison_value);
			}

			case ComparisonMethod::NotEqual:
				if (current_value != comparison_value)
				{
					return !meta_any_string_compare(current_value, comparison_value);
				}

				return false;


			/*
			case ComparisonMethod::LessThan:
				return (current_value < comparison_value);
			case ComparisonMethod::LessThanOrEqualTo:
				return (current_value <= comparison_value);
			case ComparisonMethod::GreaterThan:
				return (current_value > comparison_value);
			case ComparisonMethod::GreaterThanOrEqualTo:
				return (current_value >= comparison_value);
			*/

			default:
				print_error("Unsupported comparison method detected: {}", static_cast<std::uint32_t>(comparison_method));

				break; // return false;
		}

		return false;
	}

	bool EventTriggerConditionType::condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		using namespace entt::literals;

		const auto comparison_value_type = comparison_value.type();
		const auto comparison_value_type_id = comparison_value_type.id();

		auto handle_data_member = [this, &event_instance, &registry, entity](auto&& data_member, auto&& false_value) -> std::optional<bool>
		{
			if (auto embedded_value = data_member.get(registry, entity); (embedded_value != false_value))
			{
				//return condition_met_impl(event_instance, embedded_value);

				// NOTE: Recursion.
				return condition_met_impl(event_instance, embedded_value, registry, entity);
			}

			return std::nullopt;
		};

		// TODO: Look into refactoring this to use `resolve_value` instead. (May be slightly slower?)
		switch (comparison_value_type_id)
		{
			case "MetaDataMember"_hs:
				if (auto result = handle_data_member(comparison_value.cast<MetaDataMember>(), false))
				{
					return *result;
				}

				break;

			case "IndirectMetaDataMember"_hs:
				if (auto result = handle_data_member(comparison_value.cast<IndirectMetaDataMember>(), false))
				{
					return *result;
				}

				break;
			case "EntityTarget"_hs:
				if (auto result = handle_data_member(comparison_value.cast<EntityTarget>(), null))
				{
					return *result;
				}

				break;
		}

		return condition_met(event_instance, comparison_value);
	}

	// EventTriggerSingleCondition:
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
		MetaType event_type
	) : EventTriggerSingleCondition
	(
		event_type_member,
		std::move(comparison_value),
		comparison_method,
		event_type.id()
	)
	{}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return condition_met(event_instance, this->comparison_value, registry, entity);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		if (!event_instance)
		{
			return false;
		}

		/*
		if (this->event_type)
		{
			if (this->event_type_id != event_instance.type().id())
			{
				return false;
			}
		}
		*/

		const auto current_value = get_member_value(event_instance);

		return compare(current_value, comparison_value, comparison_method);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance) const
	{
		return condition_met(event_instance, this->comparison_value);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		// Check against the incoming event instance, if present:
		if (event_instance)
		{
			if ((!this->event_type_id) || (this->event_type_id == event_instance.type().id()))
			{
				if (condition_met_impl(event_instance, comparison_value, registry, entity))
				{
					return true;
				}
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
					// NOTE: This calls `condition_met_impl` internally.
					if (condition_met_as_component(component_type, comparison_value, registry, entity))
					{
						return true;
					}
				}	
			}
		}

		return false;
	}

	bool EventTriggerSingleCondition::condition_met_as_component(const MetaType& component_type, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		if (const auto component = get_component(component_type, registry, entity))
		{
			return condition_met_impl(component, comparison_value, registry, entity);
		}

		return false;
	}

	EventTriggerCompoundMethod EventTriggerSingleCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::None;
	}

	MetaAny EventTriggerSingleCondition::get_member_value(const MetaAny& event_instance) const
	{
		auto type = event_instance.type();

		if (!type)
		{
			return {};
		}

		auto data_member = type.data(event_type_member);

		if (!data_member)
		{
			print_warn("Unable to resolve data member (#{}) in type: #{}", event_type_member, type.id());

			return false;
		}

		return data_member.get(event_instance);
	}

	MetaType EventTriggerSingleCondition::get_type() const
	{
		return resolve(event_type_id);
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

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
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

	bool EventTriggerMemberCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		if (!event_instance)
		{
			return false;
		}

		const auto target_value = resolve_value(registry, entity, component_member, false);

		if (!target_value)
		{
			print_warn("Unable to resolve target value for `EventTriggerMemberCondition`.");

			return false;
		}

		const auto resolved_comparison_value = resolve_value(registry, entity, comparison_value, true);

		if (!resolved_comparison_value)
		{
			print_warn("Unable to resolve comparison value for `EventTriggerMemberCondition`.");

			return false;
		}

		return compare(target_value, resolved_comparison_value, comparison_method);
	}

	EventTriggerCompoundMethod EventTriggerMemberCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::None;
	}

	// EventTriggerCompoundCondition:
	std::size_t EventTriggerCompoundCondition::add_condition(EventTriggerSingleCondition&& condition_in)
	{
		conditions.emplace_back(std::move(condition_in));

		return 1;
	}

	std::size_t EventTriggerCompoundCondition::add_condition(EventTriggerMemberCondition&& condition_in)
	{
		conditions.emplace_back(std::move(condition_in));

		return 1;
	}

	std::size_t EventTriggerCompoundCondition::add_condition(EventTriggerAndCondition&& condition_in)
	{
		conditions.emplace_back(std::move(condition_in));

		return 1;
	}

	std::size_t EventTriggerCompoundCondition::add_condition(EventTriggerOrCondition&& condition_in)
	{
		conditions.emplace_back(std::move(condition_in));

		return 1;
	}

	std::size_t EventTriggerCompoundCondition::add_condition(EventTriggerTrueCondition&& condition_in)
	{
		conditions.emplace_back(std::move(condition_in));

		return 1;
	}

	std::size_t EventTriggerCompoundCondition::add_condition(EventTriggerFalseCondition&& condition_in)
	{
		conditions.emplace_back(std::move(condition_in));

		return 1;
	}

	std::size_t EventTriggerCompoundCondition::add_condition(EventTriggerInverseCondition&& condition_in)
	{
		conditions.emplace_back(std::move(condition_in));

		return 1;
	}

	// EventTriggerAndCondition:
	template <typename ...Args>
	static bool EventTriggerAndCondition_condition_met_impl(const EventTriggerCompoundCondition::ConditionContainer& conditions, Args&&... args)
	{
		for (const auto& condition_any : conditions)
		{
			const auto* condition_interface = EventTriggerConditionType::get_condition_interface(condition_any);

			if (!condition_interface)
			{
				//return false;

				continue;
			}

			if (!condition_interface->condition_met(std::forward<Args>(args)...))
			{
				return false;
			}
		}

		return true;
	}

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return EventTriggerAndCondition_condition_met_impl(conditions, event_instance, registry, entity);
	}

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		return EventTriggerAndCondition_condition_met_impl(conditions, event_instance, comparison_value, registry, entity);
	}

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		return EventTriggerAndCondition_condition_met_impl(conditions, event_instance, comparison_value);
	}

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance) const
	{
		return EventTriggerAndCondition_condition_met_impl(conditions, event_instance);
	}

	EventTriggerCompoundMethod EventTriggerAndCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::And;
	}

	// EventTriggerOrCondition:
	template <typename ...Args>
	static bool EventTriggerOrCondition_condition_met_impl(const EventTriggerCompoundCondition::ConditionContainer& conditions, Args&&... args)
	{
		for (const auto& condition_any : conditions)
		{
			const auto* condition_interface = EventTriggerConditionType::get_condition_interface(condition_any);

			if (!condition_interface)
			{
				//return false;

				continue;
			}

			if (condition_interface->condition_met(std::forward<Args>(args)...))
			{
				return true;
			}
		}

		return false;
	}

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return EventTriggerOrCondition_condition_met_impl(conditions, event_instance, registry, entity);
	}

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		return EventTriggerOrCondition_condition_met_impl(conditions, event_instance, comparison_value, registry, entity);
	}

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		return EventTriggerOrCondition_condition_met_impl(conditions, event_instance, comparison_value);
	}

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance) const
	{
		return EventTriggerOrCondition_condition_met_impl(conditions, event_instance);
	}

	EventTriggerCompoundMethod EventTriggerOrCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::Or;
	}

	// EventTriggerTrueCondition:
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const { return true; }
	bool EventTriggerTrueCondition::condition_met(const MetaAny& event_instance) const { return true; }
	EventTriggerCompoundMethod EventTriggerTrueCondition::compound_method() const { return EventTriggerCompoundMethod::None; }

	// EventTriggerFalseCondition:
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const { return false; }
	bool EventTriggerFalseCondition::condition_met(const MetaAny& event_instance) const { return false; }
	EventTriggerCompoundMethod EventTriggerFalseCondition::compound_method() const { return EventTriggerCompoundMethod::None; }

	// EventTriggerInverseCondition:
	EventTriggerInverseCondition::EventTriggerInverseCondition(MetaAny&& inv_condition)
		: inv_condition(std::move(inv_condition)) {}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		return !get_inverse().condition_met(event_instance, registry, entity);
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		return !get_inverse().condition_met(event_instance, comparison_value, registry, entity);
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		return !get_inverse().condition_met(event_instance, comparison_value);
	}

	bool EventTriggerInverseCondition::condition_met(const MetaAny& event_instance) const
	{
		return !get_inverse().condition_met(event_instance);
	}
	
	EventTriggerCompoundMethod EventTriggerInverseCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::None;
	}

	const EventTriggerConditionType& EventTriggerInverseCondition::get_inverse() const
	{
		const auto result = get_condition_interface(inv_condition);

		assert(result);

		return *result;
	}
}