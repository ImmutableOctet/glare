#pragma once

#include "serial.hpp"

#include "types.hpp"
#include "event_trigger_condition.hpp"
#include "entity_target.hpp"
#include "entity_descriptor.hpp"
#include "entity_thread_target.hpp"
#include "entity_thread_range.hpp"
#include "entity_thread_builder.hpp"

#include "events.hpp"

//#include <engine/meta/meta.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/meta_type_resolution_context.hpp>
#include <engine/meta/meta_parsing_instructions.hpp>
#include <engine/meta/meta_variable_context.hpp>
#include <engine/meta/shared_storage_interface.hpp>
#include <engine/meta/meta_value_operator.hpp>
#include <engine/meta/meta_value_operation.hpp>
#include <engine/meta/meta_variable_target.hpp>
#include <engine/meta/indirect_meta_variable_target.hpp>

//#include <util/json.hpp>
#include <util/magic_enum.hpp>

#include <string>
#include <string_view>
#include <optional>
#include <type_traits>
//#include <stdexcept>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	class EntityDescriptor;
	class EntityState;

	// Subroutine of `process_member_trigger_condition`, `process_standard_trigger_condition`, etc.
	MetaAny process_trigger_condition_value
	(
		EntityDescriptor& descriptor,
		std::string_view compared_value_raw,
		const MetaParsingContext& opt_parsing_context={}
	);

	// Subroutine of `process_trigger_expression`.
	std::optional<EventTriggerSingleCondition> process_standard_trigger_condition // std::optional<EventTriggerCondition>
	(
		EntityDescriptor& descriptor,

		const entt::meta_type& type,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,

		// Used for debugging purposes, etc.
		std::string_view trigger_condition_expr={},

		bool embed_type_in_condition=false,
		bool invert_comparison_operator=false,

		const MetaParsingContext& opt_parsing_context={}
	);

	// Subroutine of `process_trigger_expression`.
	std::optional<EventTriggerMemberCondition> process_member_trigger_condition
	(
		EntityDescriptor& descriptor,

		const entt::meta_type& type,

		std::string_view entity_ref,

		std::string_view member_name,
		std::string_view comparison_operator,
		std::string_view compared_value_raw,
		
		// Used for debugging purposes, etc.
		std::string_view trigger_condition_expr={},

		bool invert_comparison_operator=false,

		const MetaParsingContext& opt_parsing_context={}
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
		EntityDescriptor& descriptor,
		EventTriggerCondition& condition_out,
		AddedConditionType&& condition_to_add,
		EventTriggerCompoundMethod combinator,
		StorageRoutine&& store_condition
	)
	{
		using Combinator = EventTriggerCompoundMethod;

		// Simplified wrapper for `store_condition`.
		auto store = [&store_condition, &condition_out, &condition_to_add]() -> bool
		{
			if constexpr (std::is_invocable_r_v<bool, StorageRoutine, EventTriggerCondition&, AddedConditionType&>)
			{
				return store_condition(condition_out, condition_to_add);
			}
			else if constexpr (std::is_invocable_r_v<void, StorageRoutine, EventTriggerCondition&, AddedConditionType&>)
			{
				store_condition(condition_out, condition_to_add);

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
			else if constexpr (std::is_invocable_r_v<bool, StorageRoutine>)
			{
				return store_condition();
			}
			else if constexpr (std::is_invocable_r_v<void, StorageRoutine>)
			{
				store_condition();

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
		auto basic_condition = [&descriptor, combinator, &store, &condition_out, &condition_to_add](auto& condition)
		{
			switch (combinator)
			{
				case Combinator::And:
				{
					// Generate an AND container:
					EventTriggerAndCondition and_condition;

					and_condition.add_condition(descriptor.allocate<EventTriggerCondition>(std::move(condition)));
					and_condition.add_condition(descriptor.allocate<EventTriggerCondition>(std::move(condition_to_add)));

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

					or_condition.add_condition(descriptor.allocate<EventTriggerCondition>(std::move(condition)));
					or_condition.add_condition(descriptor.allocate<EventTriggerCondition>(std::move(condition_to_add)));

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
			condition_out.value,

			[&basic_condition](EventTriggerSingleCondition& single_condition)   { basic_condition(single_condition);  },
			[&basic_condition](EventTriggerMemberCondition& member_condition)   { basic_condition(member_condition);  },
			[&basic_condition](EventTriggerTrueCondition& true_condition)       { basic_condition(true_condition);    },
			[&basic_condition](EventTriggerFalseCondition& false_condition)     { basic_condition(false_condition);   },
			[&basic_condition](EventTriggerInverseCondition& inverse_condition) { basic_condition(inverse_condition); },

			// Existing AND container:
			[&descriptor, combinator, &store, &condition_out, &condition_to_add](EventTriggerAndCondition& and_condition)
			{
				switch (combinator)
				{
					case Combinator::And:
						// Add to this container as usual.
						and_condition.add_condition(descriptor.allocate<EventTriggerCondition>(std::move(condition_to_add)));

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
			[&descriptor, combinator, &store, &condition_out, &condition_to_add](EventTriggerOrCondition& or_condition)
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
						or_condition.add_condition(descriptor.allocate<EventTriggerCondition>(std::move(condition_to_add)));

						break;

					default:
						// Unsupported conbinator; ignore this fragment.

						break;
				}
			}
		);
	}

	// This overload performs in-line combinations, which may
	// not be supported in event-driven scenarios.
	// (Usually due to a required event-type association)
	template <typename AddedConditionType>
	void concatenate_conditions
	(
		EntityDescriptor& descriptor,
		EventTriggerCondition& condition_out,
		AddedConditionType&& condition_to_add,
		EventTriggerCompoundMethod combinator
	)
	{
		concatenate_conditions
		(
			descriptor,
			condition_out,
			std::forward<AddedConditionType>(condition_to_add),
			combinator,

			[&descriptor, combinator](auto& existing_instance, auto& new_instance) -> bool
			{
				auto inline_combine = [&descriptor, &existing_instance, &new_instance]<typename CompoundType>() -> bool
				{
					auto condition_out = CompoundType {};

					condition_out.add_condition(descriptor.allocate<EventTriggerCondition>(std::move(existing_instance)));
					condition_out.add_condition(descriptor.allocate<EventTriggerCondition>(std::move(new_instance)));

					existing_instance = std::move(condition_out);

					// Ownership of `new_instance` taken.
					return false;
				};

				switch (combinator)
				{
					case EventTriggerCompoundMethod::And:
						return inline_combine.operator()<EventTriggerAndCondition>();

					case EventTriggerCompoundMethod::Or:
						return inline_combine.operator()<EventTriggerOrCondition>();

					case EventTriggerCompoundMethod::None:
						break;
				}
				
				return true;
			}
		);
	}

	// TODO: Rework this routine to properly take advantage of `meta_any_from_string`'s parsing functionality,
	// rather than manually parsing portions of the expression.
	// 
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
		EntityDescriptor& descriptor,
		std::string_view trigger_condition_expr,
		Callback&& callback,
		bool always_embed_type=false,
		const MetaParsingContext& opt_parsing_context={},
		bool encode_default_variable_comparison_as_null_check=false
	)
	{
		using namespace engine::literals;

		using Combinator = EventTriggerCompoundMethod;

		std::size_t offset = 0; // std::ptrdiff_t

		std::optional<EventTriggerCondition> condition_out;
		
		auto active_type = MetaType {};
		auto active_combinator = Combinator::None;

		bool is_first_expr = true;

		// NOTE: Nesting (i.e. parentheses) is not currently supported.
		// Therefore, a 'store' operation simply processes a rule immediately, consuming the condition object.
		auto store_condition = [&active_type, &condition_out, &callback](bool fallback_to_empty=true) // -> bool
		{
			if (!condition_out.has_value())
			{
				return;
			}

			if (!active_type)
			{
				return;
			}

			callback(active_type.id(), std::move(*condition_out));

			// NOTE: If `fallback_to_empty` is set to false,
			// `condition_out` will be left in a 'moved-from' state.
			if (fallback_to_empty)
			{
				condition_out = std::nullopt;
			}
		};

		do
		{
			bool invert_condition = false;

			if (auto [leading_operator, remainder] = util::parse_standard_operator_segment(trigger_condition_expr); (leading_operator == "!"))
			{
				invert_condition = true;

				trigger_condition_expr = remainder;
			}

			auto
			[
				entity_ref,
				first_symbol,
				second_symbol,
				comparison_operator,
				compared_value_raw,
				updated_offset
			] = parse_qualified_assignment_or_comparison
			(
				trigger_condition_expr, offset, {},
				true, true, true, true, true
			);

			auto& type_name = first_symbol;
			auto& member_name = second_symbol;

			bool is_standalone_event_trigger = false;

			if (type_name.empty())
			{
				/*
				// Disabled for now (No longer needed with current implementation of `parse_qualified_assignment_or_comparison`):
				
				// TODO: Optimize. (Temporary string created due to limitation of `std::regex`)
				auto standalone_event_result = parse_event_type(std::string { trigger_condition_expr }, offset);

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
				*/

				break;
			}
			else
			{
				// See above notes about behavior of `parse_qualified_assignment_or_comparison`.
				if (second_symbol.empty() && compared_value_raw.empty())
				{
					assert(comparison_operator.empty());

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

				auto [local_combinator_symbol_idx, local_combinator_symbol] = util::find_logic_operator(expr_gap, false);

				if (local_combinator_symbol_idx == std::string_view::npos)
				{
					print_warn("Missing combinator symbol in state-rule trigger expression; using last known combinator as fallback.");
				}
				else
				{
					// Perform view-local lookup:
					if (local_combinator_symbol == "&&")
					{
						active_combinator = Combinator::And;
					}
					else if (local_combinator_symbol == "||")
					{
						active_combinator = Combinator::Or;
					}
					else
					{
						// Unsupported symbol.
						//assert(false);
					}
				}
			}
				
			// Update the processing offset to the latest location reported by our parsing step.
			offset = static_cast<std::size_t>(updated_offset);

			bool active_type_changed = false;

			const auto opt_type_context = opt_parsing_context.get_type_context();

			MetaType expr_type = (opt_type_context)
				? opt_type_context->get_type(type_name) // TODO: Optimize.
				: resolve(hash(type_name).value())
			;

			if (!active_type)
			{
				//assert(expr_type);

				if (expr_type)
				{
					active_type = expr_type;
				}
			}
			else if (active_combinator == Combinator::And)
			{
				if (!expr_type)
				{
					// NOTE: `expr_type` is technically considered optional in the case of `And` combinators.
					// (i.e. we use the active type instead)
					expr_type = active_type;
				}
			}
			else // && (active_combinator == Combinator::Or)
			{
				if (!expr_type || (active_type.id() != expr_type.id()))
				{
					// Since combining event types in trigger-clauses is not supported,
					// we need to process the condition we've already built.
					store_condition(true);
				}

				if (expr_type)
				{
					// With the previous condition handled, we can switch to the new event-type:
					active_type = expr_type;

					active_type_changed = true;
				}
				/*
				else
				{
					//throw std::runtime_error(util::format("Unable to resolve trigger/event type in compound condition: \"{}\"", type_name));
					//print_warn("Unable to resolve trigger/event type in compound condition: \"{}\"", type_name);

					break;
				}
				*/
			}

			auto on_condition = [&descriptor, &store_condition](EventTriggerCondition& condition_out, auto&& generated_condition, Combinator combinator)
			{
				concatenate_conditions
				(
					descriptor,
					condition_out,
					std::forward<decltype(generated_condition)>(generated_condition),
					combinator,

					[&store_condition]()
					{
						store_condition(false);
					}
				);
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
				auto handle_condition = [&on_condition, &condition_out](auto combinator, auto&& generated_condition)
				{
					if constexpr (std::is_base_of_v<EventTriggerConditionType, std::decay_t<decltype(generated_condition)>>)
					{
						if (condition_out.has_value())
						{
							on_condition(*condition_out, generated_condition, combinator);
						}
						else
						{
							condition_out = { std::move(generated_condition) };
						}
					}
					else
					{
						// Attempt to resolve a single condition (i.e. fragment) from the expression we parsed.
						if (generated_condition) // *active_type
						{
							if (condition_out.has_value())
							{
								on_condition(*condition_out, *generated_condition, combinator); // std::move(*generated_condition)
							}
							else
							{
								condition_out = { std::move(*generated_condition) };
							}
						}
						else
						{
							if (condition_out.has_value())
							{
								on_condition(*condition_out, EventTriggerTrueCondition {}, combinator);
							}
							else
							{
								condition_out = { EventTriggerTrueCondition {} };
							}
						}
					}
				};

				if (expr_type)
				{
					if (entity_ref.empty())
					{
						// Standard event triggers:
						handle_condition
						(
							active_combinator,

							process_standard_trigger_condition
							(
								descriptor,
								expr_type, member_name,
								comparison_operator, compared_value_raw,
								parsed_expr,

								(always_embed_type || (expr_type != active_type)),
								invert_condition,
								opt_parsing_context
							)
						);
					}
					else
					{
						// Fully qualified component-member event triggers:
						handle_condition
						(
							active_combinator,

							process_member_trigger_condition
							(
								descriptor,
								expr_type, entity_ref, member_name,
								comparison_operator, compared_value_raw,
								parsed_expr,
								invert_condition,
								opt_parsing_context
							)
						);
					}
				}
				else
				{
					auto variable_name = std::string_view {};
					auto thread_name = std::string_view {};

					if (first_symbol.empty())
					{
						break;
					}
					else
					{
						if (second_symbol.empty())
						{
							variable_name = first_symbol;
						}
						else
						{
							thread_name = first_symbol;
							variable_name = second_symbol;
						}
					}

					auto thread_id = (thread_name.empty())
						? (EntityThreadID {})
						: (hash(thread_name).value())
					;

					auto entity_target = EntityTarget::parse(entity_ref);

					const auto is_remote_variable = ((entity_target) && (!entity_target->is_self_targeted()));

					if (!entity_target)
					{
						// Default to 'self' target.
						entity_target = EntityTarget {};
					}

					// NOTE: May change this logic later. (see also: `EntityThreadBuilder::process_remote_variable_assignment`)
					auto variable_scope = (thread_id || entity_target->is_self_targeted()) // (is_remote_variable)
						? MetaVariableScope::Local
						: MetaVariableScope::Global
					;

					auto resolved_variable_id = MetaSymbolID {};

					if (!is_remote_variable && !thread_id && (variable_scope == MetaVariableScope::Local))
					{
						auto opt_variable_context = opt_parsing_context.get_variable_context();

						if (!opt_variable_context)
						{
							break;
						}

						const auto variable_entry = opt_variable_context->retrieve_variable(variable_name);

						if (!variable_entry)
						{
							break;
						}

						variable_scope = variable_entry->scope;
						resolved_variable_id = variable_entry->resolved_name;
					}
					else // if (!resolved_variable_id)
					{
						resolved_variable_id = MetaVariableContext::resolve_path
						(
							static_cast<MetaSymbolID>(thread_id),

							variable_name,
							variable_scope
						);
					}

					expr_type = resolve<OnThreadVariableUpdate>();

					if (expr_type)
					{
						if (!active_type)
						{
							active_type = expr_type;
						}
					}
					else
					{
						break;
					}

					const auto expr_type_id = expr_type.id();

					auto& shared_storage = descriptor.get_shared_storage();

					constexpr MetaSymbolID entity_field_id = "entity"_hs;

					assert(expr_type.data(entity_field_id));

					handle_condition
					(
						active_combinator,

						EventTriggerSingleCondition
						{
							entity_field_id,

							allocate_meta_any(*entity_target, &shared_storage), // std::move(*entity_target)

							(variable_scope == MetaVariableScope::Local)
								? EventTriggerComparisonMethod::Equal
								: EventTriggerComparisonMethod::NotEqual
							,

							expr_type_id
						}
					);

					constexpr MetaSymbolID variable_scope_field_id = "variable_scope"_hs;

					assert(expr_type.data(variable_scope_field_id));

					handle_condition
					(
						Combinator::And,

						EventTriggerSingleCondition
						{
							variable_scope_field_id,
								
							allocate_meta_any(MetaVariableScope::Local, &shared_storage),

							(variable_scope == MetaVariableScope::Local)
								? EventTriggerComparisonMethod::Equal
								: EventTriggerComparisonMethod::NotEqual
							,

							expr_type_id
						}
					);

					constexpr MetaSymbolID resolved_variable_name_field_id = "resolved_variable_name"_hs;

					assert(expr_type.data(resolved_variable_name_field_id));

					handle_condition
					(
						Combinator::And,

						EventTriggerSingleCondition
						{
							resolved_variable_name_field_id,
								
							allocate_meta_any(resolved_variable_id, &shared_storage),

							EventTriggerComparisonMethod::Equal,

							expr_type_id
						}
					);

					auto comparison_value = MetaAny {};
					auto comparison_method = EventTriggerConditionType::ComparisonMethod::Equal;

					if (compared_value_raw.empty())
					{
						if (encode_default_variable_comparison_as_null_check)
						{
							comparison_value = MetaAny { Entity { engine::null } };
							comparison_method = EventTriggerConditionType::get_comparison_method("!=", invert_condition);
						}
						else
						{
							comparison_value = MetaAny { true };

							// This behavior may change in the future.
							comparison_method = EventTriggerConditionType::get_comparison_method(">=", invert_condition); // EventTriggerConditionType::ComparisonMethod::GreaterThanOrEqual

							// Alternative:
							//comparison_method = EventTriggerConditionType::get_comparison_method("==", invert_condition); // EventTriggerConditionType::ComparisonMethod::Equal
						}
					}
					else
					{
						comparison_value = process_trigger_condition_value(descriptor, compared_value_raw, opt_parsing_context);
						comparison_method = EventTriggerConditionType::get_comparison_method(comparison_operator, invert_condition);
					}

					if (comparison_value)
					{
						constexpr MetaSymbolID variable_update_result_field_id = "variable_update_result"_hs;

						assert(expr_type.data(variable_update_result_field_id));

						handle_condition
						(
							Combinator::And,

							EventTriggerSingleCondition
							{
								variable_update_result_field_id,
								
								// NOTE: This is an intentional copy of `comparison_value`, see below for secondary usage.
								MetaAny { comparison_value }, // std::move(comparison_value) // comparison_value.as_ref()

								comparison_method,

								expr_type_id
							}
						);

						const auto value_operator = EventTriggerConditionType::operation_from_comparison_method(comparison_method);

						auto operation_out = MetaValueOperation {};

						// if (value_has_indirection(comparison_value)) ...

						if (is_remote_variable || thread_id)
						{
							operation_out.segments.emplace_back
							(
								allocate_meta_any
								(
									IndirectMetaVariableTarget
									{
										*entity_target,

										MetaVariableTarget
										{
											resolved_variable_id,
											variable_scope
										},
										
										EntityThreadTarget { thread_id }
									},

									&shared_storage
								),

								value_operator
							);
						}
						else
						{
							operation_out.segments.emplace_back
							(
								allocate_meta_any
								(
									MetaVariableTarget
									{
										resolved_variable_id,
										variable_scope
									},

									&shared_storage
								),

								value_operator
							);
						}

						operation_out.segments.emplace_back
						(
							std::move(comparison_value), // MetaAny { comparison_value }, // comparison_value.as_ref()
							value_operator
						);

						//store_condition();

						handle_condition
						(
							Combinator::Or,

							EventTriggerSingleCondition
							{
								allocate_meta_any
								(
									std::move(operation_out),
									&shared_storage
								)
							}
						);

						/*
						// Alternative implementation:
						if (!is_remote_variable)
						{
							const auto comparison_expr = std::string_view
							{
								variable_name.data(),
								static_cast<std::size_t>((compared_value_raw.data() + compared_value_raw.length()) - variable_name.data())
							};

							handle_condition
							(
								Combinator::Or,

								EventTriggerSingleCondition
								{
									process_trigger_condition_value(descriptor, comparison_expr, opt_parsing_context)
								}
							);
						}
						*/
					}
				}
			}

			is_first_expr = false;

			// Debugging related:
			/*
			auto remaining_length = (trigger_condition_expr.size() - static_cast<std::size_t>(updated_offset));
			auto remaining = std::string_view { (trigger_condition_expr.data() + updated_offset), remaining_length };

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
			callback(active_type.id(), std::move(*condition_out));
		}
		else
		{
			callback(active_type.id(), std::nullopt);
		}
	}
	
	// Processes an 'all-in-one' (unified) condition-block from `trigger_condition_expr`.
	// This condition cannot be used in a traditional rule/trigger scenario, since it could encode multiple primary types.
	inline std::optional<EventTriggerCondition> process_unified_condition_block(EntityDescriptor& descriptor, std::string_view trigger_condition_expr, const MetaParsingContext& opt_parsing_context={}, bool encode_default_variable_comparison_as_null_check=false)
	{
		using Combinator = EventTriggerCompoundMethod;

		std::optional<EventTriggerCondition> condition_out = std::nullopt;

		process_trigger_expression
		(
			descriptor,

			trigger_condition_expr,

			[&descriptor, &condition_out](MetaTypeID type_id, std::optional<EventTriggerCondition> condition)
			{
				if (!condition.has_value())
				{
					return;
				}

				if (condition_out.has_value())
				{
					concatenate_conditions(descriptor, *condition_out, std::move(*condition), Combinator::Or);
				}
				else
				{
					condition_out = std::move(condition);
				}
			},

			// Always embed type information. (Required for `EntityThread` runtime)
			true,

			opt_parsing_context,

			encode_default_variable_comparison_as_null_check
		);

		return condition_out;
	}

	/*
		Processes multiple threads from `content`, calling `thread_range_callback` for the top-level
		thread range(s) generated, and `existing_thread_callback` for any threads found that have already been defined.

		Both `thread_range_callback` and `existing_thread_callback` take an `EntityThreadIndex`
		as the first parameter, and an `EntityThreadCount` as the second.

		NOTE: For best interoperability, `opt_parsing_context->variable_context` should not store a name.
		(This could lead to variable name conflicts between threads)
	*/
	template <typename ContentType, typename ThreadRangeCallback, typename ExistingThreadCallback>
	EntityThreadCount process_thread_list
	(
		EntityDescriptor& descriptor,
		const ContentType& content,
		
		ThreadRangeCallback&& thread_range_callback,
		ExistingThreadCallback&& existing_thread_callback,

		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr,

		bool parse_content_as_script_reference=false
	)
	{
		using ContentValueType = std::remove_cvref_t<ContentType>;

		constexpr bool base_content_is_json = std::is_same_v<ContentValueType, util::json>;

		const auto initial_thread_index = descriptor.get_next_thread_index();

		auto shared_thread_context = MetaVariableContext { opt_parsing_context };

		auto allocate_top_level_threads = [&descriptor](std::size_t threads_to_allocate) -> std::size_t
		{
			if (!threads_to_allocate)
			{
				return {};
			}

			const auto& thread_storage = descriptor.shared_storage.get_storage<EntityThreadDescription>();
			const auto initial_thread_count = thread_storage.data().size();

			// TODO: Implement bulk allocation interface for `shared_storage`.
			for (std::size_t i = 0; i < threads_to_allocate; i++)
			{
				descriptor.shared_storage.allocate<EntityThreadDescription>();
			}

			assert(thread_storage.data().size() == (initial_thread_count + threads_to_allocate));

			return threads_to_allocate;
		};

		auto deallocate_top_level_thread = [&descriptor](EntityThreadIndex thread_index) -> std::size_t
		{
			const auto& thread_storage = descriptor.shared_storage.get_storage<EntityThreadDescription>();
			const auto initial_thread_count = thread_storage.data().size();
			const auto threads_to_deallocate = static_cast<decltype(initial_thread_count)>(1);

			// TODO: Implement bulk deallocation interface for `shared_storage`.
			descriptor.shared_storage.deallocate<EntityThreadDescription>(thread_index);

			assert(thread_storage.data().size() == (initial_thread_count - threads_to_deallocate));

			return threads_to_deallocate;
		};

		// NOTE: We determine the number of threads to allocate ahead-of-time
		// to avoid conflicts with embedded thread allocations. (e.g. sub-threads)
		EntityThreadCount top_level_threads_to_allocate = 0; // static_cast<EntityThreadCount>(content.size());

		auto handle_existing_thread_path_impl = [&descriptor, opt_factory_context, opt_base_path](const auto& thread_path, auto&& callback) -> bool
		{
			if (thread_path.empty())
			{
				return true;
			}

			const auto thread_name = EntityThreadBuilder::thread_name_from_script_reference(thread_path, opt_factory_context, opt_base_path);
			const auto thread_id = hash(thread_name).value();

			if (auto thread_index = descriptor.get_thread_index(thread_id))
			{
				callback(*thread_index);

				return true;
			}

			return false;
		};

		auto handle_existing_thread_path = [&handle_existing_thread_path_impl](const auto& thread_path)
		{
			return handle_existing_thread_path_impl(thread_path, [](auto&&...){});
		};

		auto on_top_level_thread_path = [&handle_existing_thread_path_impl, &existing_thread_callback, &top_level_threads_to_allocate](const auto& thread_path) -> bool
		{
			const bool thread_handled = handle_existing_thread_path_impl
			(
				thread_path,

				[&existing_thread_callback](const auto& thread_index)
				{
					existing_thread_callback(thread_index, 1);
				}
			);

			if (!thread_handled)
			{
				top_level_threads_to_allocate++;

				return false;
			}

			return true;
		};

		// Ensure we don't allocate space for threads we're not going to process
		// (e.g. already processed threads):
		if constexpr (base_content_is_json)
		{
			switch (content.type())
			{
				case util::json::value_t::array:
				{
					util::json_for_each
					(
						content,

						[&](const util::json& thread_content)
						{
							if (thread_content.empty() || !thread_content.is_string())
							{
								// Make sure to allocate a top-level thread, even if the content is empty.
								top_level_threads_to_allocate++;
							}
							else
							{
								const auto& thread_path = thread_content.get<std::string>();

								on_top_level_thread_path(thread_path);
							}
						}
					);

					break;
				}

				case util::json::value_t::object:
				{
					for (const auto& proxy : content.items())
					{
						const auto& thread_content = proxy.value();

						if (!thread_content.empty())
						{
							const auto& thread_name = proxy.key();
							const auto thread_id = hash(thread_name).value();

							if (auto thread_index = descriptor.get_thread_index(thread_id))
							{
								existing_thread_callback(*thread_index, 1);
							}
							else
							{
								top_level_threads_to_allocate++;
							}
						}
					}

					break;
				}

				default:
				{
					if (!content.empty())
					{
						if (content.is_string())
						{
							const auto& thread_path = content.get<std::string>();

							if (on_top_level_thread_path(thread_path))
							{
								break;
							}
						}

						top_level_threads_to_allocate++;
					}

					break;
				}
			}
		}
		else
		{
			on_top_level_thread_path(content);
		}
		
		const auto top_level_threads_allocated = allocate_top_level_threads(top_level_threads_to_allocate);

		assert(top_level_threads_allocated == top_level_threads_to_allocate);

		if (!top_level_threads_allocated)
		{
			return 0;
		}

		const auto top_level_thread_max_index_allowed = (static_cast<EntityThreadIndex>(initial_thread_index) + static_cast<EntityThreadIndex>(top_level_threads_allocated));

		EntityThreadCount top_level_threads_processed = 0;

		auto process_thread = [&descriptor, opt_factory_context, opt_base_path, &opt_parsing_context, &shared_thread_context, initial_thread_index, top_level_thread_max_index_allowed, parse_content_as_script_reference, &handle_existing_thread_path, &top_level_threads_processed]
		(
			const auto& content,
			const auto& opt_thread_name
		) -> bool
		{
			using content_t = decltype(content);
			using content_value_t = std::remove_cvref_t<content_t>;

			constexpr bool content_is_json = std::is_same_v<content_value_t, util::json>;

			constexpr bool content_is_string =
			(
				(std::is_same_v<content_value_t, std::string>)
				||
				(std::is_same_v<content_value_t, std::string_view>)
			);

			if (content.empty())
			{
				return false;
			}

			if (opt_thread_name.empty())
			{
				if constexpr (content_is_json)
				{
					if (content.is_string())
					{
						const auto& thread_path = content.get<std::string>();

						if (handle_existing_thread_path(thread_path))
						{
							return false;
						}
					}
				}
				else
				{
					if constexpr (content_is_string)
					{
						const auto& thread_path = content;

						if (handle_existing_thread_path(thread_path))
						{
							return false;
						}
					}
				}
			}
			else
			{
				const auto thread_id = hash(opt_thread_name).value();

				if (descriptor.has_thread(thread_id))
				{
					return false;
				}
			}

			auto thread_variable_context = MetaVariableContext
			{
				&shared_thread_context,

				(opt_thread_name.empty())
					? static_cast<MetaSymbolID>(descriptor.get_threads().size()) // MetaSymbolID {}
					: hash(opt_thread_name).value()
			};

			const auto top_level_thread_index = static_cast<EntityThreadIndex>((initial_thread_index + top_level_threads_processed));

			assert(top_level_thread_index <= top_level_thread_max_index_allowed);

			//auto& thread_description = descriptor.shared_storage.get<EntityThreadDescription>(top_level_thread_index);

			auto thread_builder = EntityThreadBuilder
			{
				descriptor,
				
				EntityThreadBuilder::ThreadDescriptor
				(
					top_level_thread_index // descriptor, thread_description
				),

				opt_thread_name,

				opt_factory_context,
				opt_base_path,
				
				MetaParsingContext
				{
					opt_parsing_context.get_type_context(),
					&thread_variable_context
				}
			};

			if (parse_content_as_script_reference)
			{
				if constexpr (content_is_json)
				{
					if (content.is_string())
					{
						const auto& thread_path = content.get<std::string>();

						if (!thread_path.empty())
						{
							if (thread_builder.process_from_file(thread_path))
							{
								top_level_threads_processed++;

								return true;
							}
						}
					}
				}
				else
				{
					constexpr bool content_is_filepath = (std::is_same_v<content_value_t, std::filesystem::path>);

					if constexpr ((content_is_string) || (content_is_filepath))
					{
						if (thread_builder.process_from_file(content))
						{
							top_level_threads_processed++;

							return true;
						}
					}
				}
			}
			else
			{
				if (thread_builder.process(content))
				{
					top_level_threads_processed++;

					return true;
				}
			}

			return false;
		};

		if constexpr (base_content_is_json)
		{
			switch (content.type())
			{
				case util::json::value_t::array:
				{
					util::json_for_each
					(
						content,

						[&process_thread](const util::json& thread_content)
						{
							process_thread(thread_content, std::string_view {});
						}
					);

					break;
				}

				case util::json::value_t::object:
				{
					for (const auto& proxy : content.items())
					{
						const auto& thread_name = proxy.key();
						const auto& thread_content = proxy.value();

						process_thread(thread_content, thread_name);
					}

					break;
				}

				default:
				{
					process_thread(content, std::string_view {});

					break;
				}
			}
		}
		else
		{
			constexpr bool base_content_is_string =
			(
				(std::is_same_v<ContentValueType, std::string>)
				||
				(std::is_same_v<ContentValueType, std::string_view>)
			);

			if constexpr (base_content_is_string)
			{
				if (parse_content_as_script_reference)
				{
					const auto& thread_name = content;

					process_thread(content, thread_name);
				}
				else
				{
					process_thread(content, std::string_view {});
				}
			}
			else
			{
				process_thread(content, std::string_view {});
			}
		}

		// NOTE: The total processed may be greater than `top_level_threads_processed`, since this includes sub-threads.
		const auto total_processed_count = static_cast<EntityThreadCount>(descriptor.get_next_thread_index() - initial_thread_index);

		if (parse_content_as_script_reference)
		{
			if ((total_processed_count == 1) && (top_level_threads_processed == 0) && (top_level_threads_allocated == 1))
			{
				deallocate_top_level_thread(initial_thread_index);

				return {};
			}
		}
		else
		{
			assert(top_level_threads_processed == top_level_threads_allocated);
			assert(total_processed_count >= top_level_threads_processed);
		}

		thread_range_callback(initial_thread_index, top_level_threads_processed);

		return total_processed_count;
	}

	// This overload uses `thread_range_callback` for both existing and new thread ranges.
	// 
	// For more details, please see the primary overload.
	template <typename ThreadRangeCallback>
	EntityThreadCount process_thread_list
	(
		EntityDescriptor& descriptor,
		const util::json& content,

		ThreadRangeCallback&& thread_range_callback,
		
		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	)
	{
		return process_thread_list
		(
			descriptor,
			content,
			thread_range_callback,
			thread_range_callback,
			opt_base_path,
			opt_parsing_context,
			opt_factory_context
		);
	}

	template <typename ThreadRangeCallback>
	EntityThreadCount process_default_threads
	(
		EntityDescriptor& descriptor,
		
		const auto& name_as_content,

		ThreadRangeCallback&& thread_range_callback,
		
		const std::filesystem::path* opt_base_path=nullptr,
		const MetaParsingContext& opt_parsing_context={},
		const EntityFactoryContext* opt_factory_context=nullptr
	)
	{
		return process_thread_list
		(
			descriptor,
			
			name_as_content,
			
			thread_range_callback,
			thread_range_callback,
			opt_base_path,
			opt_parsing_context,
			opt_factory_context,

			// Handle `name_as_content` as a reference to a script.
			true
		);
	}
}