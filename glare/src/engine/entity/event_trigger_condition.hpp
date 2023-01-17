#pragma once

#include "types.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>

#include <util/small_vector.hpp>
#include <util/variant.hpp>

#include <string_view>
#include <utility>
#include <variant>
#include <type_traits>

namespace engine
{
	// TODO: Implement compound conditions.
	// i.e. Check against clauses/multiple data-members of a type.

	enum class EventTriggerComparisonMethod : std::uint8_t
	{
		Equal,
		NotEqual,

		// Currently unsupported:
		LessThan,
		LessThanOrEqualTo,

		GreaterThan,
		GreaterThanOrEqualTo,
	};

	enum class EventTriggerCompoundMethod : std::uint8_t
	{
		None,
		And,
		Or
	};

	class EventTriggerSingleCondition;
	class EventTriggerMemberCondition;
	class EventTriggerCompoundCondition;
	class EventTriggerAndCondition;
	class EventTriggerOrCondition;
	class EventTriggerTrueCondition;
	class EventTriggerFalseCondition;
	class EventTriggerInverseCondition;

	// Abstract base-class for event-trigger conditions.
	class EventTriggerConditionType
	{
		public:
			using ComparisonMethod = EventTriggerComparisonMethod;

			template <typename ConditionType>
			static inline constexpr bool has_type_introspection()
			{
				if constexpr (std::is_base_of_v<EventTriggerSingleCondition, ConditionType>)
				{
					return true;
				}
				else if constexpr (std::is_base_of_v<EventTriggerCompoundCondition, ConditionType>)
				{
					return true;
				}
				else if constexpr (std::is_base_of_v<EventTriggerInverseCondition, ConditionType>)
				{
					// True by proxy.
					return true;
				}
				else
				{
					return false;
				}
			}

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

					[&out](const EventTriggerFalseCondition& false_condition)
					{
						out = &false_condition;
					},

					[&out](const EventTriggerInverseCondition& inverse_condition)
					{
						out = &inverse_condition;
					}
				);

				assert(out);

