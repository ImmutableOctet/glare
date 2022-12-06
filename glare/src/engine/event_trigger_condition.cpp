#include "event_trigger_condition.hpp"

#include "meta/meta.hpp"
#include "meta/meta_data_member.hpp"

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	// EventTriggerConditionType:
	EventTriggerConditionType::ComparisonMethod EventTriggerConditionType::get_comparison_method(std::string_view comparison_operator)
	{
		using namespace entt::literals;

		switch (hash(comparison_operator))
		{
			case "!="_hs:
			case "!=="_hs:
				return ComparisonMethod::NotEqual;

			/*
			case "<"_hs:
				return ComparisonMethod::LessThan
			case "<="_hs:
				return ComparisonMethod::LessThanOrEqualTo;
			case ">"_hs:
				return ComparisonMethod::GreaterThan
			case ">="_hs:
				return ComparisonMethod::GreaterThanOrEqualTo;
			*/
		}

		return ComparisonMethod::Equal;
	}

	const EventTriggerConditionType* EventTriggerConditionType::get_condition_interface(const MetaAny& condition)
	{
		using namespace entt::literals;

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
			case "EventTriggerAndCondition"_hs:
			case "EventTriggerOrCondition"_hs:
				return reinterpret_cast<const EventTriggerConditionType*>(condition.data());
			//case "EventTriggerCondition"_hs:
		}

		return {};
	}

	EventTriggerConditionType::~EventTriggerConditionType() {}

	// EventTriggerSingleCondition:
	EventTriggerSingleCondition::EventTriggerSingleCondition(MetaSymbolID event_type_member, MetaAny&& comparison_value, ComparisonMethod comparison_method) :
		event_type_member(event_type_member),
		comparison_value(std::move(comparison_value)),
		comparison_method(comparison_method)
	{}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
	{
		using namespace entt::literals;

		if (!event_instance)
		{
			return false;
		}

		const auto comparison_value_type    = comparison_value.type();
		const auto comparison_value_type_id = comparison_value_type.id();

		switch (comparison_value_type_id)
		{
			case "MetaDataMember"_hs:
			{
				const auto data_member = this->comparison_value.cast<MetaDataMember>();

				if (auto embedded_value = data_member.get(registry, entity))
				{
					return condition_met(event_instance, embedded_value);
				}

				break;
			}
		}

		return condition_met(event_instance, comparison_value);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		if (!event_instance)
		{
			return false;
		}

		const auto current_value = get_member_value(event_instance);

		switch (comparison_method)
		{
			case ComparisonMethod::Equal:
				return (current_value == comparison_value);

			case ComparisonMethod::NotEqual:
				return (current_value != comparison_value);

			default:
				print_warn("Unsupported comparison method detected.");

				break; // return false;
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

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance) const
	{
		return condition_met(event_instance, this->comparison_value);
	}

	// EventTriggerCompoundCondition:
	std::size_t EventTriggerCompoundCondition::add_condition(EventTriggerSingleCondition&& condition_in)
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

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance) const
	{
		return EventTriggerAndCondition_condition_met_impl(conditions, event_instance);
	}

	bool EventTriggerAndCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		return EventTriggerAndCondition_condition_met_impl(conditions, event_instance, comparison_value);
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

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance) const
	{
		return EventTriggerOrCondition_condition_met_impl(conditions, event_instance);
	}

	bool EventTriggerOrCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
	{
		return EventTriggerOrCondition_condition_met_impl(conditions, event_instance, comparison_value);
	}

	EventTriggerCompoundMethod EventTriggerOrCondition::compound_method() const
	{
		return EventTriggerCompoundMethod::Or;
	}
}