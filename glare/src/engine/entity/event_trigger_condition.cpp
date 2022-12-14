#include "event_trigger_condition.hpp"

#include "entity_target.hpp"

#include <engine/meta/meta.hpp>
#include <engine/meta/meta_data_member.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>

#include <optional>

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
		return condition_met(event_instance, this->comparison_value, registry, entity);
	}

	bool EventTriggerSingleCondition::condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const
	{
		using namespace entt::literals;

		if (!event_instance)
		{
			return false;
		}

		/*
		// TODO: Determine if we want this check.
		if (!comparison_value)
		{
			return false;
		}
		*/

		const auto comparison_value_type = comparison_value.type();
		const auto comparison_value_type_id = comparison_value_type.id();

		auto handle_data_member = [this, &event_instance, &registry, entity](auto&& data_member, auto&& false_value) -> std::optional<bool>
		{
			if (auto embedded_value = data_member.get(registry, entity); (embedded_value != false_value))
			{
				//return condition_met(event_instance, embedded_value);

				// NOTE: Recursion.
				return condition_met(event_instance, embedded_value, registry, entity);
			}

			return std::nullopt;
		};

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
}