				return *out;
			}

			// Visits the exact type of `condition` if said type has type-introspection abilities.
			template <typename ConditionVariant, typename Callback>
			static void visit_type_enabled(ConditionVariant&& condition, Callback&& callback)
			{
				std::visit
				(
					[&callback](auto&& exact_condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(exact_condition)>>())
						{
							callback(std::forward<decltype(exact_condition)>(exact_condition));
						}
					},

					std::forward<ConditionVariant>(condition)
				);
			}

			// Resolves the underlying type of `condition`, then wraps the object in a `MetaAny` object.
			template <typename ConditionVariant>
			static MetaAny opaque_condition(ConditionVariant&& condition, bool move_content=false) // EventTriggerCondition
			{
				MetaAny out;

				auto capture = [move_content, &out](auto& underlying_condition)
				{
					if (move_content)
					{
						out = std::move(underlying_condition);
					}
					else
					{
						out = entt::forward_as_meta(underlying_condition);
					}
				};

				util::visit
				(
					condition,

					[&capture](EventTriggerSingleCondition& single_condition)
					{
						capture(single_condition);
					},

					[&capture](EventTriggerMemberCondition& member_condition)
					{
						capture(member_condition);
					},

					[&capture](EventTriggerAndCondition& and_condition)
					{
						capture(and_condition);
					},

					[&capture](EventTriggerOrCondition& or_condition)
					{
						capture(or_condition);
					},

					[&capture](EventTriggerTrueCondition& true_condition)
					{
						capture(true_condition);
					},

					[&capture](EventTriggerFalseCondition& false_condition)
					{
						capture(false_condition);
					},

					[&capture](EventTriggerInverseCondition& inverse_condition)
					{
						capture(inverse_condition);
					}
				);

				assert(out);

				return out;
			}

			// Creates a copy of `condition`, then stores the copy inside an opaque object.
			template <typename ConditionVariant>
			static MetaAny opaque_copy(ConditionVariant&& condition)
			{
				return opaque_condition
				(
					ConditionVariant(std::forward<ConditionVariant>(condition)),

					true
				);
			}

			inline static const EventTriggerConditionType* get_condition_interface(const MetaAny& condition)
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
					case "EventTriggerMemberCondition"_hs:
					case "EventTriggerAndCondition"_hs:
					case "EventTriggerOrCondition"_hs:
					case "EventTriggerTrueCondition"_hs:
					case "EventTriggerFalseCondition"_hs:
					case "EventTriggerInverseCondition"_hs:
						return reinterpret_cast<const EventTriggerConditionType*>(condition.data());
					//case "EventTriggerCondition"_hs:
				}

				return {};
			}

			template <typename Callback>
			inline static bool from_opaque_condition(const MetaAny& condition, Callback&& callback)
			{
				using namespace entt::literals;

				if (!condition)
				{
					return false;
				}

				const auto type = condition.type();

				if (!type)
				{
					return false;
				}

				switch (type.id())
				{
					case "EventTriggerSingleCondition"_hs:
						callback(condition.cast<EventTriggerSingleCondition>());

						break;
					case "EventTriggerMemberCondition"_hs:
						callback(condition.cast<EventTriggerMemberCondition>());

						break;
					case "EventTriggerAndCondition"_hs:
						callback(condition.cast<EventTriggerAndCondition>());

						break;
					case "EventTriggerOrCondition"_hs:
						callback(condition.cast<EventTriggerOrCondition>());

						break;
					case "EventTriggerTrueCondition"_hs:
						callback(condition.cast<EventTriggerTrueCondition>());

						break;
					case "EventTriggerFalseCondition"_hs:
						callback(condition.cast<EventTriggerFalseCondition>());

						break;
					case "EventTriggerInverseCondition"_hs:
						callback(condition.cast<EventTriggerInverseCondition>());

						break;

					// NOTE: The `EventTriggerCondition` variant is currently unsupported
					// for opaque storage due to limitations in EnTT.
					//case "EventTriggerCondition"_hs:
						// ...

					default:
						return false;
				}

				return true;
			}

			template <typename ConditionVariant, typename ...Args>
			static bool get_condition_status(const ConditionVariant& condition, Args&&... args) // EventTriggerCondition
			{
				auto check_condition = [&](auto&& condition_obj)
				{
					return condition_obj.condition_met(std::forward<Args>(args)...);
				};

				bool result = false;

				std::visit
				(
					[&check_condition, &result](const auto& exact_condition)
					{
						if constexpr (std::is_same_v<EventTriggerTrueCondition, std::decay_t<decltype(exact_condition)>>)
						{
							result = true;
						}
						else if constexpr (std::is_same_v<EventTriggerFalseCondition, std::decay_t<decltype(exact_condition)>>)
						{
							result = true;
						}
						else
						{
							result = check_condition(exact_condition);
						}
					},

					condition
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
				MetaTypeID event_type_id={}
			);

			EventTriggerSingleCondition
			(
				MetaSymbolID event_type_member,
				MetaAny&& comparison_value,
				ComparisonMethod comparison_method,
				MetaType event_type
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

			inline MetaTypeID get_type_id() const
			{
				return event_type_id;
			}

			MetaType get_type() const;

			// Interface added for compatibility with compound condition types:
			template <typename Callback>
			inline std::size_t enumerate_conditions(Callback&& callback, bool recursive=true) const
			{
				if constexpr (std::is_invocable_r_v<bool, Callback, const EventTriggerSingleCondition&>)
				{
					if (!callback(*this))
					{
						return 0;
					}
				}
				else
				{
					callback(*this);
				}

				return 1;
			}

			template <typename Callback>
			inline bool enumerate_types(Callback&& callback, bool recursive=true) const
			{
				if constexpr (std::is_invocable_r_v<bool, Callback, MetaTypeID>)
				{
					return callback(get_type_id());
				}
				else
				{
					callback(get_type_id());

					return true;
				}
			}

			template <typename Callback>
			inline std::size_t enumerate_type_compatible(MetaTypeID type_id, Callback&& callback, bool recursive=true, bool allow_true_condition=true) const
			{
				if (get_type_id() == type_id)
				{
					if constexpr (std::is_invocable_r_v<bool, Callback, const EventTriggerSingleCondition&>)
					{
						if (!callback(*this))
						{
							return 0;
						}
					}
					else
					{
						callback(*this);
					}

					return 1;
				}

				return 0;
			}

			inline std::size_t count_type_compatible(MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return enumerate_type_compatible
				(
					type_id,

					// Empty lambda; no additional operations needed.
					[](const auto& condition) {},

					recursive,
					allow_true_condition
				);
			}

			inline bool has_type_compatible(MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return (count_type_compatible(type_id, recursive, allow_true_condition) > 0);
			}
		protected:
			MetaSymbolID event_type_member;
			MetaAny comparison_value;

			// NOTE: Optional; uses type of event instance if not specified.
			MetaTypeID event_type_id = {};

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
			std::size_t add_condition(EventTriggerInverseCondition&& condition_in);

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
					},

					[this, &result](EventTriggerInverseCondition&& inverse_condition)
					{
						result = this->add_condition(std::move(inverse_condition));
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

			template <typename Callback>
			inline std::size_t enumerate_conditions(Callback&& callback, bool recursive=true) const
			{
				std::size_t condition_count = 0;

				for (const auto& opaque_condition : conditions)
				{
					bool should_break = false;

					from_opaque_condition
					(
						opaque_condition,

						[&callback, &condition_count, recursive, &should_break](const auto& condition)
						{
							if constexpr (std::is_base_of_v<EventTriggerCompoundCondition, std::decay_t<decltype(condition)>>)
							{
								if (!recursive)
								{
									return;
								}

								condition_count += condition.enumerate_conditions(callback, recursive);
							}
							else if constexpr (std::is_invocable_r_v<bool, Callback, decltype(condition)>)
							{
								if (!callback(condition))
								{
									should_break = true;

									return;
								}

								condition_count++;
							}
							else
							{
								callback(condition);

								condition_count++;
							}
						}
					);

					if (should_break)
					{
						break;
					}
				}

				return condition_count;
			}

			template <typename Callback>
			inline bool enumerate_types(Callback&& callback, bool recursive=true) const
			{
				enumerate_conditions
				(
					[&callback, recursive](const auto& condition)
					{
						auto on_type_enabled = [&callback, recursive](const auto& condition)
						{
							return condition.enumerate_types(callback, recursive);
						};

						if constexpr (std::is_base_of_v<EventTriggerCompoundCondition, std::decay_t<decltype(condition)>>)
						{
							if (recursive)
							{
								return on_type_enabled(condition);
							}
							else
							{
								return true;
							}
						}
						else if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							return on_type_enabled(condition);
						}
						else
						{
							return true;
						}
					},

					recursive
				);

				return true;
			}

			template <typename Callback>
			inline std::size_t enumerate_type_compatible(MetaTypeID type_id, Callback&& callback, bool recursive=true, bool allow_true_condition=true) const
			{
				std::size_t type_compatible = 0;

				enumerate_conditions
				(
					[type_id, recursive, allow_true_condition, &callback, &type_compatible](const auto& condition) -> bool
					{
						auto on_match = [&callback, &type_compatible](const auto& condition) -> bool
						{
							if constexpr (std::is_invocable_r_v<bool, Callback, decltype(condition)>)
							{
								if (!callback(condition))
								{
									return false;
								}
							}
							else
							{
								callback(condition);
							}

							type_compatible++;

							return true;
						};

						if constexpr (std::is_base_of_v<EventTriggerSingleCondition, std::decay_t<decltype(condition)>>)
						{
							if (condition.get_type_id() == type_id)
							{
								return on_match(condition);
							}
						}
						else if constexpr (std::is_base_of_v<EventTriggerTrueCondition, std::decay_t<decltype(condition)>>)
						{
							if (allow_true_condition)
							{
								return on_match(condition);
							}
						}
						else if constexpr (std::is_base_of_v<EventTriggerInverseCondition, std::decay_t<decltype(condition)>>)
						{
							if (condition.enumerate_type_compatible(type_id, callback, recursive, allow_true_condition) > 0)
							{
								return on_match(condition);
							}
						}
						
						return true;
					},

					recursive
				);

				return type_compatible;
			}

			inline std::size_t count_type_compatible(MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return enumerate_type_compatible
				(
					type_id,

					// Empty lambda; no additional operations needed.
					[](const auto& condition) {},

					recursive, allow_true_condition
				);
			}

			inline bool has_type_compatible(MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return (count_type_compatible(type_id, recursive, allow_true_condition) > 0);
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

	// Inverts the result of `condition_met` for a given condition object.
	class EventTriggerInverseCondition : public EventTriggerConditionType
	{
		public:
			EventTriggerInverseCondition(MetaAny&& inv_condition);

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;

			const EventTriggerConditionType& get_inverse() const;

			// Interface added for compatibility with compound condition types:
			template <typename Callback>
			inline std::size_t enumerate_conditions(Callback&& callback, bool recursive=true) const
			{
				if constexpr (std::is_invocable_r_v<bool, Callback, const EventTriggerInverseCondition&>)
				{
					if (!callback(*this))
					{
						return 0;
					}
				}
				else
				{
					callback(*this);
				}

				return 1;
			}

			template <typename Callback>
			inline bool enumerate_types(Callback&& callback, bool recursive=true) const
			{
				bool result = false;

				from_opaque_condition
				(
					inv_condition,

					[&callback, recursive, &result](const auto& condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							result = condition.enumerate_types(callback, recursive);
						}
					}
				);

				return result;
			}

			template <typename Callback>
			inline std::size_t enumerate_type_compatible(MetaTypeID type_id, Callback&& callback, bool recursive=true, bool allow_true_condition=true) const
			{
				std::size_t result = 0;

				from_opaque_condition
				(
					inv_condition,

					[type_id, &callback, recursive, allow_true_condition, &result](const auto& condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							result = condition.enumerate_type_compatible(type_id, callback, recursive, allow_true_condition);
						}
					}
				);

				return result;
			}

			inline std::size_t count_type_compatible(MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				std::size_t result = 0;

				from_opaque_condition
				(
					inv_condition,

					[type_id, recursive, allow_true_condition, &result](const auto& condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							result = condition.count_type_compatible(type_id, recursive, allow_true_condition);
						}
					}
				);

				return result;
			}

			inline bool has_type_compatible(MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				bool result = false;

				from_opaque_condition
				(
					inv_condition,

					[type_id, recursive, allow_true_condition, &result](const auto& condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							result = condition.has_type_compatible(type_id, recursive, allow_true_condition);
						}
					}
				);

				return result;
			}

		protected:
			MetaAny inv_condition;
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
		EventTriggerFalseCondition,
		EventTriggerInverseCondition
	>;
}