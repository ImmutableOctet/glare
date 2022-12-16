#pragma once

#include "types.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>

#include <util/small_vector.hpp>
#include <util/variant.hpp>

#include <string_view>
#include <utility>
#include <variant>

namespace engine
{
	// TODO: Implement compound conditions.
	// i.e. Check against clauses/multiple data-members of a type.

	enum class EventTriggerComparisonMethod : std::uint8_t
	{
		Equal,
		NotEqual,

		// Currently unsupported:
		/*
		LessThan,
		LessThanOrEqualTo,

		GreaterThan,
		GreaterThanOrEqualTo,
		*/
	};

	enum class EventTriggerCompoundMethod : std::uint8_t
	{
		None,
		And,
		Or
	};

	class EventTriggerSingleCondition;
	class EventTriggerMemberCondition;
	class EventTriggerAndCondition;
	class EventTriggerOrCondition;
	class EventTriggerTrueCondition;
	class EventTriggerFalseCondition;

	// Abstract base-class for event-trigger conditions.
	class EventTriggerConditionType
	{
		public:
			using ComparisonMethod = EventTriggerComparisonMethod;

			static ComparisonMethod get_comparison_method(std::string_view comparison_operator);
			static MetaAny resolve_value(Registry& registry, Entity entity, const MetaAny& data, bool fallback_to_input_value=true, bool recursive=true);
			static MetaAny get_component(const MetaType& component_type, Registry& registry, Entity entity);

			static bool compare(const MetaAny& current_value, const MetaAny& comparison_value, ComparisonMethod comparison_method);

			// Retrieves a reference to the shared `EventTriggerConditionType` interface from an `EventTriggerCondition` variant.
			// TODO: Look into adding `std::monostate` behavior. (Would require changing to pointer return-type)
			template <typename ConditionVariant>
			static const EventTriggerConditionType& get_condition_interface(const ConditionVariant& condition) // EventTriggerCondition
			{
				const EventTriggerConditionType* out = nullptr;

				util::visit
				(
					condition,

					[&out](const EventTriggerSingleCondition& single_condition)
					{
						out = &single_condition;
					},

					[&out](const EventTriggerMemberCondition& member_condition)
					{
						out = &member_condition;
					},

					[&out](const EventTriggerAndCondition& and_condition)
					{
						out = &and_condition;
					},

					[&out](const EventTriggerOrCondition& or_condition)
					{
						out = &or_condition;
					},

					[&out](const EventTriggerTrueCondition& true_condition)
					{
						out = &true_condition;
					},

					[&out](const EventTriggerTrueCondition& false_condition)
					{
						out = &false_condition;
					}
				);

				assert(out);

				return *out;
			}

			static const EventTriggerConditionType* get_condition_interface(const MetaAny& condition);

			template <typename ConditionVariant, typename ...Args>
			static bool get_condition_status(const ConditionVariant& condition, Args&&... args) // EventTriggerCondition
			{
				bool result = false;

				auto check_condition = [&](auto&& condition_obj)
				{
					return condition_obj.condition_met(std::forward<Args>(args)...);
				};

				util::visit
				(
					condition,

					[&check_condition, &result](const EventTriggerSingleCondition& single_condition)
					{
						result = check_condition(single_condition);
					},

					[&check_condition, &result](const EventTriggerMemberCondition& member_condition)
					{
						result = check_condition(member_condition);
					},

					[&check_condition, &result](const EventTriggerAndCondition& and_condition)
					{
						result = check_condition(and_condition);
					},

					[&check_condition, &result](const EventTriggerOrCondition& or_condition)
					{
						result = check_condition(or_condition);
					},

					[&result](const EventTriggerTrueCondition& true_condition)
					{
						//result = check_condition(true_condition);
						result = true;
					},

					[&result](const EventTriggerFalseCondition& false_condition)
					{
						//result = check_condition(false_condition);
						result = false;
					}
				);

				return result;
			}

			virtual ~EventTriggerConditionType();

			virtual bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const = 0;
			virtual bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const = 0;
			virtual bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const = 0;
			virtual bool condition_met(const MetaAny& event_instance) const = 0;

			virtual EventTriggerCompoundMethod compound_method() const = 0;

			template <typename ...Args>
			inline bool operator()(Args&&... args) const
			{
				return condition_met(std::forward<Args>(args)...);
			}
		protected:
			// Default implementation of most explicit form of `condition_met`.
			// (Primarily responsible for resolving indirection from `comparison_value`)
			// 
			// Calls virtual `condition_met(event_instance, comparison_value)` method internally.
			bool condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const;
	};

	class EventTriggerSingleCondition : public EventTriggerConditionType
	{
		public:
			EventTriggerSingleCondition
			(
				MetaSymbolID event_type_member,
				MetaAny&& comparison_value,
				ComparisonMethod comparison_method=ComparisonMethod::Equal,

				// Explicit event-type specification.
				// (Leave default to support any type with `event_type_member`)
				MetaType event_type={}
			);

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			bool condition_met_as_component(const MetaType& component_type, const MetaAny& comparison_value, Registry& registry, Entity entity) const;

