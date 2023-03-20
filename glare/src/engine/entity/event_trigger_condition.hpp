#pragma once

#include "types.hpp"
#include "entity_descriptor_shared.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/indirect_meta_data_member.hpp>
#include <engine/meta/meta_value_operator.hpp>

#include <util/small_vector.hpp>
#include <util/variant.hpp>

#include <string_view>
#include <utility>
#include <variant>
#include <type_traits>
#include <optional>

namespace engine
{
	class EntityDescriptor;

	struct MetaEvaluationContext;

	// TODO: Implement compound conditions.
	// i.e. Check against clauses/multiple data-members of a type.

	enum class EventTriggerComparisonMethod : std::uint8_t
	{
		Equal,
		NotEqual,

		LessThan,
		LessThanOrEqual,

		GreaterThan,
		GreaterThanOrEqual,
	};

	enum class EventTriggerCompoundMethod : std::uint8_t
	{
		None,
		And,
		Or
	};

	class EventTriggerCondition;
	class EventTriggerConditionType;
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
			using RemoteConditionType = EntityDescriptorShared<EventTriggerCondition>;

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

			static const EntityDescriptor& get_descriptor(Registry& registry, Entity entity);
			static const EntityDescriptor* try_get_descriptor(Registry& registry, Entity entity);

			static const EventTriggerCondition& from_remote_condition(const EntityDescriptor& descriptor, const RemoteConditionType& condition);

			static const EventTriggerConditionType* get_condition_interface(const MetaAny& condition);
			static const EventTriggerConditionType* get_condition_interface(const MetaAny& condition, Registry& registry, Entity entity);

			static ComparisonMethod get_comparison_method(std::string_view comparison_operator);
			static ComparisonMethod get_comparison_method(std::string_view comparison_operator, bool apply_inversion);

			static ComparisonMethod invert_comparison_method(ComparisonMethod comparison_method);

			static MetaAny resolve_value(Registry& registry, Entity entity, const MetaAny& data, bool fallback_to_input_value=true, bool recursive=true);
			
			static MetaValueOperator operation_from_comparison_method(EventTriggerComparisonMethod comparison_method);
			static std::optional<EventTriggerComparisonMethod> comparison_method_from_operation(MetaValueOperator operation);

			static MetaAny get_component(const MetaType& component_type, Registry& registry, Entity entity);

			// NOTE: Indirection of the `current_value` and `comparison_value` is not resolved when using this overload.
			static bool compare(const MetaAny& current_value, const MetaAny& comparison_value, ComparisonMethod comparison_method);

