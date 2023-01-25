#pragma once

#include "types.hpp"
#include "event_trigger_condition.hpp"
#include "entity_target.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/meta.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/parsing_context.hpp>

//#include <util/json.hpp>

#include <string>
#include <string_view>
#include <optional>
#include <type_traits>
#include <stdexcept>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	class EntityDescriptor;

	MetaAny process_trigger_condition_value(std::string_view compared_value_raw);

	std::optional<EventTriggerSingleCondition> process_standard_trigger_condition // std::optional<EventTriggerCondition>
	(
		const entt::meta_type& type,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,

		// Used for debugging purposes, etc.
		std::string_view trigger_condition_expr={},

		bool embed_type_in_condition=false
	);

	std::optional<EventTriggerMemberCondition> process_member_trigger_condition
	(
		const entt::meta_type& type,

		std::string_view entity_ref,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,
				
		// Used for debugging purposes, etc.
		std::string_view trigger_condition_expr={}
	);

	// Attempts to concatenate/add `condition_to_add` (exact-type) to `condition_out` (opaque-type) using the method described by `combinator`.
	// NOTE: If the underlying type of `condition_out` is a non-compound type, it will be promoted to one based on the `combinator` requested.
	//
	// If concatenation cannot be performed, the `store_condition` callback will be called.
	// 
	// If `store_condition` does not return a value, or if the returned value is `true`, the existing contents of
	// `condition_out` will be move-assigned to `condition_to_add`, representing a new condition-block.
	template <typename AddedConditionType, typename StorageRoutine>
	void concatenate_conditions
	(
		EventTriggerCondition& condition_out,
		AddedConditionType&& condition_to_add,
		EventTriggerCompoundMethod combinator,
		StorageRoutine&& store_condition
	)
	{
		using Combinator = EventTriggerCompoundMethod;

		// Simplified wrapper for `store_condition`.
		auto store = [&store_condition, &condition_out]() -> bool
		{
			if constexpr (std::is_invocable_r_v<bool, StorageRoutine>)
			{
				return store_condition();
			}
			else if constexpr (std::is_invocable_r_v<void, StorageRoutine>)
			{
				store_condition();

				return true;
			}
			else if constexpr (std::is_invocable_r_v<bool, StorageRoutine, EventTriggerCondition&>)
			{
				return store_condition(condition_out);
			}
			else if constexpr (std::is_invocable_r_v<void, StorageRoutine, EventTriggerCondition&>)
			{
				store_condition(condition_out);

				return true;
			}
			else
			{
				static_assert
				(
					std::integral_constant<StorageRoutine, false>::value,
					"Unable to determine suitable overload for condition storage routine."
				);
			}
		};

		// Covers the first time visiting a condition. (Fragment only; no AND/OR container generated)
		// 'Basic condition' scenarios: `EventTriggerSingleCondition`, `EventTriggerTrueCondition`, `EventTriggerFalseCondition`, `EventTriggerInverseCondition` (see below)
		auto basic_condition = [combinator, &store, &condition_out, &condition_to_add](auto& condition)
		{
			switch (combinator)
			{
				case Combinator::And:
				{
					// Generate an AND container:
					EventTriggerAndCondition and_condition;

					and_condition.add_condition(std::move(condition));
					and_condition.add_condition(std::move(condition_to_add));

					// Store the newly generated container in place of the old condition-fragment.
					condition_out = std::move(and_condition);

					break;
				}
				case Combinator::Or:
				{
					// Generate an OR container:
					EventTriggerOrCondition or_condition;

					if constexpr (std::is_same_v<std::decay_t<decltype(condition)>, EventTriggerTrueCondition>)
					{
						print_warn("Attempting to add 'always-true' condition(s) to 'OR' compound condition.");
					}

					or_condition.add_condition(std::move(condition));
					or_condition.add_condition(std::move(condition_to_add));

					// Store the newly generated container in place of the old condition-fragment.
					condition_out = std::move(or_condition);

					break;
				}

				default:
					// In the event that there is no conbinator, finish processing (or store) the existing
					// condition/fragment and swap in the newly generated one:
					if (store())
					{
						condition_out = std::move(condition_to_add);
					}

					break;
			}
		};

		util::visit
		(
			condition_out,

			[&basic_condition](EventTriggerSingleCondition& single_condition)   { basic_condition(single_condition);  },
			[&basic_condition](EventTriggerMemberCondition& member_condition)   { basic_condition(member_condition);  },
			[&basic_condition](EventTriggerTrueCondition& true_condition)       { basic_condition(true_condition);    },
			[&basic_condition](EventTriggerFalseCondition& false_condition)     { basic_condition(false_condition);   },
			[&basic_condition](EventTriggerInverseCondition& inverse_condition) { basic_condition(inverse_condition); },

			// Existing AND container:
			[combinator, &store, &condition_out, &condition_to_add](EventTriggerAndCondition& and_condition)
			{
				switch (combinator)
				{
					case Combinator::And:
						// Add to this container as usual.
						and_condition.add_condition(std::move(condition_to_add));

						break;
					case Combinator::Or:
						// The user has indicated a switch to an OR clause.
						// Process this AND container as-is, then swap in the
						// new condition fragment for further operation.
						if (store())
						{
							condition_out = std::move(condition_to_add);
						}

						break;

					default:
						// Unsupported conbinator; ignore this fragment.

						break;
				}
			},

			// Existing OR container:
			[combinator, &store, &condition_out, &condition_to_add](EventTriggerOrCondition& or_condition)
			{
				switch (combinator)
				{
					case Combinator::And:
						// The user has indicated a switch to an AND clause.
						// Process this OR container as-is, then swap in the
						// new condition fragment for further operation.
						if (store())
						{
							condition_out = std::move(condition_to_add);
						}

						break;
					case Combinator::Or:
						// Add to this container as usual.
						or_condition.add_condition(std::move(condition_to_add));

						break;

					default:
						// Unsupported conbinator; ignore this fragment.

						break;
				}
			}
		);
	}

	// This overload assumes that attempting to store or otherwise flush the
	// contents of `condition_out` is an invalid operation that should never happen.
	template <typename AddedConditionType>
	void concatenate_conditions
	(
		EventTriggerCondition& condition_out,
		AddedConditionType&& condition_to_add,
		EventTriggerCompoundMethod combinator
	)
	{
		concatenate_conditions
		(
			condition_out,
			std::forward<AddedConditionType>(condition_to_add),
			combinator,

			[]() -> bool
			{
				// Illegal operation.
				assert(false);
				
				// If asserts are disabled, ensure this is a no-op.
				return false;
			}
		);
	}

	// Processes the expression specified, executing `callback` for
	// every condition block or compound-condition block generated.
	// 
	// Blocks are generally defined as a series of rules (i.e. member-comparisons)
	// applicable to a single event or component type.
	//
	// The `callback` argument should be a callable expecting a `MetaTypeID` and an `std::optional<EventTriggerCondition>` as input.
	template <typename Callback>
	void process_trigger_expression
	(
		const std::string& trigger_condition_expr, // std::string_view
		Callback&& callback,
		bool always_embed_type=false,
		const ParsingContext* opt_parsing_context=nullptr
	)
	{
		//using namespace entt::literals;
		using Combinator = EventTriggerCompoundMethod;

		std::size_t offset = 0; // std::ptrdiff_t

		std::optional<EventTriggerCondition> condition_out;
		std::optional<MetaType> active_type;
		Combinator active_combinator = Combinator::None;

		bool is_first_expr = true;

		// NOTE: Nesting (i.e. parentheses) is not currently supported.
		// Therefore, a 'store' operation simply processes a rule immediately, consuming the condition object.
		auto store_condition = [&active_type, &condition_out, &callback]()
		{
			if (!condition_out.has_value())
			{
				return;
			}

			if (!active_type)
			{
				return;
			}

			callback(active_type->id(), std::move(*condition_out));

			// Ensure `std::nullopt` status.
			// (As opposed to 'moved-from' status)
			condition_out = std::nullopt;
		};

		do
		{
			auto
			[
				entity_ref,
				type_name,
				member_name,
				comparison_operator,
				compared_value_raw,
				updated_offset
			] = util::parse_qualified_assignment_or_comparison(trigger_condition_expr, static_cast<std::ptrdiff_t>(offset));

			bool is_standalone_event_trigger = false;

			if (type_name.empty())
			{
				auto standalone_event_result = parse_event_type(trigger_condition_expr, static_cast<std::ptrdiff_t>(offset));

				type_name = std::get<0>(standalone_event_result);

				if (type_name.empty())
				{
					// Exit the loop, nothing else can be processed.
					break;
				}
				else
				{
					updated_offset = std::get<1>(standalone_event_result);

					is_standalone_event_trigger = true;
				}
			}

			auto expr_start_idx = static_cast<std::size_t>
			(
				(
					(entity_ref.empty())
					? type_name.data()
					: entity_ref.data()
				)
				-
				trigger_condition_expr.data()
			);

			//auto processed_length = (static_cast<std::size_t>(updated_offset) - offset);

			auto parsed_expr = std::string_view
			{
				(trigger_condition_expr.data()+expr_start_idx),
				static_cast<std::size_t>(updated_offset-expr_start_idx)
			};

			if (!is_first_expr)
			{
				// Space between the old `offset` (i.e. end of previous expression) and the start of current expression.
				auto expr_gap = std::string_view
				{
					(trigger_condition_expr.data() + offset),
					(expr_start_idx - offset)
				};

				auto local_combinator_symbol_idx = std::string_view::npos;

				// Perform view-local lookup:
				if (local_combinator_symbol_idx = expr_gap.find("&&"); local_combinator_symbol_idx != std::string_view::npos)
				{
					active_combinator = Combinator::And;
				}
				else if (local_combinator_symbol_idx = expr_gap.find("||"); local_combinator_symbol_idx != std::string_view::npos)
				{
					active_combinator = Combinator::Or;
				}
					
				if (local_combinator_symbol_idx == std::string_view::npos)
				{
					print_warn("Missing combinator symbol in state-rule trigger expression; using last known combinator as fallback.");
				}
				/*
				else
				{
					// Convert from view-local index to global index.
					const auto combinator_symbol_idx = static_cast<std::size_t>((expr_gap.data() + local_combinator_symbol_idx) - trigger_condition_expr.data());

					constexpr std::size_t combinator_symbol_size = 2;
					if (trigger_condition_expr.find("(", (combinator_symbol_idx + combinator_symbol_size)) != std::string_view::npos)
					{
						// ...
					}
				}
				*/
			}
				
			// Update the processing offset to the latest location reported by our parsing step.
			offset = static_cast<std::size_t>(updated_offset);

			bool active_type_changed = false;

			MetaType expr_type = (opt_parsing_context)
				? opt_parsing_context->get_type(type_name) // TODO: Optimize.
				: resolve(hash(type_name).value())
			;

			if (!active_type.has_value())
			{
				//assert(expr_type);

				if (!expr_type)
				{
					throw std::runtime_error(util::format("Unable to resolve trigger/event type: \"{}\"", type_name));
				}

				active_type = expr_type;
			}
			else if (active_combinator == Combinator::And)
			{
				if (!expr_type)
				{
					// NOTE: `expr_type` is technically considered optional in the case of `And` combinators.
					// (i.e. we use the active type instead)
					expr_type = *active_type;
				}
			}
			else // && (active_combinator == Combinator::Or)
			{
				//assert(expr_type);

				if (!expr_type)
				{
					throw std::runtime_error(util::format("Unable to resolve trigger/event type in compound condition: \"{}\"", type_name));
				}

				if (active_type->id() != expr_type.id())
				{
					// Since combining event types in trigger-clauses is not supported,
					// we need to process the condition we've already built.
					store_condition();

					// With the previous condition handled, we can switch to the new event-type:
					active_type = expr_type;

					active_type_changed = true;
				}
			}

			//assert(expr_type);
			assert(active_type);

			auto on_condition = [&store_condition](EventTriggerCondition& condition_out, auto&& generated_condition, Combinator combinator)
			{
				concatenate_conditions(condition_out, std::forward<decltype(generated_condition)>(generated_condition), combinator, store_condition);
			};
			
			// Workaround for compound AND/OR on 'standalone' (no-condition) of same event type.
			// Without this workaround, an extra rule would be generated.
			// 
			// NOTE: This does not cover duplication through multiple triggers, only duplication within the same trigger expression.
			// TODO: Look into implications of duplicated triggers.
			if ((is_standalone_event_trigger) && (!is_first_expr) && (!active_type_changed))
			{
				//print_warn("Detected conditionless event-trigger of same type as previous conditioned trigger -- generating dummy 'always-true' condition to avoid anomalies.");
				//on_condition(*condition_out, EventTriggerTrueCondition {}, active_combinator);

				print_warn("Detected conditionless event-trigger of same type as previous conditioned trigger -- ignoring to avoid anomalies.");
			}
			else
			{
				auto handle_condition = [&on_condition, &active_combinator, &condition_out](auto&& generated_condition)
				{
					// Attempt to resolve a single condition (i.e. fragment) from the expression we parsed.
					if (generated_condition) // *active_type
					{
						if (condition_out.has_value())
						{
							on_condition(*condition_out, *generated_condition, active_combinator);
						}
						else
						{
							condition_out = std::move(*generated_condition);
						}
					}
					else
					{
						if (condition_out.has_value())
						{
							on_condition(*condition_out, EventTriggerTrueCondition {}, active_combinator);
						}
						else
						{
							condition_out = EventTriggerTrueCondition {};
						}
					}
				};

				if (entity_ref.empty())
				{
					// Standard event triggers:
					handle_condition
					(
						process_standard_trigger_condition
						(
							expr_type, member_name,
							comparison_operator, compared_value_raw,
							parsed_expr,

							(always_embed_type || (expr_type != *active_type))
						)
					);
				}
				else
				{
					// Fully qualified component-member event triggers:
					handle_condition
					(
						process_member_trigger_condition
						(
							expr_type, entity_ref, member_name,
							comparison_operator, compared_value_raw,
							parsed_expr
						)
					);
				}
			}

			is_first_expr = false;

			// Debugging related:
			/*
			auto remaining_length = (trigger_condition_expr.size() - static_cast<std::size_t>(updated_offset));
			auto remaining = std::string_view{ (trigger_condition_expr.data() + updated_offset), remaining_length };

			print(remaining);
			*/
		} while (offset < trigger_condition_expr.size());

		if (!active_type)
		{
			return;
		}

		// Process the rule using whatever's left.
		if (condition_out)
		{
			callback(active_type->id(), std::move(*condition_out));
		}
		else
		{
			callback(active_type->id(), std::nullopt);
		}
	}
	
	// Processes an 'all-in-one' (unified) condition-block from `trigger_condition_expr`.
	// This condition cannot be used in a traditional rule/trigger scenario, since it could encode multiple primary types.
	inline std::optional<EventTriggerCondition> process_unified_condition_block(const std::string& trigger_condition_expr, const ParsingContext* opt_parsing_context=nullptr) // std::string_view
	{
		using Combinator = EventTriggerCompoundMethod;

		std::optional<EventTriggerCondition> condition_out = std::nullopt;

		process_trigger_expression
		(
			trigger_condition_expr,

			[&condition_out](MetaTypeID type_id, std::optional<EventTriggerCondition> condition)
			{
				if (!condition.has_value())
				{
					return;
				}

				if (condition_out.has_value())
				{
					concatenate_conditions(*condition_out, std::move(*condition), Combinator::Or);
				}
				else
				{
					condition_out = std::move(condition);
				}
			},

			// Always embed type information. (Required for `EntityThread` runtime)
			true,

			opt_parsing_context
		);

		return condition_out;
	}

	// TODO: Optimize. (Temporary string generated due to limitation with `std::regex`)
	inline std::optional<EventTriggerCondition> process_unified_condition_block(std::string_view trigger_condition_expr, const ParsingContext* opt_parsing_context=nullptr)
	{
		return process_unified_condition_block(std::string(trigger_condition_expr), opt_parsing_context);
	}
}