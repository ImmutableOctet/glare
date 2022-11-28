#include "event_trigger_condition.hpp"
#include "meta/meta.hpp"

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	EventTriggerCondition::ComparisonMethod EventTriggerCondition::get_comparison_method(std::string_view comparison_operator)
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

	bool EventTriggerCondition::condition_met(const MetaAny& instance) const
	{
		if (!instance)
		{
			return false;
		}

		auto type = instance.type();

		auto data_member = type.data(event_type_member);

		if (!data_member)
		{
			print_warn("Unable to resolve data member (#{}) in type: #{}", event_type_member, type.id());

			return false;
		}

		auto current_value = data_member.get(instance);

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
}