			// Retrieves a reference to the shared `EventTriggerConditionType` interface from an `EventTriggerCondition` variant.
			// TODO: Look into adding `std::monostate` behavior. (Would require changing to pointer return-type)
			template <typename ConditionWrapper>
			static const EventTriggerConditionType* get_condition_interface(const ConditionWrapper& condition) // EventTriggerCondition
			{
				const EventTriggerConditionType* out = nullptr;

				util::visit
				(
					condition.value,

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

				return out;
			}

			// Visits the exact type of `condition` if said type has type-introspection abilities.
			template <typename ConditionWrapper, typename Callback>
			static void visit_type_enabled(ConditionWrapper&& condition, Callback&& callback)
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

					condition.value
				);
			}

			// Resolves the underlying type of `condition`, then wraps the object in a `MetaAny` object.
			template <typename ConditionWrapper>
			static MetaAny opaque_condition(ConditionWrapper&& condition, bool move_content=false) // EventTriggerCondition
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
					condition.value,

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
			template <typename ConditionWrapper>
			static MetaAny opaque_copy(ConditionWrapper&& condition)
			{
				return opaque_condition
				(
					ConditionWrapper(std::forward<ConditionWrapper>(condition)),

					true
				);
			}

			template <typename Callback>
			static bool from_opaque_condition(const MetaAny& condition, Callback&& callback)
			{
				using namespace engine::literals;

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

			template <typename ConditionWrapper, typename Callback>
			static void from_condition_wrapper(ConditionWrapper&& condition, Callback&& callback)
			{
				std::visit
				(
					std::forward<Callback>(callback),

					condition.value
				);
			}

			template <typename Callback>
			static void from_remote_condition(const EntityDescriptor& descriptor, const RemoteConditionType& condition, Callback&& callback)
			{
				from_condition_wrapper(from_remote_condition(descriptor, condition), callback);
			}

			template <typename ConditionWrapper, typename ...Args>
			static bool get_condition_status(const ConditionWrapper& condition, Args&&... args) // EventTriggerCondition
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
							result = false;
						}
						else
						{
							result = check_condition(exact_condition);
						}
					},

					condition.value
				);

				return result;
			}

			virtual ~EventTriggerConditionType();

			virtual bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const = 0;
			virtual bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const = 0;
			virtual bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const = 0;
			virtual bool condition_met(const MetaAny& event_instance) const = 0;

			virtual bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const = 0;
			virtual bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const = 0;

			virtual EventTriggerCompoundMethod compound_method() const = 0;

			template <typename ...Args>
			inline bool operator()(Args&&... args) const
			{
				return condition_met(std::forward<Args>(args)...);
			}

			bool operator==(const EventTriggerConditionType&) const noexcept = default;
			bool operator!=(const EventTriggerConditionType&) const noexcept = default;

			/*
			inline bool operator==(const EventTriggerConditionType& value) const noexcept
			{
				return (&value == this);
			}

			inline bool operator!=(const EventTriggerConditionType& value) const noexcept
			{
				return !operator==(value);
			}
			*/
		protected:
			// Overload added for generic programming compatibility.
			static const EntityDescriptor& get_descriptor(Registry& registry, Entity entity, const MetaEvaluationContext& context);

			// Overload added for generic programming compatibility.
			static const EntityDescriptor* try_get_descriptor(Registry& registry, Entity entity, const MetaEvaluationContext& context);

			// Overload added for generic programming compatibility.
			static MetaAny get_component(const MetaType& component_type, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			// This overload is used internally; use the non-template version instead.
			template <typename ...Args>
			static bool compare(const MetaAny& current_value, const MetaAny& comparison_value, ComparisonMethod comparison_method, Args&&... args);
	};

	class EventTriggerSingleCondition : public EventTriggerConditionType
	{
		public:
			EventTriggerSingleCondition
			(
				MetaAny&& comparison_value,
				ComparisonMethod comparison_method=ComparisonMethod::Equal,
				MetaTypeID event_type_id={}
			);

			EventTriggerSingleCondition
			(
				MetaAny&& comparison_value,
				ComparisonMethod comparison_method,
				const MetaType& event_type
			);

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
				const MetaType& event_type
			);

			bool operator==(const EventTriggerSingleCondition&) const noexcept = default;
			bool operator!=(const EventTriggerSingleCondition&) const noexcept = default;

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;

			MetaAny get_member_value(const MetaAny& event_instance, bool resolve_underlying=true) const;
			MetaAny get_member_value(const MetaAny& event_instance, Registry& registry, Entity entity, bool resolve_underlying=true) const;
			MetaAny get_member_value(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context, bool resolve_underlying=true) const;

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
			inline std::size_t enumerate_conditions(const EntityDescriptor& descriptor, Callback&& callback, bool recursive=true) const
			{
				return enumerate_conditions(std::forward<Callback>(callback), recursive);
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
			inline bool enumerate_types(const EntityDescriptor& descriptor, Callback&& callback, bool recursive=true) const
			{
				return enumerate_types(std::forward<Callback>(callback), recursive);
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

			template <typename Callback>
			inline std::size_t enumerate_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, Callback&& callback, bool recursive=true, bool allow_true_condition=true) const
			{
				return enumerate_type_compatible(type_id, std::forward<Callback>(callback), recursive, allow_true_condition);
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

			inline std::size_t count_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return count_type_compatible(type_id, recursive, allow_true_condition);
			}

			inline bool has_type_compatible(MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return (count_type_compatible(type_id, recursive, allow_true_condition) > 0);
			}

			inline bool has_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return has_type_compatible(type_id, recursive, allow_true_condition);
			}
		private:
			template <typename ...Args>
			bool condition_met_as_component(const MetaType& component_type, const MetaAny& comparison_value, Args&&... args) const;

			template <typename ...Args>
			MetaAny get_indirect_member_value_impl(const MetaAny& event_instance, bool resolve_underlying, Args&&... args) const;

			// Default implementation of most explicit form of `condition_met`.
			// (Primarily responsible for resolving indirection from `comparison_value`)
			template <typename ...Args>
			bool condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const;

			template <typename ...Args>
			bool condition_met_comparison_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const;

			template <typename ...Args>
			bool standalone_boolean_result(Args&&... args) const;
		protected:
			MetaSymbolID event_type_member = {};
			MetaAny comparison_value = {};

			// NOTE: Optional; uses type of event instance if not specified.
			MetaTypeID event_type_id = {};

			ComparisonMethod comparison_method = ComparisonMethod::Equal;

			bool fallback_to_component : 1 = true;
			bool fallback_to_boolean_evaluation : 1 = true;
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

			bool operator==(const EventTriggerMemberCondition&) const noexcept = default;
			bool operator!=(const EventTriggerMemberCondition&) const noexcept = default;

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;

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

			// TODO: Determine if introspection functions make sense for this type. (see `has_type_introspection`)
		private:
			template <typename ...Args>
			bool condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const;
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
			using ConditionContainer = util::small_vector<RemoteConditionType, 4>; // 2 // 8 // MetaAny

			template <typename ConditionWrapper>
			static const EventTriggerCompoundCondition* get_compound_condition_interface(const ConditionWrapper& condition) // EventTriggerCondition
			{
				const EventTriggerCompoundCondition* out = nullptr;

				util::visit
				(
					condition.value,

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

			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			std::size_t add_condition(RemoteConditionType&& condition_in);

			std::size_t size() const;
			bool empty() const;

			inline const ConditionContainer& get_conditions() const
			{
				return conditions;
			}

			inline ConditionContainer& get_conditions()
			{
				return conditions;
			}

			template <typename Callback>
			inline std::size_t enumerate_conditions(const EntityDescriptor& descriptor, Callback&& callback, bool recursive=true) const
			{
				std::size_t condition_count = 0;

				for (const auto& remote_condition : conditions)
				{
					bool should_break = false;

					from_remote_condition
					(
						descriptor,
						remote_condition,

						[&descriptor, &callback, &condition_count, recursive, &should_break](const auto& condition)
						{
							if constexpr (std::is_base_of_v<EventTriggerCompoundCondition, std::decay_t<decltype(condition)>>)
							{
								if (!recursive)
								{
									return;
								}

								condition_count += condition.enumerate_conditions(descriptor, callback, recursive);
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
			inline bool enumerate_types(const EntityDescriptor& descriptor, Callback&& callback, bool recursive=true) const
			{
				enumerate_conditions
				(
					descriptor,

					[&descriptor, &callback, recursive](const auto& condition)
					{
						auto on_type_enabled = [&descriptor, &callback, recursive](const auto& condition)
						{
							return condition.enumerate_types(descriptor, callback, recursive);
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
			inline std::size_t enumerate_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, Callback&& callback, bool recursive=true, bool allow_true_condition=true) const
			{
				std::size_t type_compatible = 0;

				enumerate_conditions
				(
					descriptor,

					[&descriptor, type_id, recursive, allow_true_condition, &callback, &type_compatible](const auto& condition) -> bool
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
							if (condition.enumerate_type_compatible(descriptor, type_id, callback, recursive, allow_true_condition) > 0)
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

			inline std::size_t count_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return enumerate_type_compatible
				(
					descriptor,
					type_id,

					// Empty lambda; no additional operations needed.
					[](const auto& condition) {},

					recursive, allow_true_condition
				);
			}

			inline bool has_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				return (count_type_compatible(descriptor, type_id, recursive, allow_true_condition) > 0);
			}
		protected:
			ConditionContainer conditions;
	};

	class EventTriggerAndCondition : public EventTriggerCompoundCondition
	{
		public:
			bool operator==(const EventTriggerAndCondition&) const noexcept = default;
			bool operator!=(const EventTriggerAndCondition&) const noexcept = default;

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;

			EventTriggerCompoundMethod compound_method() const override;
		private:
			template <typename ...Args>
			bool implicit_condition_met_impl(const MetaAny& event_instance, Args&&... args) const;

			template <typename ...Args>
			bool explicit_condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const;
	};

	class EventTriggerOrCondition : public EventTriggerCompoundCondition
	{
		public:
			bool operator==(const EventTriggerOrCondition&) const noexcept = default;
			bool operator!=(const EventTriggerOrCondition&) const noexcept = default;

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;

			EventTriggerCompoundMethod compound_method() const override;
		private:
			template <typename ...Args>
			bool implicit_condition_met_impl(const MetaAny& event_instance, Args&&... args) const;

			template <typename ...Args>
			bool explicit_condition_met_impl(const MetaAny& event_instance, const MetaAny& comparison_value, Args&&... args) const;
	};

	class EventTriggerTrueCondition : public EventTriggerConditionType
	{
		public:
			inline bool operator==(const EventTriggerTrueCondition&) const noexcept { return true; }
			inline bool operator!=(const EventTriggerTrueCondition&) const noexcept { return true; }

			inline bool operator==(const EventTriggerFalseCondition&) const noexcept { return false; }
			inline bool operator!=(const EventTriggerFalseCondition&) const noexcept { return false; }

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;
	};

	class EventTriggerFalseCondition : public EventTriggerConditionType
	{
		public:
			inline bool operator==(const EventTriggerFalseCondition&) const noexcept { return true; }
			inline bool operator!=(const EventTriggerFalseCondition&) const noexcept { return true; }

			inline bool operator==(const EventTriggerTrueCondition&) const noexcept { return false; }
			inline bool operator!=(const EventTriggerTrueCondition&) const noexcept { return false; }

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;
	};

	// Inverts the result of `condition_met` for a given condition object.
	class EventTriggerInverseCondition : public EventTriggerConditionType
	{
		public:
			EventTriggerInverseCondition(RemoteConditionType&& inv_condition);

			bool operator==(const EventTriggerInverseCondition&) const noexcept = default;
			bool operator!=(const EventTriggerInverseCondition&) const noexcept = default;

			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value, Registry& registry, Entity entity, const MetaEvaluationContext& context) const override;
			bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const override;
			bool condition_met(const MetaAny& event_instance) const override;

			EventTriggerCompoundMethod compound_method() const override;

			const EventTriggerConditionType& get_inverse(const EntityDescriptor& descriptor) const;
			const EventTriggerConditionType& get_inverse(Registry& registry, Entity entity) const;

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
			inline std::size_t enumerate_conditions(const EntityDescriptor& descriptor, Callback&& callback, bool recursive=true) const
			{
				return enumerate_conditions(std::forward<Callback>(callback), recursive);
			}

			template <typename Callback>
			inline bool enumerate_types(const EntityDescriptor& descriptor, Callback&& callback, bool recursive=true) const
			{
				bool result = false;

				from_remote_condition
				(
					descriptor,
					inv_condition,

					[&descriptor, &callback, recursive, &result](const auto& condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							result = condition.enumerate_types(descriptor, callback, recursive);
						}
					}
				);

				return result;
			}

			template <typename Callback>
			inline std::size_t enumerate_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, Callback&& callback, bool recursive=true, bool allow_true_condition=true) const
			{
				std::size_t result = 0;

				from_remote_condition
				(
					descriptor,
					inv_condition,

					[&descriptor, type_id, &callback, recursive, allow_true_condition, &result](const auto& condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							result = condition.enumerate_type_compatible(descriptor, type_id, callback, recursive, allow_true_condition);
						}
					}
				);

				return result;
			}

			inline std::size_t count_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				std::size_t result = 0;

				from_remote_condition
				(
					descriptor,
					inv_condition,

					[&descriptor, type_id, recursive, allow_true_condition, &result](const auto& condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							result = condition.count_type_compatible(descriptor, type_id, recursive, allow_true_condition);
						}
					}
				);

				return result;
			}

			inline bool has_type_compatible(const EntityDescriptor& descriptor, MetaTypeID type_id, bool recursive=true, bool allow_true_condition=true) const
			{
				bool result = false;

				from_remote_condition
				(
					descriptor,
					inv_condition,

					[&descriptor, type_id, recursive, allow_true_condition, &result](const auto& condition)
					{
						if constexpr (has_type_introspection<std::decay_t<decltype(condition)>>())
						{
							result = condition.has_type_compatible(descriptor, type_id, recursive, allow_true_condition);
						}
					}
				);

				return result;
			}

		protected:
			RemoteConditionType inv_condition;
	};

	// Variant type used to represent an event-trigger condition held by an external type. (e.g. `EntityStateRule`)
	// TODO: Look into adding `std::monostate` option here. (Currently using `std::optional` to wrap 'no condition' scenario)
	using EventTriggerConditionVariant = std::variant
	<
		EventTriggerSingleCondition,
		EventTriggerMemberCondition,
		EventTriggerAndCondition,
		EventTriggerOrCondition,
		EventTriggerTrueCondition,
		EventTriggerFalseCondition,
		EventTriggerInverseCondition
	>;

	class EventTriggerCondition
	{
		public:
			using ValueType = EventTriggerConditionVariant;

			ValueType value;

			EventTriggerCondition(const ValueType& value)
				: value(value) {}

			EventTriggerCondition(ValueType&& value) noexcept
				: value(std::move(value)) {}

			EventTriggerCondition(const EventTriggerCondition&) = default;
			EventTriggerCondition(EventTriggerCondition&&) noexcept = default;

			EventTriggerCondition& operator=(const EventTriggerCondition&) = default;
			EventTriggerCondition& operator=(EventTriggerCondition&&) noexcept = default;

			EventTriggerCondition& operator=(ValueType&& value_in) noexcept
			{
				value = std::move(value_in);

				return *this;
			}

			EventTriggerCondition& operator=(const ValueType& value_in)
			{
				value = value_in;

				return *this;
			}

			inline auto index() const { return value.index(); }

			bool operator==(const EventTriggerCondition&) const noexcept = default;
			bool operator!=(const EventTriggerCondition&) const noexcept = default;

			inline bool operator==(const ValueType& value_in) const noexcept
			{
				return (value == value_in);
			}

			inline bool operator!=(const ValueType& value_in) const noexcept
			{
				return (value != value_in);
			}
	};
}