			EventTriggerCompoundMethod compound_method() const override;

			MetaAny get_member_value(const MetaAny& event_instance) const;

			inline MetaSymbolID get_event_type_member() const
			{
				return event_type_member;
			}

			inline const MetaAny& get_comparison_value() const
			{
				return comparison_value;
			}

			inline ComparisonMethod get_comparison_method() const
			{
				return comparison_method;
			}
		protected:
			MetaSymbolID event_type_member;
			MetaAny comparison_value;

			// NOTE: Optional; uses type of event instance if not specified.
			MetaType event_type;

			ComparisonMethod comparison_method;
			bool fallback_to_component : 1 = true;
	};

	class EventTriggerMemberCondition : public EventTriggerConditionType
	{
		public:
			EventTriggerMemberCondition
			(
				IndirectMetaDataMember&& component_member,
				MetaAny&& comparison_value,
				ComparisonMethod comparison_method=ComparisonMethod::Equal
			);

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;

			EventTriggerCompoundMethod compound_method() const override;

			inline const MetaAny& get_comparison_value() const
			{
				return comparison_value;
			}

			inline ComparisonMethod get_comparison_method() const
			{
				return comparison_method;
			}

			inline MetaSymbolID get_event_type_member() const
			{
				return component_member.data_member.data_member_id;
			}

		protected:
			// This field effectively acts as a path and 'accessor'
			// to the component data member we're referencing.
			IndirectMetaDataMember component_member;

			MetaAny comparison_value;
			ComparisonMethod comparison_method;
	};

	// Abstract base-class for compound condition types.
	class EventTriggerCompoundCondition : public EventTriggerConditionType
	{
		public:
			using ConditionContainer = util::small_vector<MetaAny, 2>;

			template <typename ConditionVariant>
			static const EventTriggerCompoundCondition* get_compound_condition_interface(const ConditionVariant& condition) // EventTriggerCondition
			{
				const EventTriggerCompoundCondition* out = nullptr;

				util::visit
				(
					condition,

					[](const EventTriggerSingleCondition& single_condition) {},
					[](const EventTriggerMemberCondition& member_condition) {},

					[&out](const EventTriggerAndCondition& and_condition)
					{
						out = &and_condition;
					},

					[&out](const EventTriggerOrCondition& or_condition)
					{
						out = &or_condition;
					},

					[](const EventTriggerTrueCondition& true_condition) {},
					[](const EventTriggerFalseCondition& false_condition) {}
				);

				return out;
			}

			std::size_t add_condition(EventTriggerSingleCondition&& condition_in);
			std::size_t add_condition(EventTriggerMemberCondition&& condition_in);
			std::size_t add_condition(EventTriggerAndCondition&& condition_in);
			std::size_t add_condition(EventTriggerOrCondition&& condition_in);
			std::size_t add_condition(EventTriggerTrueCondition&& condition_in);
			std::size_t add_condition(EventTriggerFalseCondition&& condition_in);

			template <typename ConditionVariant>
			std::size_t add_condition(ConditionVariant&& condition_in)
			{
				std::size_t result = 0;

				util::visit
				(
					std::move(condition_in),

					[this, &result](EventTriggerSingleCondition&& single_condition)
					{
						result = this->add_condition(std::move(single_condition));
					},

					[this, &result](EventTriggerMemberCondition&& member_condition)
					{
						result = this->add_condition(std::move(member_condition));
					},

					[this, &result](EventTriggerAndCondition&& and_condition)
					{
						result = this->add_condition(std::move(and_condition));
					},

					[this, &result](EventTriggerOrCondition&& or_condition)
					{
						result = this->add_condition(std::move(or_condition));
					},

					[this, &result](EventTriggerTrueCondition&& true_condition)
					{
						result = this->add_condition(std::move(true_condition));
					},

					[this, &result](EventTriggerFalseCondition&& false_condition)
					{
						result = this->add_condition(std::move(false_condition));
					}
				);

				return result;
			}

			inline const ConditionContainer& get_conditions() const
			{
				return conditions;
			}

			inline ConditionContainer& get_conditions()
			{
				return conditions;
			}
		protected:
			ConditionContainer conditions;
	};

	class EventTriggerAndCondition : public EventTriggerCompoundCondition
	{
		public:
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;
	};

	class EventTriggerOrCondition : public EventTriggerCompoundCondition
	{
		public:
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;
	};

	class EventTriggerTrueCondition : public EventTriggerConditionType
	{
		public:
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;
	};

	class EventTriggerFalseCondition : public EventTriggerConditionType
	{
		public:
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;
	};

	// Variant type used to represent an event-trigger condition held by an external type. (e.g. `EntityStateRule`)
	// TODO: Look into adding `std::monostate` option here. (Currently using `std::optional` to wrap 'no condition' scenario)
	using EventTriggerCondition = std::variant
	<
		EventTriggerSingleCondition,
		EventTriggerMemberCondition,
		EventTriggerAndCondition,
		EventTriggerOrCondition,
		EventTriggerTrueCondition,
		EventTriggerFalseCondition
	>;
}