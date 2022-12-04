#include "event_trigger_condition.hpp"

#include "meta/meta.hpp"
#include "meta/meta_data_member.hpp"

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

	bool EventTriggerCondition::condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const
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

	bool EventTriggerCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const
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

	MetaAny EventTriggerCondition::get_member_value(const MetaAny& event_instance) const
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

	bool EventTriggerCondition::condition_met(const MetaAny& event_instance) const
	{
		return condition_met(event_instance, this->comparison_value);
	}
}