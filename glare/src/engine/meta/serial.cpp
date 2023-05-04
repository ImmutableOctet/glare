#include "serial.hpp"

#include "hash.hpp"
#include "function.hpp"
#include "indirection.hpp"
#include "data_member.hpp"
#include "cast.hpp"

#include "meta_type_descriptor.hpp"
#include "meta_type_resolution_context.hpp"
#include "meta_value_operation.hpp"
#include "meta_value_operator.hpp"
#include "meta_function_call.hpp"
#include "meta_type_reference.hpp"
#include "meta_type_conversion.hpp"
#include "meta_parsing_context.hpp"
#include "meta_variable_context.hpp"
#include "meta_variable_target.hpp"
#include "meta_property.hpp"

#include "indirect_meta_any.hpp"
#include "indirect_meta_variable_target.hpp"

#include "shared_storage_interface.hpp"

#include <engine/types.hpp>

#include <engine/entity/entity_thread_target.hpp>
#include <engine/entity/entity_target.hpp>
#include <engine/entity/parse.hpp>

#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/format.hpp>

#include <tuple>
#include <vector>
#include <optional>
#include <type_traits>
#include <algorithm>
#include <iterator>
#include <cctype>

// Debugging related:
#include <util/log.hpp>

#include <regex>

namespace engine
{
	namespace impl
	{
		static std::size_t resolve_meta_any_sequence_container_impl
		(
			const util::json& content,
			MetaAny& instance,
			const MetaParsingInstructions& parse_instructions
		)
		{
			if (!instance)
			{
				return 0;
			}

			assert(instance.type().is_sequence_container());

			auto sequence_container = instance.as_sequence_container();

			if (!sequence_container)
			{
				return 0;
			}

			const auto element_type = sequence_container.value_type();

			if (!element_type)
			{
				return 0;
			}

			std::size_t elements_processed = 0;

			util::json_for_each
			(
				content,

				[&parse_instructions, &element_type, &sequence_container, &elements_processed](const util::json& entry)
				{
					if (auto element = resolve_meta_any(entry, element_type, parse_instructions))
					{
						if (auto resolved = try_get_underlying_value(element)) // , args...
						{
							element = std::move(resolved);
						}

						if (element.allow_cast(element_type))
						{
							if (sequence_container.insert(sequence_container.end(), std::move(element)))
							{
								elements_processed++;
							}
						}
					}
				}
			);

			return elements_processed;
		}

		static MetaAny resolve_meta_any_sequence_container_impl
		(
			const util::json& content,
			MetaType type,
			const MetaParsingInstructions& parse_instructions
		)
		{
			if (!type)
			{
				return {};
			}

			if (!type.is_sequence_container())
			{
				return {};
			}

			auto container_out = type.construct();

			if (!container_out)
			{
				return {};
			}

			if (resolve_meta_any_sequence_container_impl(content, container_out, parse_instructions))
			{
				return container_out;
			}

			return {};
		}

		static std::size_t resolve_meta_any_associative_container_impl
		(
			const util::json& content,
			MetaAny& instance,
			const MetaParsingInstructions& parse_instructions
		)
		{
			if (!instance)
			{
				return 0;
			}

			assert(instance.type().is_associative_container());

			auto associative_container = instance.as_associative_container();

			if (!associative_container)
			{
				return 0;
			}

			const auto key_type = associative_container.key_type();

			if (!key_type)
			{
				return 0;
			}

			const auto intended_value_type = associative_container.mapped_type(); // value_type();

			if (!intended_value_type)
			{
				return 0;
			}

			// TODO: Look into possible performance benefits of manually
			// checking against `intended_type` prior to resolution and cast steps.
			auto try_get_instance = [](MetaAny instance, const MetaType& intended_type) -> MetaAny
			{
				if (!instance)
				{
					return {};
				}

				if (auto resolved = try_get_underlying_value(instance)) // , args...
				{
					instance = std::move(resolved);
				}

				if (try_direct_cast(instance, intended_type, true))
				{
					return instance;
				}

				return {};
			};

			auto resolve_key = [&parse_instructions, &key_type, &try_get_instance](auto&& key_name) -> MetaAny
			{
				if (auto key_instance = meta_any_from_string(key_name, parse_instructions, key_type))
				{
					return try_get_instance(std::move(key_instance), key_type);
				}

				return std::nullopt;
			};

			auto resolve_value = [&parse_instructions, &try_get_instance](auto&& value_content, const MetaType& value_type) -> MetaAny
			{
				if (auto value_instance = resolve_meta_any(value_content, value_type, parse_instructions))
				{
					return try_get_instance(std::move(value_instance), value_type);
				}

				return std::nullopt;
			};

			std::size_t elements_processed = 0;

			for (const auto& entry : content.items())
			{
				const auto& key_raw = entry.key();

				auto [key_name, value_type_spec, _unused_trailing, _unused_expr_syntax] = util::parse_key_expr_and_value_type
				(
					std::string_view { key_raw },
					":", false
				);

				if (key_name.empty())
				{
					continue;
				}

				auto value_type = intended_value_type;

				if (!value_type_spec.empty())
				{
					const auto opt_type_context = parse_instructions.context.get_type_context();

					const auto manual_value_type = (opt_type_context)
						? opt_type_context->get_type(value_type_spec, parse_instructions)
						: resolve(hash(value_type_spec).value())
					;

					if (manual_value_type)
					{
						value_type = manual_value_type;
					}
				}

				if (auto key_result = resolve_key(key_name))
				{
					const auto& value_content = entry.value();

					if (auto value_out = resolve_value(value_content, value_type))
					{
						if (associative_container.insert(std::move(key_result), std::move(value_out)))
						{
							elements_processed++;
						}
					}
				}
			}

			return elements_processed;
		}

		static MetaAny resolve_meta_any_associative_container_impl
		(
			const util::json& content,
			MetaType type,
			const MetaParsingInstructions& parse_instructions
		)
		{
			if (!type)
			{
				return {};
			}

			if (!type.is_associative_container())
			{
				return {};
			}

			auto container_out = type.construct();

			if (!container_out)
			{
				return {};
			}

			if (resolve_meta_any_associative_container_impl(content, container_out, parse_instructions))
			{
				return container_out;
			}

			return {};
		}
	};

	static MetaAny meta_any_from_string_to_string_impl(std::string_view value, bool strip_quotes=true, MetaParsingStorage* storage=nullptr)
	{
		if (strip_quotes)
		{
			value = util::unquote_safe(value);
		}

		/*
		// TODO: Revisit EnTT's native casting functionality for string conversions.
		auto as_str = MetaAny { std::string(value) };

		if (type)
		{
			if (MetaAny result = as_str.allow_cast(type))
			{
				return result;
			}
		}

		return as_str;
		*/

		return allocate_meta_any(std::string(value), storage);
	}

	static MetaAny meta_any_from_string_fallback_to_string_impl(std::string_view value, const MetaParsingInstructions& instructions, MetaType type={}, bool arithmetic_type_check=true)
	{
		// If we reach this point and `type` is a known arithmetic type,
		// we've failed processing, since a string shouldn't be allowed here.
		// (See `meta_any_from_string_resolve_expression_impl` for resolution of arithmetic and boolean values)
		// 
		// NOTE: We explicitly check `is_arithmetic` here, since `type`
		// could still be converted from a string at runtime. (See below)
		if (arithmetic_type_check && type && type.is_arithmetic())
		{
			return {};
		}

		return meta_any_from_string_to_string_impl(util::trim(value), instructions.strip_quotes, instructions.storage);
	}
	
	static MetaAny meta_any_from_string_arithmetic_impl(std::string_view value, const MetaParsingInstructions& instructions, MetaType type={})
	{
		if (auto non_numeric = value.find_first_not_of("0123456789. \t\n"); non_numeric != std::string_view::npos)
		{
			return {};
		}

		if (!value.contains('.'))
		{
			if (!type || type.is_integral())
			{
				const auto type_id = (type)
					? type.id()
					: MetaTypeID {}
				;

				// TODO: Look into support for smaller integer types.
				switch (type_id)
				{
					case entt::type_hash<std::int64_t>::value():
						if (auto as_integer = util::from_string<std::int64_t>(value))
						{
							return *as_integer;
						}

						break;
					case entt::type_hash<std::uint64_t>::value():
						if (auto as_integer = util::from_string<std::uint64_t>(value))
						{
							return *as_integer;
						}

						break;
					default:
						if (!type || type.is_signed())
						{
							if (auto as_integer = util::from_string<std::int32_t>(value))
							{
								return *as_integer;
							}

							break;
						}
						else
						{
							if (auto as_integer = util::from_string<std::uint32_t>(value))
							{
								return *as_integer;
							}

							break;
						}
				}
			}
			else
			{
				if (auto as_integer = util::from_string<std::int32_t>(value)) // std::int64_t
				{
					if (auto construct_from = type.construct(*as_integer))
					{
						return construct_from;
					}

					auto as_any = MetaAny { *as_integer };

					if (as_any.allow_cast(type))
					{
						return as_any;
					}
				}
			}
		}

		if (auto as_float = util::from_string<float>(value)) // double
		{
			if (!type || (type.is_arithmetic() && !type.is_integral()))
			{
				return *as_float;
			}
			else
			{
				if (auto construct_from = type.construct(*as_float))
				{
					return construct_from;
				}

				auto as_any = MetaAny { *as_float };

				if (as_any.allow_cast(type))
				{
					return as_any;
				}
			}
		}

		return {};
	}

	static MetaAny meta_any_from_string_boolean_impl(std::string_view value, const MetaParsingInstructions& instructions, MetaType type={})
	{
		using namespace engine::literals;

		switch (hash(value).value())
		{
			case "true"_hs:
				if (!type || (type.id() == entt::type_hash<bool>::value()))
				{
					return true;
				}
				else
				{
					if (auto construct_from = type.construct(true))
					{
						return construct_from;
					}

					auto as_any = MetaAny { true };

					if (as_any.allow_cast(type))
					{
						return as_any;
					}
				}

				break;

			case "false"_hs:
				if (!type || (type.id() == entt::type_hash<bool>::value()))
				{
					return false;
				}
				else
				{
					if (auto construct_from = type.construct(false))
					{
						return construct_from;
					}

					auto as_any = MetaAny { false };

					if (as_any.allow_cast(type))
					{
						return as_any;
					}
				}

				break;
		}

		return {};
	}

	static std::tuple<MetaAny, bool> meta_any_from_string_execute_string_command(const auto command_id, const auto& content, std::string_view content_raw={}, bool allow_entity_indirection=true, const MetaParsingContext& context={}, MetaParsingStorage* storage = nullptr)
	{
		using namespace engine::literals;

		MetaAny result;

		bool command_found = true;

		switch (command_id)
		{
			case "id"_hs:
			case "Id"_hs:
			case "ID"_hs:
			case "name"_hs:
			case "hash"_hs:
				if (!content.empty())
				{
					result = allocate_meta_any(hash(content).value(), storage);
				}

				break;

			case "thread"_hs:
				if (!content.empty())
				{
					result = allocate_meta_any(EntityThreadTarget(content), storage);
				}

				break;

			case "type"_hs:
			case "component"_hs:
				if (!content.empty())
				{
					auto type = MetaType {};

					if (const auto opt_type_context = context.get_type_context())
					{
						type = opt_type_context->get_type(content, true, false, false);
					}
					else
					{
						const auto type_id = hash(content).value();

						type = resolve(type_id);
					}

					if (type)
					{
						result = allocate_meta_any(MetaTypeReference { type.id() }, storage);
					}
				}

				break;

			default:
				command_found = false;

				break;
		}

		return { std::move(result), command_found };
	}

	static std::tuple<MetaAny, bool> meta_any_from_string_execute_any_command(const auto command_id, const MetaAny& content, std::string_view content_raw={}, bool allow_entity_indirection=true, const MetaParsingContext& context={}, MetaParsingStorage* storage = nullptr)
	{
		using namespace engine::literals;

		auto try_value = [command_id, context, storage, &content_raw, allow_entity_indirection](auto&& value) -> std::optional<std::tuple<MetaAny, bool>>
		{
			//switch (command_id)
			{
				//case "..."_hs:
				// break;

				//default:
				{
					if (const auto* as_str_view = value.try_cast<std::string_view>())
					{
						return meta_any_from_string_execute_string_command(command_id, *as_str_view, content_raw, allow_entity_indirection, context, storage);
					}
					else if (const auto* as_str = value.try_cast<std::string>())
					{
						return meta_any_from_string_execute_string_command(command_id, *as_str, content_raw, allow_entity_indirection, context, storage);
					}

					//break;
				}
			}

			return std::nullopt;
		};

		if (auto underlying = try_get_underlying_value(content))
		{
			if (auto result = try_value(underlying))
			{
				return *result;
			}
		}
		
		if (auto result = try_value(content))
		{
			return *result;
		}

		return meta_any_from_string_execute_string_command(command_id, std::string_view {}, std::string_view {}, allow_entity_indirection, context, storage);
	}

	// Primary routine for resolving a `MetaAny` string expression.
	static std::size_t meta_any_from_string_resolve_expression_impl
	(
		MetaValueOperation& output,
		std::string_view value,
		const MetaParsingInstructions& instructions,
		MetaType type={},
		bool allow_entity_fallback=true,
		bool allow_component_fallback=true,
		bool allow_standalone_opaque_function=true,
		bool allow_remote_variables=true
	)
	{
		using namespace engine::literals;

		std::size_t values_processed = 0;

		const auto initial_type = type;
		const auto had_initial_type = static_cast<bool>(initial_type); // type

		auto clear = [&values_processed, &output]() -> bool
		{
			values_processed = 0;

			output.segments.clear();

			return true;
		};

		auto enqueue_raw = [&type, &output, &values_processed]
		(MetaAny&& instance, bool update_type=true, MetaValueOperator operation=MetaValueOperator::Get) -> bool
		{
			if (!instance)
			{
				return false;
			}

			if (update_type)
			{
				if (auto type_out = instance.type())
				{
					if (!value_has_indirection(instance))
					{
						type = type_out;
					}
				}
			}

			const bool apply_to_beginning = (operation == MetaValueOperator::Get);

			if (apply_to_beginning)
			{
				output.segments.insert
				(
					output.segments.begin(),

					MetaValueOperation::Segment
					{
						std::move(instance),
						operation
					}
				);
			}
			else
			{
				output.segments.emplace_back
				(
					std::move(instance),
					operation
				);
			}

			values_processed++;

			return true;
		};

		auto enqueue = [&instructions, &enqueue_raw, &type]
		(auto&& instance, bool update_type=true, MetaValueOperator operation=MetaValueOperator::Get) -> bool
		{
			using instance_t = std::decay_t<decltype(instance)>;

			if constexpr (std::is_same_v<instance_t, MetaAny> || has_method_get_type_v<instance_t, MetaType> || has_method_get_type_v<instance_t, MetaTypeID>)
			{
				if (update_type)
				{
					if (auto underlying_type = try_get_underlying_type(instance))
					{
						type = underlying_type;
					}
				}

				update_type = false;
			}

			if constexpr (!std::is_same_v<instance_t, MetaAny>)
			{
				if (instructions.storage)
				{
					auto instance_meta_type = resolve<instance_t>();

					if (instance_meta_type)
					{
						if (update_type)
						{
							if (!type_has_indirection(instance_meta_type))
							{
								type = instance_meta_type;
							}

							update_type = false;
						}

						// NOTE: We forward `instance` as an lvalue-reference here to ensure proper move semantics.
						if (auto remote_index = instructions.storage->allocate(entt::forward_as_meta(instance)))
						{
							return enqueue_raw
							(
								MetaAny
								{
									IndirectMetaAny
									{
										instance_meta_type.id(),
										*remote_index,
										instructions.storage->get_checksum()
									}
								},

								false,
								operation
							);
						}
					}
				}
			}

			return enqueue_raw(std::forward<decltype(instance)>(instance), update_type, operation);
		};

		auto replace = [&enqueue, &clear](auto&& instance) -> bool
		{
			clear();

			return enqueue(std::forward<decltype(instance)>(instance));
		};

		auto construct_type = [&instructions, &type, &enqueue, &replace](std::string_view content, bool allow_indirection=true) -> bool
		{
			auto type_desc = MetaTypeDescriptor(type);

			if (type_desc.set_variables(content, instructions))
			{
				const bool has_indirection = type_desc.has_indirection(false);

				if (has_indirection)
				{
					if (!allow_indirection)
					{
						return false;
					}
				}
				else
				{
					if (auto instance = type_desc.instance())
					{
						return replace(std::move(instance));
					}
				}

				return enqueue(std::move(type_desc));
			}

			return false;
		};

		auto resolve_property = [](const MetaType& type, std::string_view symbol, MetaSymbolID symbol_id={}) -> std::optional<MetaProperty>
		{
			const auto [getter_id, setter_id] = MetaProperty::generate_accessor_identifiers(symbol);

			const auto getter_fn = type.func(getter_id);
			const auto setter_fn = type.func(setter_id);
			
			const auto data_member = resolve_data_member_by_id(type, true, symbol_id);

			if (getter_fn || setter_fn || data_member)
			{
				return MetaProperty
				{
					type.id(),
									
					((getter_fn) ? getter_id : MetaFunctionID {}), // getter_id
					((setter_fn) ? setter_id : MetaFunctionID {}), // setter_id

					((data_member) ? symbol_id : MetaSymbolID {})
				};
			}

			return std::nullopt;
		};

		auto make_opaque_property = [](std::string_view symbol, MetaSymbolID data_member_id={}, MetaTypeID type_id={})
		{
			const auto [getter_id, setter_id] = MetaProperty::generate_accessor_identifiers(symbol);

			return MetaProperty
			{
				type_id,

				getter_id,
				setter_id,

				data_member_id
			};
		};

		util::split
		(
			value,

			std::array
			{
				std::string_view { "::" },
				std::string_view { "."  },
				std::string_view { "->" }
			},

			[
				&instructions, allow_entity_fallback, allow_component_fallback, allow_standalone_opaque_function, &type, &value,
				&initial_type, had_initial_type, &enqueue, &replace, &construct_type, &resolve_property, &make_opaque_property, &output
			]
			(std::string_view& symbol, bool is_first_symbol, bool is_last_symbol) -> bool
			{
				//constexpr std::string_view call_begin_symbol = "(";
				//constexpr std::string_view call_end_symbol = ")";

				constexpr auto call_begin_symbol = '(';
				constexpr auto call_end_symbol = ')';

				constexpr std::string_view subscript_begin_symbol = "[";
				constexpr std::string_view subscript_end_symbol = "]";

				// This is a workaround for symbol collisions found within substrings.
				// (e.g. `.` symbols found inside of function calls):
				if (auto call_syntax_begin = symbol.find(call_begin_symbol); call_syntax_begin != std::string_view::npos) // && !symbol.ends_with(')')
				{
					// TODO: Determine if implementing 'subscript prior to call' makes sense. (e.g. `fn[2]()`)
					// Trailing subscript is covered naturally via this truncation routine. (i.e. remainder of string prior to access operator)

					const auto scope_begin = static_cast<std::size_t>((symbol.data() + call_syntax_begin) - value.data());
					const auto remainder_of_value = value.substr((scope_begin + 1));

					const auto call_syntax_end = util::find_closing_parenthesis(remainder_of_value);

					if (call_syntax_end != std::string_view::npos)
					{
						const auto symbol_begin = static_cast<std::size_t>(symbol.data() - value.data());
						const auto scope_end = static_cast<std::size_t>((remainder_of_value.data() + call_syntax_end) - symbol.data());

						symbol = value.substr(symbol_begin, (scope_end + 1));
					}
				}
				else
				{
					const auto subscript_begin = symbol.find(subscript_begin_symbol);

					if (subscript_begin != std::string_view::npos)
					{
						if (subscript_begin > 0)
						{
							symbol = symbol.substr(0, subscript_begin);
						}
						else
						{
							auto subscript_end = util::find_closing_subscript(symbol, (subscript_begin + 1)); // subscript_begin

							if (subscript_end != std::string_view::npos) // (subscript_end < (symbol.length()-1))
							{
								const auto subscript_value_begin = (subscript_begin + subscript_begin_symbol.length());
								const auto subscript_value_end = subscript_end;

								const auto subscript_value = symbol.substr(subscript_value_begin, (subscript_value_end - subscript_value_begin));

								return enqueue
								(
									meta_any_from_string // meta_any_from_string_compound_expr_impl
									(
										subscript_value,
										instructions
									),
									
									false,

									MetaValueOperator::Subscript
								);
							}
						}
					}
				}

				auto
				[
					command_name_or_value,
					content,
					trailing_expr,
					is_string_content,
					is_command,
					parsed_length
				] = util::parse_command_or_value
				(
					symbol,
					
					true, // allow_trailing_expr
					true, // allow_empty_command_name
					
					false, // truncate_value_at_first_accessor
					false, // truncate_command_name_at_first_accessor
					false, // allow_operator_symbol_in_command_name
					false, // truncate_value_at_operator_symbol

					false  // remove_quotes_in_string_content
				);

				if (command_name_or_value.empty())
				{
					return false;
				}

				if (!is_command)
				{
					symbol = command_name_or_value;
				}

				const auto symbol_id = hash(command_name_or_value).value();

				bool symbol_used_as_type = false;

				auto get_as_type = [&instructions, &value, &command_name_or_value, &symbol_id]() -> MetaType
				{
					const auto current_end_point = static_cast<std::size_t>((command_name_or_value.data() + command_name_or_value.length()) - value.data());
					const auto cumulative = value.substr(0, current_end_point);

					if (const auto opt_type_context = instructions.context.get_type_context())
					{
						if (auto as_type = opt_type_context->get_type(cumulative, instructions))
						{
							return as_type;
						}
						else if (auto as_type = opt_type_context->get_type(command_name_or_value, instructions))
						{
							return as_type;
						}
					}
					else
					{
						const auto cumulative_as_type_id = hash(cumulative).value();

						if (auto as_type = resolve(cumulative_as_type_id))
						{
							return as_type;
						}
						else if (auto as_type = resolve(symbol_id))
						{
							return as_type;
						}
					}

					return {};
				};

				// TODO: Revisit this behavior.
				if (true) // (output.empty() || !type)
				{
					if (!is_last_symbol || is_command)
					{
						if (auto as_type = get_as_type())
						{
							type = as_type;

							symbol_used_as_type = true;
						}
					}
				}

				if (!output.empty() && !symbol_used_as_type)
				{
					const auto& latest_segment = output.segments[0];

					if (latest_segment.value)
					{
						if (!value_has_indirection(latest_segment.value))
						{
							type = latest_segment.value.type();
						}
					}
				}

				if ((is_command) && (symbol_used_as_type || (type && command_name_or_value.empty())))
				{
					// Forward usage of system-types in construction syntax to regular type references.
					// (May remove this later)
					if (type_is_system(type))
					{
						return replace(MetaTypeReference { type.id() });
					}
					else if (instructions.allow_explicit_type_construction)
					{
						if (auto result = construct_type(content))
						{
							return result;
						}
					}
				}

				if (symbol_used_as_type)
				{
					return true;
				}
				
				// NOTE: 'System' references are handled here, as opposed to the initial type-resolution
				// phase to avoid complexity when handling sub-types of 'system' types.
				// 
				// If the system type is used as a standalone reference, it should be caught by
				// the fallback `MetaTypeReference` control-path found below.
				if (instructions.resolve_system_references)
				{
					if ((type) && (type != initial_type) && (output.empty()) && (type_is_system(type)))
					{
						// Reached the first non-type symbol and the type thus far is a 'system' reference.
						// Enqueue the system reference before doing anything else, allowing other segments to reference it.
						enqueue(MetaTypeReference { type.id() });

						// Continue processing as usual.
					}
				}

				if (!symbol.empty())
				{
					if (output.empty())
					{
						if (instructions.allow_entity_indirection)
						{
							if (auto as_entity_target = EntityTarget::parse(symbol, &instructions))
							{
								return replace(std::move(*as_entity_target)); // enqueue
							}
						}

						// TODO: Review whether this condition makes sense to add.
						if (true) // (is_first_symbol && !is_command)
						{
							if (auto opt_variable_context = instructions.context.get_variable_context())
							{
								if (auto variable_entry = opt_variable_context->retrieve_variable(symbol))
								{
									return enqueue(MetaVariableTarget { variable_entry->resolved_name, variable_entry->scope }); // replace
								}
							}
						}

						if (instructions.allow_value_resolution_commands)
						{
							if (is_command)
							{
								// Non-string inputs are disabled for now; seems too costly for false-positives. (Subroutine probably needs to be refactored)
								//if (is_string_content)
								{
									if (auto result = meta_any_from_string_execute_string_command(symbol_id, util::unquote_safe(content), symbol, instructions.allow_entity_indirection, instructions.context, instructions.storage); std::get<0>(result))
									{
										return replace(std::move(std::get<0>(result))); // enqueue
									}
								}
								/*
								else
								{
									// Disabled for now; seems too costly for false-positives.
									if (auto resolved = meta_any_from_string(content, instructions)) // type
									{
										if (auto result = meta_any_from_string_execute_any_command(symbol_id, resolved, symbol, instructions.allow_entity_indirection, instructions.context, instructions.storage); std::get<0>(result))
										{
											return replace(std::move(std::get<0>(result))); // enqueue
										}
									}
								}
								*/
							}
							else
							{
								if (symbol.starts_with('$') || symbol.starts_with('#'))
								{
									const auto symbol_start_index = static_cast<std::size_t>(symbol.data() - value.data());

									// Remove symbol prefix.
									symbol = value.substr(symbol_start_index);

									const auto sub_symbol = util::trim(util::unquote_safe(symbol.substr(1)));

									if (auto result = meta_any_from_string_execute_string_command("hash"_hs, sub_symbol, symbol, instructions.allow_entity_indirection, instructions.context, instructions.storage); std::get<0>(result))
									{
										return replace(std::move(std::get<0>(result))); // enqueue
									}
								}
							}
						}
					}

					if (instructions.allow_function_call_semantics)
					{
						if (is_command)
						{
							if ((type) || ((instructions.allow_opaque_function_references) && (allow_standalone_opaque_function || !output.empty())))
							{
								auto fn = MetaFunction {};

								if (type)
								{
									fn = type.func(symbol_id);
								}

								if ((fn) || (instructions.allow_opaque_function_references)) // ((instructions.allow_opaque_function_references) && (allow_standalone_opaque_function || !output.empty()))
								{
									auto function_out = MetaFunctionCall
									{
										(type)
											? type.id()
											: MetaTypeID {}
										,

										symbol_id
									};

									std::size_t argument_index = 0;
									std::size_t content_position = 0;

									while (content_position < content.length())
									{
										const auto remaining_content = content.substr(content_position);

										/*
										// Type forwarding disabled for now; too much potential for
										// conflicts when there's multiple function overloads.
										// (i.e. type conversion should be performed at runtime)
										// 
										// See also: `MetaTypeDescriptor::set_variables_impl` -- Similar situation.
										const auto argument_type = (fn)
											? fn.arg(argument_index)
											: MetaType {}
										;
										*/

										const auto argument_type = MetaType {};

										auto [argument, length_processed] = meta_any_from_string_compound_expr_impl
										(
											remaining_content,
											instructions,
											argument_type
										);

										if (!argument)
										{
											break;
										}

										function_out.arguments.emplace_back(std::move(argument));

										content_position += length_processed;
										argument_index++;
									}

									return enqueue(std::move(function_out));
								}
							}
						}
					}
				}

				if (type)
				{
					if (auto as_data = resolve_data_member_by_id(type, true, symbol_id))
					{
						if (as_data.is_static())
						{
							if (auto result = as_data.get({}))
							{
								return replace(std::move(result));
							}
						}
						else
						{
							if (!output.empty())
							{
								const auto& latest_segment = output.segments[0];

								if (auto result = as_data.get(latest_segment.value))
								{
									return replace(std::move(result));
								}
							}

							if (instructions.allow_member_references)
							{
								return enqueue
								(
									MetaDataMember
									{
										type.id(),

										symbol_id
									}
								);
							}

							type = as_data.type();

							return true;
						}
					}

					if (auto as_prop = type.prop(symbol_id))
					{
						if (auto prop_value = as_prop.value())
						{
							return replace(std::move(prop_value));
						}
					}

					if (instructions.allow_function_call_semantics)
					{
						if (auto as_basic_function = type.func(symbol_id))
						{
							do
							{
								const auto argument_count = as_basic_function.arity();

								if (argument_count == 0)
								{
									return enqueue(MetaFunctionCall { type.id(), symbol_id });
								}
								else
								{
									const auto first_arg_type = as_basic_function.arg(0);

									if (first_arg_type.id() == entt::type_hash<Registry>::value()) // resolve<Registry>().id()
									{
										return enqueue(MetaFunctionCall { type.id(), symbol_id });
									}
								}

								as_basic_function = as_basic_function.next();
							} while (as_basic_function);
						}
					}

					if (auto str_fn = type.func("string_to_value"_hs))
					{
						if (auto result = str_fn.invoke({}, symbol))
						{
							return replace(std::move(result));
						}
					}

					if (instructions.allow_property_translation && (!is_command))
					{
						if (auto meta_property = resolve_property(type, symbol, symbol_id))
						{
							return enqueue(std::move(*meta_property));
						}
					}
				}

				// Fallback control-paths:
				if (!symbol.empty())
				{
					// NOTE: These if-conditions are split this way intentionally to allow for context-specific prioritization of fallback methods.
					// (e.g. type-references are higher priority than entity-references, system-types take precedence over component-types, etc.)

					if (!is_command)
					{
						if (instructions.resolve_system_references)
						{
							// See previous `resolve_system_references` section for regular system-type deduction.
							if (output.empty() && is_last_symbol)
							{
								if (auto as_type = get_as_type())
								{
									if (type_is_system(as_type)) // && allow_system_references
									{
										return enqueue(MetaTypeReference { as_type.id() });
									}
								}
							}
						}
						
						if
						(
							((!output.empty()) && (output.segments[0].value.type().id() == "MetaVariableTarget"_hs)) // resolve<MetaVariableTarget>()
							||
							(
								(instructions.fallback_to_component_reference && allow_component_fallback)
								&&
								(
									(output.empty())
									|| (!type)
									|| (type.id() == "Entity"_hs) // resolve<Entity>()
								)
							)
						)
						{
							if (auto as_type = get_as_type())
							{
								return enqueue
								(
									MetaTypeReference
									{
										.type_id = as_type.id(),
										.validate_assignment_type = false
									}
								);
							}
						}
					}

					if (output.empty())
					{
						if ((instructions.fallback_to_entity_reference && instructions.allow_entity_indirection) && (allow_entity_fallback && !is_command))
						{
							return enqueue(EntityTarget { EntityTarget::EntityNameTarget { symbol_id } }); // EntityTarget::from_string(symbol)
						}
					}
					else
					{
						if ((!is_command) && (instructions.allow_member_references && instructions.allow_opaque_member_references)) // !is_first_symbol && ...
						{
							return enqueue
							(
								/*
								// Alternative implementation (doesn't support 'property' accessors):
								MetaDataMember
								{
									MetaTypeID {},
									symbol_id
								}
								*/

								make_opaque_property(symbol, symbol_id)
							);
						}
					}
				}

				return true;
			}
		);

		return values_processed;
	}

	static std::optional<IndirectMetaVariableTarget> meta_any_resolve_remote_variable_reference
	(
		std::string_view first_symbol,
		std::string_view second_symbol,

		const MetaParsingInstructions& instructions,

		std::optional<EntityTarget> target=std::nullopt,

		bool validate_first_symbol=true,
		bool validate_second_symbol=true
	)
	{
		if (first_symbol.empty())
		{
			return std::nullopt;
		}

		if (!target && second_symbol.empty())
		{
			return std::nullopt;
		}

		auto validate_symbol = [&instructions](const std::string_view& symbol, bool validate_not_local_variable=false, bool validate_not_type=true)
		{
			if (symbol.empty())
			{
				return false;
			}

			if (validate_not_type)
			{
				const auto opt_type_context = instructions.context.get_type_context();

				const auto type = (opt_type_context)
					? opt_type_context->get_type(symbol, instructions)
					: resolve(hash(symbol).value())
				;

				if (type)
				{
					return false;
				}
			}

			if (validate_not_local_variable)
			{
				if (const auto opt_variable_context = instructions.context.get_variable_context())
				{
					if (opt_variable_context->contains(symbol))
					{
						return false;
					}
				}
			}

			return true;
		};

		if (validate_first_symbol)
		{
			if (!validate_symbol(first_symbol, (!target || !target->is_self_targeted()), true))
			{
				return std::nullopt;
			}
		}

		if (!target)
		{
			// Default to being self-targeted.
			target = EntityTarget {};
		}

		auto variable_id = MetaSymbolID {};
		auto variable_scope = MetaVariableScope::Local;

		std::optional<EntityThreadID> thread_id = std::nullopt;

		if (second_symbol.empty())
		{
			const auto& variable_name = first_symbol;

			if (target->is_self_targeted())
			{
				variable_scope = MetaVariableScope::Local;
			}
			else
			{
				variable_scope = MetaVariableScope::Global;
			}

			variable_id = MetaVariableContext::resolve_path
			(
				MetaSymbolID {},
				variable_name,
				variable_scope
			);
		}
		else
		{
			if (validate_second_symbol)
			{
				if (!validate_symbol(second_symbol, false, true))
				{
					return std::nullopt;
				}
			}

			const auto& thread_name = first_symbol;
			const auto& variable_name = second_symbol;

			thread_id = hash(thread_name).value();

			variable_scope = MetaVariableScope::Local;

			variable_id = MetaVariableContext::resolve_path
			(
				*thread_id,
				variable_name,
				variable_scope
			);
		}

		return IndirectMetaVariableTarget
		{
			*target,

			MetaVariableTarget
			{
				variable_id,
				variable_scope
			},
									
			(thread_id)
				? EntityThreadTarget { *thread_id }
				: EntityThreadTarget {}
		};
	}

	// Primary routine for resolving a `MetaAny` string expression.
	static MetaAny meta_any_from_string_resolve_expression_impl
	(
		std::string_view value,
		
		const MetaParsingInstructions& instructions,
		
		MetaType type={},
		
		bool try_arithmetic=true,
		bool try_string_fallback=true,
		bool try_boolean=true,
		bool allow_entity_fallback=true,
		bool allow_component_fallback=true,
		bool allow_standalone_opaque_function=true,
		bool allow_remote_variables=true
	)
	{
		using namespace engine::literals;

		const bool value_is_quoted = util::is_quoted(value);

		if (!instructions.allow_implicit_type_construction)
		{
			if (value_is_quoted)
			{
				return meta_any_from_string_to_string_impl(value, instructions.strip_quotes, instructions.storage);
			}

			if (try_arithmetic && instructions.allow_numeric_literals)
			{
				if (auto as_arithmetic = meta_any_from_string_arithmetic_impl(value, instructions, type))
				{
					return as_arithmetic;
				}
			}

			if (try_boolean && instructions.allow_boolean_literals)
			{
				if (auto as_boolean = meta_any_from_string_boolean_impl(value, instructions, type))
				{
					return as_boolean;
				}
			}
		}

		if (!value_is_quoted)
		{
			MetaValueOperation output;

			if (allow_remote_variables && instructions.allow_remote_variable_references)
			{
				auto
				[
					entity_target_parse_result,
					first_symbol,
					second_symbol,
					access_operator,
					updated_offset
				] = parse_qualified_reference(value, 0, &instructions, true);

				if (updated_offset > 0)
				{
					std::size_t remainder_offset = 0;

					std::optional<EntityTarget> target = std::nullopt;

					if (entity_target_parse_result)
					{
						auto& target_type = std::get<0>(*entity_target_parse_result);

						target = EntityTarget { std::move(target_type) };
					}

					if (auto remote_variable_reference = meta_any_resolve_remote_variable_reference(first_symbol, second_symbol, instructions, target))
					{
						output.segments.emplace_back
						(
							allocate_meta_any
							(
								std::move(*remote_variable_reference),
								instructions.storage
							),

							MetaValueOperator::Get
						);

						remainder_offset = updated_offset;
					}
					// Minor optimization to reduce `EntityTarget` parsing:
					else if (target && entity_target_parse_result)
					{
						auto entity_target_parse_offset = std::get<1>(*entity_target_parse_result);

						output.segments.emplace_back(allocate_meta_any(*target, instructions.storage), MetaValueOperator::Get);

						remainder_offset = entity_target_parse_offset;
					}

					if (remainder_offset > 0)
					{
						value = util::trim(value.substr(remainder_offset));
					}
				}
			}

			meta_any_from_string_resolve_expression_impl
			(
				output, value, instructions, type,
				allow_entity_fallback, allow_component_fallback,
				allow_standalone_opaque_function, allow_remote_variables
			);

			if (!output.empty())
			{
				if ((output.segments.size() == 1) && (output.segments[0].operation == MetaValueOperator::Get))
				{
					return std::move(output.segments[0].value);
				}

				return allocate_meta_any(std::move(output), instructions.storage);
			}

			if (try_string_fallback && instructions.fallback_to_string)
			{
				return meta_any_from_string_fallback_to_string_impl(value, instructions, type, true);
			}
		}

		if (type && instructions.allow_implicit_type_construction)
		{
			auto initial_value = MetaAny {};

			auto type_desc = MetaTypeDescriptor(type);

			if (value_is_quoted)
			{
				if (instructions.strip_quotes)
				{
					value = util::unquote_safe(value);
				}
			}
			else
			{
				if (try_arithmetic && instructions.allow_numeric_literals)
				{
					initial_value = meta_any_from_string_arithmetic_impl(value, instructions, type);
				}

				if (!initial_value)
				{
					if (try_boolean && instructions.allow_boolean_literals)
					{
						initial_value = meta_any_from_string_boolean_impl(value, instructions, type);
					}
				}
			}

			if (!initial_value)
			{
				// Raw `std::string_view`
				initial_value = allocate_meta_any(value, instructions.storage);
			}

			if (!initial_value)
			{
				return {};
			}

			if (type_desc.set_variable(MetaVariable { MetaSymbolID {}, std::move(initial_value) }, true, true))
			{
				if (auto instance = type_desc.instance_exact())
				{
					return std::move(instance);
				}
				
				auto& first_value = type_desc.field_values[0];

				// NOTE: See below `TODO` section about temporary string instances.
				first_value = meta_any_from_string_to_string_impl(value, false, {}); // instructions.storage

				if (!first_value)
				{
					return {};
				}

				if (auto instance = type_desc.instance_exact())
				{
					// TODO: Add some form of deallocation step for temporary strings
					// allocated with `instructions.storage` during implicit conversions.
					// 
					// For now, we'll forgo usage of `instructions.storage` here.

					return std::move(instance);
				}
			}
		}

		return {};
	}

	/*
	static MetaAny meta_any_from_string_trailing_expression_impl(MetaAny&& current_value, std::string_view trailing_expr, const MetaParsingInstructions& instructions, MetaType type={})
	{
		if (trailing_expr.empty())
		{
			return std::move(current_value);
		}

		if (!type)
		{
			auto current_value_type = current_value.type();

			if (!type_has_indirection(current_value_type))
			{
				type = current_value_type;
			}
		}

		auto output = MetaValueOperation {{{ std::move(current_value), MetaValueOperator::Get }}};

		meta_any_from_string_resolve_expression_impl(output, trailing_expr, instructions, type);

		if (!output.empty())
		{
			if (output.segments.size() == 1)
			{
				return std::move(output.segments[0].value);
			}

			return output;
		}

		return {};
	}
	*/

	std::tuple<MetaAny, std::size_t>
	meta_any_from_string_compound_expr_impl
	(
		std::string_view expr,
		
		const MetaParsingInstructions& instructions,
		MetaType type,
		
		bool try_arithmetic,
		bool try_string_fallback,
		bool try_boolean,
		bool allow_entity_fallback,
		bool allow_component_fallback,
		bool allow_standalone_opaque_function,
		bool allow_remote_variables,

		bool assume_static_type
	)
	{
		using namespace engine::literals;

		using OperatorType = std::optional<std::tuple<MetaValueOperator, MetaOperatorPrecedence>>;

		if (!instructions.resolve_symbol)
		{
			// NOTE: This should only be able to happen if this function has been called outside of `meta_any_from_string`.
			return { meta_any_from_string(expr, instructions, type, try_string_fallback, try_arithmetic), expr.length() };
		}

		std::size_t offset = 0;
		MetaValueOperation operation_out;

		OperatorType prev_operator = std::nullopt;

		std::size_t finalized_sub_expressions = 0;

		MetaAny current_value;

		std::string_view remainder;

		auto update_offset = [&expr, &offset](std::string_view current_expr, bool is_completed)
		{
			if (current_expr.empty())
			{
				return;
			}

			auto updated_offset = static_cast<std::size_t>(current_expr.data() - (expr.data()));

			if (is_completed)
			{
				updated_offset += current_expr.length();
			}

			offset = std::max(offset, updated_offset);
		};

		// Remove leading whitespace via offset.
		update_offset(util::trim(expr, util::whitespace_symbols), false);

		const auto initial_offset = offset;

		auto cast_type = MetaType {};

		bool owns_cast_type = false;
		bool cast_applied   = false;

		// Enqueues a deferred cast operation using `cast_type`.
		// 
		// If this operation is successful, this will return true, the active
		// `type` will be changed, and `cast_type` will be default constructed.
		auto try_encode_cast_type = [&instructions, &cast_type, &type]
		(MetaValueOperation& operation, MetaValueOperator next_operation=MetaValueOperator::Get, bool decay=true) -> bool
		{
			//assert(cast_type);

			if (!cast_type)
			{
				return false;
			}
			
			operation.segments.emplace_back
			(
				allocate_meta_any(MetaTypeConversion { cast_type.id() }, instructions.storage),
				next_operation
			);

			type = cast_type;

			if (decay)
			{
				cast_type = {};
			}

			return true;
		};

		while (offset < expr.size())
		{
			const bool is_first_expr = (offset == initial_offset);

			const auto current_expr = expr.substr(offset);
			auto current_expr_cutoff = current_expr.length();

			std::string_view current_value_raw;

			MetaType current_value_type;

			bool is_unary_placeholder_value = false;

			if (auto [unary_operator_position, unary_operator_symbol] = util::find_operator(current_expr); unary_operator_position == 0)
			{
				constexpr bool handle_operator_as_value = false;

				if constexpr (handle_operator_as_value)
				{
					current_value_raw = unary_operator_symbol;
					remainder = current_expr.substr(unary_operator_position + unary_operator_symbol.length());
				}
				else
				{
					remainder = current_expr;

					current_value = {};
					current_value_type = {};

					is_unary_placeholder_value = true;

					switch (hash(unary_operator_symbol))
					{
						case "!"_hs:
							cast_type = resolve<bool>();
							owns_cast_type = true;
							//current_value_type = cast_type;

							break;
					}
				}
			}
			else
			{
				const auto [paren_begin, paren_end] = util::find_parentheses(current_expr);

				if ((paren_begin != std::string_view::npos) && (paren_end == std::string_view::npos))
				{
					print_warn("Mismatched parentheses detected.");

					break;
				}

				const bool has_parentheses = (paren_end != std::string_view::npos);

				const auto [quote_begin, quote_end] = util::find_quotes(current_expr);

				/*
				// Disabled due to not being a possible output with current implementation:
				if ((quote_begin != std::string_view::npos) && (quote_end == std::string_view::npos))
				{
					print_warn("Mismatched quotes detected.");

					break;
				}
				*/

				const bool has_quotes = (quote_end != std::string_view::npos);

				std::size_t scope_begin = std::string_view::npos;
				std::size_t scope_end = std::string_view::npos;

				bool scope_symbols_included_in_expr = false;

				if (has_quotes && (!has_parentheses || (quote_begin < paren_begin)))
				{
					scope_begin = quote_begin;
					scope_end = quote_end;

					scope_symbols_included_in_expr = true;
				}
				else if (has_parentheses)
				{
					scope_begin = paren_begin;
					scope_end = paren_end;
				}

				std::size_t projected_operator_position = std::string_view::npos;

				const bool isolate_value_resolution = (scope_begin == 0);

				if (isolate_value_resolution)
				{
					if (!instructions.resolve_value_operations)
					{
						break;
					}

					if (scope_symbols_included_in_expr)
					{
						current_value_raw = current_expr.substr(scope_begin, (scope_end + 1));
					}
					else
					{
						current_value_raw = current_expr.substr((scope_begin + 1), (scope_end-1));
					}

					if ((scope_end + 1) < current_expr.length())
					{
						const auto trailing_expr = current_expr.substr((scope_end + 1));

						auto projected_operator_sub_position = std::get<0>(util::find_operator(trailing_expr));

						if (projected_operator_sub_position == std::string_view::npos)
						{
							projected_operator_sub_position = current_expr.find(',');
						}

						if (projected_operator_sub_position != std::string_view::npos)
						{
							projected_operator_position = static_cast<std::size_t>((trailing_expr.data() + projected_operator_sub_position) - current_expr.data());
						}
					}
				}
				else
				{
					// Ensure we're only looking at the term before an operator, rather than an entire expression:
					projected_operator_position = std::get<0>(util::find_operator(current_expr));

					if (projected_operator_position == std::string_view::npos)
					{
						projected_operator_position = current_expr.find(',');
					}
				}

				owns_cast_type = false;

				auto cast_type_raw = std::string_view {};

				constexpr std::string_view type_cast_operator = ":";

				std::size_t type_cast_operator_offset = 0;

				while (type_cast_operator_offset < current_expr.length())
				{
					const auto type_cast_operator_position = util::find_singular(current_expr, type_cast_operator, type_cast_operator_offset);

					if (type_cast_operator_position == std::string_view::npos)
					{
						// No cast operator found, stop searching.
						break;
					}

					// Check if this cast operator is within an established scope.
					if ((scope_end != std::string_view::npos) && (type_cast_operator_position > scope_begin) && (type_cast_operator_position < scope_end)) // (scope_begin != std::string_view::npos) && ...
					{
						// Bypass the scope in question.
						type_cast_operator_offset = (scope_end + 1);

						continue;
					}

					// Ensure there's content to process after the type-cast operator.
					if ((type_cast_operator_position + type_cast_operator.length()) >= current_expr.length())
					{
						break;
					}

					// Ensure we haven't move too far into the current expression (i.e. past the projected operator).
					if ((projected_operator_position != std::string_view::npos) && (type_cast_operator_position >= projected_operator_position))
					{
						break;
					}

					const auto cast_type_area_raw = current_expr.substr((type_cast_operator_position + type_cast_operator.length()));

					current_expr_cutoff = static_cast<std::size_t>((cast_type_area_raw.data() - type_cast_operator.length()) - current_expr.data());

					const auto cast_type_area_begin = cast_type_area_raw.find_first_not_of(util::whitespace_symbols);

					if (cast_type_area_begin == std::string_view::npos)
					{
						// Couldn't find anything but whitespace characters, stop searching.
						break;
					}

					const auto cast_type_area = cast_type_area_raw.substr(cast_type_area_begin);

					if (const auto cast_type_length = cast_type_area.find_first_of(util::whitespace_symbols); cast_type_length != std::string_view::npos)
					{
						if (projected_operator_position == std::string_view::npos)
						{
							// NOTE: No need to trim here, since the only whitespace that
							// would be included must be after the bounds of the type specifier.
							cast_type_raw = cast_type_area.substr(0, cast_type_length);
						}
						else
						{
							const auto projected_operator_position_in_cast_type = static_cast<std::ptrdiff_t>((current_expr.data() + projected_operator_position) - cast_type_area.data());

							if ((projected_operator_position_in_cast_type > 0) && (projected_operator_position_in_cast_type < cast_type_length))
							{
								cast_type_raw = util::trim(cast_type_area.substr(0, projected_operator_position_in_cast_type));
							}
							else
							{
								cast_type_raw = util::trim(cast_type_area.substr(0, cast_type_length));
							}
						}
					}
					else if (projected_operator_position != std::string_view::npos)
					{
						cast_type_raw = util::trim(cast_type_area.substr(0, projected_operator_position));
					}
					// Check for end-of-string:
					else if ((cast_type_area.data() + cast_type_area.length()) == (current_expr.data() + current_expr.length()))
					{
						cast_type_raw = util::trim(cast_type_area);
					}

					if (!cast_type_raw.empty())
					{
						if (instructions.allow_explicit_type_construction)
						{
							if (const auto opt_type_context = instructions.context.get_type_context())
							{
								cast_type = opt_type_context->get_type
								(
									cast_type_raw,

									instructions.resolve_component_aliases,
									instructions.resolve_command_aliases,
									instructions.resolve_instruction_aliases
								);
							}
							else
							{
								const auto type_id = hash(cast_type_raw).value();

								cast_type = resolve(type_id);
							}

							owns_cast_type = static_cast<bool>(cast_type);

							update_offset(cast_type_raw, true);
						}
						else
						{
							print_warn("Ignored type-cast expression for: `{}` (Disabled by caller)", cast_type_raw);
						}
					}

					break;
				
					//type_cast_operator_offset = (type_cast_operator_position + 1);
				}

				if (isolate_value_resolution)
				{
					if (!cast_type_raw.empty())
					{
						//remainder = current_expr.substr(current_expr_cutoff);

						const auto cast_type_raw_trailing_offset = ((cast_type_raw.data() + cast_type_raw.length()) - current_expr.data());

						remainder = current_expr.substr(cast_type_raw_trailing_offset);
					}
					else if (const auto after_scope_end = (scope_end + 1); after_scope_end < current_expr.length())
					{
						remainder = current_expr.substr(after_scope_end);
					}
					else
					{
						remainder = {};
					}
				}
				else
				{
					if (projected_operator_position == std::string_view::npos)
					{
						current_value_raw = current_expr.substr(0, current_expr_cutoff);
						remainder = {};
					}
					else if ((scope_end == std::string_view::npos) || (projected_operator_position < scope_begin) || (projected_operator_position > scope_end))
					{
						current_value_raw = current_expr.substr(0, std::min(current_expr_cutoff, projected_operator_position));
						remainder = current_expr.substr(projected_operator_position);
					}
					else // if ((scope_end != std::string_view::npos) && (projected_operator_position > scope_begin && projected_operator_position < scope_end)) // && (scope_begin != std::string_view::npos)
					{
						const auto operator_area = current_expr.substr(scope_end);

						// Ensure we're only looking at the term before an operator, rather than an entire expression:
						auto [trailing_operator_position, trailing_operator_raw] = util::find_operator(operator_area);

						if (trailing_operator_position == std::string_view::npos)
						{
							trailing_operator_position = operator_area.find(',');
						}

						if (trailing_operator_position == std::string_view::npos)
						{
							current_value_raw = current_expr.substr(0, current_expr_cutoff);
							remainder = {};
						}
						else
						{
							const auto expr_end_point = static_cast<std::size_t>((operator_area.data() + trailing_operator_position) - current_expr.data());

							current_value_raw = current_expr.substr(0, expr_end_point);
							remainder = current_expr.substr(expr_end_point);
						}
					}
				}

				if (current_value_raw.empty())
				{
					break;
				}

				if (isolate_value_resolution)
				{
					// NOTE: Potential recursion.
					current_value = meta_any_from_string
					(
						current_value_raw,
						instructions,
					
						// See below for cast behavior.
						MetaType {}, // type,

						true, true

						//(!is_first_expr || try_arithmetic),
						//(!is_first_expr || try_string_fallback)
					);
				}
				else
				{
					current_value = meta_any_from_string_resolve_expression_impl
					(
						current_value_raw,
						instructions,

						// See below for cast behavior.
						type, // MetaType {}

						(!is_first_expr || try_arithmetic),
						(!is_first_expr || try_string_fallback),
						(!is_first_expr || try_boolean),
						(!is_first_expr || allow_entity_fallback),
						(!is_first_expr || allow_component_fallback),
						(!is_first_expr || allow_standalone_opaque_function),
						(!is_first_expr || allow_remote_variables)
					);
				}

				if (!current_value)
				{
					break;
				}

				current_value_type = current_value.type();
			}

			update_offset(current_value_raw, true);

			remainder = util::trim(remainder);

			// TODO: Look into optimizing this by re-using the results during the beginning of the loop.
			auto [current_operator_symbol, trailing_expr] = util::parse_standard_operator_segment(remainder);

			bool exit_early = false;

			if (current_operator_symbol.empty())
			{
				if (auto exit_operator = remainder.find(','); exit_operator != std::string_view::npos)
				{
					constexpr std::size_t exit_operator_length = 1;

					// Update the offset to account for the exit (`,`) operator.
					offset += (exit_operator + exit_operator_length);
				}

				exit_early = true;
			}
			else
			{
				update_offset(current_operator_symbol, true);

				if (!instructions.resolve_value_operations)
				{
					exit_early = true;
				}
			}

			OperatorType current_operator;

			if (exit_early)
			{
				if (prev_operator)
				{
					current_operator = prev_operator;
				}
			}
			else
			{
				current_operator = MetaValueOperation::get_operation(current_operator_symbol, is_unary_placeholder_value);

				if (!current_operator)
				{
					break;
				}

				if (!prev_operator)
				{
					prev_operator = current_operator;
				}
			}

			const auto current_operator_is_assignment = (current_operator)
				? is_assignment_operation(std::get<0>(*current_operator)) // current_operator_value
				: false
			;

			const auto current_value_type_has_indirection = (type_has_indirection(current_value_type));

			cast_applied = false;

			if (!is_unary_placeholder_value)
			{
				if (!assume_static_type) // && !is_comparison_operation(current_operator_value)
				{
					if (type)
					{
						// NOTE: May change/remove this condition later.
						if (!current_operator_is_assignment)
						{
							if (((type != current_value_type) && (!cast_type))) // && (!cast_applied) // ((...) || current_value_type_has_indirection)
							{
								cast_type = type;
								owns_cast_type = true;
							}
						}
					}
					else if (current_value_type)
					{
						if (current_value_type_has_indirection)
						{
							if (current_value_type.id() == "IndirectMetaAny"_hs)
							{
								if (instructions.storage)
								{
									if (const auto* as_indirect = current_value.try_cast<IndirectMetaAny>())
									{
										if (auto remote_value = as_indirect->get(*instructions.storage))
										{
											if (auto underlying_type = try_get_underlying_type(remote_value))
											{
												// Only attempt to cast using the underlying type if there is no user-supplied
												// cast-type and there is another term involved in this expression.
												if ((!cast_type) && ((prev_operator) || (current_operator) || (!operation_out.empty()))) // (!cast_type) && (other_term_involved)
												{
													cast_type = underlying_type;
													owns_cast_type = true;
												}
												else
												{
													type = underlying_type;
												}
											}
										}
									}
								}
							}
							else
							{
								if (auto underlying_type = try_get_underlying_type(current_value))
								{
									// Only attempt to cast using the underlying type if there is no user-supplied
									// cast-type and there is another term involved in this expression.
									if ((!cast_type) && ((prev_operator) || (current_operator) || (!operation_out.empty()))) // (!cast_type) && (other_term_involved)
									{
										cast_type = underlying_type;
										owns_cast_type = true;
									}
									else
									{
										type = underlying_type;
									}
								}
							}
						}
						else
						{
							type = current_value_type;
						}
					}
				}

				if (!current_value_type_has_indirection)
				{
					if (cast_type)
					{
						// NOTE: In the event of indirection, `cast_type` will be
						// encoded as an extra step in `operation_out`. (See `try_encode_cast_type`)
						if (try_direct_cast(current_value, cast_type))
						{
							type = cast_type;

							if (owns_cast_type)
							{
								cast_type = {};
							}

							cast_applied = true;
						}
					}
				}
			}

			if (!current_operator)
			{
				break;
			}

			const auto& current_operator_value = std::get<0>(*current_operator);

			if (!is_unary_placeholder_value)
			{
				// Ensure that only the `+` operator (concatenation) is being used with string types,
				// and that only explicit (quoted) strings are used when doing so:
				if ((current_value_type) && (current_value_type == resolve<std::string>())) // current_value_type.id() == entt::type_id<std::string>().hash()
				{
					const auto current_operator_is_direct_comparison =
					(
						(current_operator_value == MetaValueOperator::Equal)
						||
						(current_operator_value == MetaValueOperator::NotEqual)
					);

					if (!current_operator_is_assignment && !current_operator_is_direct_comparison && (current_operator_value != MetaValueOperator::Add))
					{
						// Exit immediately; invalid operation.
						return {};
					}
				}

				if (!current_operator_is_assignment)
				{
					if (cast_type && !cast_applied)
					{
						auto compound_value = MetaValueOperation {};

						if (try_encode_cast_type(compound_value))
						{
							compound_value.segments.emplace_back
							(
								std::move(current_value),
								current_operator_value
							);

							current_value = allocate_meta_any(std::move(compound_value), instructions.storage);

							cast_applied = true;
						}
					}
				}
			}

			if (!prev_operator)
			{
				break;
			}

			const auto& prev_operator_value = std::get<0>(*prev_operator);
			const auto& prev_operator_precedence = std::get<1>(*prev_operator);

			const auto& current_operator_precedence = std::get<1>(*current_operator);

			if (operation_out.segments.empty() || (current_operator_precedence == prev_operator_precedence))
			{
				operation_out.segments.emplace_back
				(
					std::move(current_value),
					current_operator_value
				);
			}
			else if (current_operator_precedence < prev_operator_precedence)
			{
				operation_out = MetaValueOperation
				{
					{
						{
							allocate_meta_any(std::move(operation_out), instructions.storage),
							prev_operator_value
						},
						
						{
							std::move(current_value),
							current_operator_value
						}
					}
				};

				finalized_sub_expressions = 1;
			}
			else
			{
				auto compound_out = MetaValueOperation {};

				for (auto i = finalized_sub_expressions; i < operation_out.segments.size(); i++)
				{
					compound_out.segments.emplace_back(std::move(operation_out.segments[i]));
				}

				compound_out.segments.emplace_back
				(
					std::move(current_value),
					current_operator_value
				);

				operation_out.segments.resize(finalized_sub_expressions);

				operation_out.segments.emplace_back
				(
					allocate_meta_any(std::move(compound_out), instructions.storage),

					current_operator_value
				);

				finalized_sub_expressions++;
			}

			current_value = {};

			update_offset(trailing_expr, false);

			if (exit_early)
			{
				break;
			}

			if (trailing_expr.empty())
			{
				offset = expr.length();

				break;
			}

			if (current_operator_is_assignment && instructions.allow_variable_assignment)
			{
				// NOTE: Possible recursion.
				auto assignment_value = meta_any_from_string
				(
					trailing_expr,
					instructions, type,

					true, true

					//(!is_first_expr || try_arithmetic),
					//(!is_first_expr || try_string_fallback)
				);

				if (!assignment_value)
				{
					break;
				}

				if (cast_type && !cast_applied)
				{
					if (value_has_indirection(assignment_value))
					{
						auto compound_value = MetaValueOperation {};

						if (try_encode_cast_type(compound_value))
						{
							cast_applied = true;

							compound_value.segments.emplace_back
							(
								std::move(assignment_value),
								current_operator_value
							);

							operation_out.segments.emplace_back
							(
								allocate_meta_any(std::move(compound_value), instructions.storage),
								current_operator_value
							);
						}
					}
					else
					{
						if (try_direct_cast(assignment_value, cast_type))
						{
							operation_out.segments.emplace_back
							(
								std::move(assignment_value),
								current_operator_value
							);

							cast_applied = true;
							cast_type = {};
						}
					}
				}
				else
				{
					operation_out.segments.emplace_back
					(
						std::move(assignment_value),
						current_operator_value
					);
				}

				break;
			}

			prev_operator = current_operator;
		}

		if (operation_out.segments.empty())
		{
			if (cast_type && !cast_applied)
			{
				try_encode_cast_type(operation_out);

				if (current_value)
				{
					operation_out.segments.emplace_back(std::move(current_value), MetaValueOperator::Get);

					return { allocate_meta_any(std::move(operation_out), instructions.storage), offset };
				}
				else if (operation_out.segments.size() == 1)
				{
					// Cast encoded as the only entry.
					return { std::move(operation_out.segments[0].value), offset };
				}
			}
			else
			{
				if (current_value)
				{
					return { std::move(current_value), offset };
				}
			}
		}
		else
		{
			const auto& latest_segment = operation_out.segments[operation_out.segments.size() - 1];

			if (cast_type && !cast_applied)
			{
				try_encode_cast_type(operation_out);
			}

			if (current_value)
			{
				operation_out.segments.emplace_back
				(
					std::move(current_value),

					latest_segment.operation
				);
			}

			if ((operation_out.segments.size() == 1)) // && (operation_out.segments[0].operation == MetaValueOperator::Get)
			{
				return { std::move(operation_out.segments[0].value), offset };
			}
			
			return { allocate_meta_any(std::move(operation_out), instructions.storage), offset };
		}

		return {};
	}

	MetaAny meta_any_from_string
	(
		std::string_view value,
		
		const MetaParsingInstructions& instructions,
		
		MetaType type,
		
		bool allow_string_fallback,
		bool allow_numeric_literals,
		bool allow_boolean_literals,
		bool allow_entity_fallback,
		bool allow_component_fallback,
		bool allow_standalone_opaque_function,
		bool allow_remote_variables
	)
	{
		using namespace engine::literals;

		if (!instructions.allow_implicit_type_construction || !instructions.resolve_symbol)
		{
			if (util::is_quoted(value))
			{
				return meta_any_from_string_to_string_impl(value, instructions.strip_quotes, instructions.storage);
			}

			if (allow_numeric_literals && instructions.allow_numeric_literals) // && instructions.resolve_symbol
			{
				if (auto as_arithmetic = meta_any_from_string_arithmetic_impl(value, instructions, type))
				{
					return as_arithmetic;
				}
			}

			if (allow_boolean_literals && instructions.allow_boolean_literals) // && instructions.resolve_symbol
			{
				if (auto as_boolean = meta_any_from_string_boolean_impl(value, instructions, type))
				{
					return as_boolean;
				}
			}
		}

		if (instructions.resolve_symbol)
		{
			auto [result, length_processed] = meta_any_from_string_compound_expr_impl
			(
				value, instructions, type,
				allow_numeric_literals, allow_string_fallback, allow_boolean_literals,
				allow_entity_fallback, allow_component_fallback, allow_standalone_opaque_function,
				allow_remote_variables
			);

			if (result)
			{
				//assert(length_processed == expr.length());

				return std::move(result);
			}
		}

		if (allow_string_fallback && instructions.fallback_to_string)
		{
			const bool arithmetic_type_check = (instructions.resolve_symbol);

			return meta_any_from_string_fallback_to_string_impl(value, instructions, type, arithmetic_type_check);
		}

		return {};
	}

	MetaAny meta_any_from_string
	(
		const util::json& value,
		
		const MetaParsingInstructions& instructions,
		
		MetaType type,

		bool allow_string_fallback,
		bool allow_numeric_literals,
		bool allow_boolean_literals,
		bool allow_entity_fallback,
		bool allow_component_fallback,
		bool allow_standalone_opaque_function,
		bool allow_remote_variables
	)
	{
		auto string_value = value.get<std::string>();

		return meta_any_from_string
		(
			std::string_view { string_value },
			
			instructions,
			
			type,

			allow_string_fallback,
			allow_numeric_literals,
			allow_boolean_literals,
			allow_entity_fallback,
			allow_component_fallback,
			allow_standalone_opaque_function,
			allow_remote_variables
		);
	}

	MetaAny resolve_meta_any
	(
		const util::json& value,
		MetaTypeID type_id,
		const MetaParsingInstructions& instructions
	)
	{
		return resolve_meta_any(value, entt::resolve(type_id), instructions);
	}

	MetaAny resolve_meta_any
	(
		const util::json& value,
		MetaType type,
		const MetaParsingInstructions& instructions
	)
	{
		using jtype = util::json::value_t;

		switch (value.type())
		{
			// Objects and arrays of a known type can
			// be immediately nested as a `MetaTypeDescriptor`:
			case jtype::array:
			case jtype::object:
			{
				if (type)
				{
					if (!instructions.defer_container_creation)
					{
						if (type.is_sequence_container())
						{
							if (auto result = impl::resolve_meta_any_sequence_container_impl(value, type, instructions))
							{
								return result;
							}
						}
						else if (type.is_associative_container())
						{
							if (auto result = impl::resolve_meta_any_associative_container_impl(value, type, instructions))
							{
								return result;
							}
						}
					}

					/*
					if (auto from_json = type.construct(value, instructions))
					{
						return from_json;
					}
					*/
				}

				// NOTE: Nesting `MetaTypeDescriptor` objects within the any-chain
				// implies recursion during object construction later on.
				return allocate_meta_any
				(
					MetaTypeDescriptor
					(
						type, value,
						instructions,
						std::nullopt,
						{}
					),

					instructions.storage
				);
			}

			case jtype::string:
				return meta_any_from_string(value, instructions, type);
		}

		return resolve_meta_any(value, instructions);
	}

	MetaAny resolve_meta_any
	(
		const util::json& value,
		const MetaParsingInstructions& instructions
	)
	{
		using jtype = util::json::value_t;

		switch (value.type())
		{
			case jtype::object:
			{
				// Completely opaque object detected.
				//assert(false);

				print_warn("Opaque JSON object detected (No type information): Opaque objects are not currently supported.");

				/*
				return MetaTypeDescriptor
				(
					MetaType {},
					value,
					instructions,
					std::nullopt,
					{}
				);
				*/

				// TODO: Maybe handle as array instead...?
				return {};
			}
			
			case jtype::array:
			{
				// Completely opaque array detected.
				//assert(false);

				print_warn("Opaque JSON array detected (No type information): Opaque arrays are not currently supported.");

				return {};
			}

			case jtype::binary:
			{
				// Binary data detected.
				//assert(false);

				print_warn("Binary detected: Not currently supported.");

				return {};
			}

			case jtype::string:
				return meta_any_from_string(value, instructions);
			case jtype::boolean:
				return value.get<bool>();
			case jtype::number_integer:
				return value.get<std::int32_t>(); // TODO: Handle larger integers, etc.
			case jtype::number_unsigned:
				return value.get<std::uint32_t>(); // TODO: Handle larger integers, etc.
			case jtype::number_float:
				return value.get<float>(); // 32-bit floats are standard for this engine.
		}

		return {};
	}

	std::optional<MetaDataMember> process_meta_data_member(std::string_view type_name, std::string_view data_member_name)
	{
		if (type_name.empty())
		{
			return std::nullopt;
		}

		const auto type_id = hash(type_name);
		const auto type = resolve(type_id);
		
		if (!type)
		{
			return std::nullopt;
		}

		const auto data_member_id = hash(data_member_name);
		const auto data_member = resolve_data_member_by_id(type, true, data_member_id);

		if (!data_member)
		{
			return std::nullopt;
		}

		return MetaDataMember { type_id, data_member_id };
	}

	// Overload added due to limitations of `std::regex`. (Shouldn't matter for most cases, thanks to SSO)
	std::optional<MetaDataMember> meta_data_member_from_string(std::string_view value)
	{
		const auto [type_name, data_member_name, parsed_length] = util::parse_member_reference(value);

		return process_meta_data_member(type_name, data_member_name);
	}

	std::optional<IndirectMetaDataMember> indirect_meta_data_member_from_string(std::string_view value, EntityTarget target)
	{
		const auto [type_name, data_member_name, parsed_length] = util::parse_member_reference(value);

		if (type_name.empty())
		{
			return std::nullopt;
		}

		if ((type_name == "self") || (type_name == "this"))
		{
			auto as_self = meta_data_member_from_string(value);

			if (!as_self)
			{
				return std::nullopt;
			}

			return IndirectMetaDataMember
			{
				std::move(target), // EntityTarget { EntityTarget::SelfTarget{} },
				std::move(*as_self)
			};
		}

		//auto data_member_reference = util::parse_member_reference(...);

		std::size_t target_parse_offset = 0;

		//EntityTarget::TargetType target = EntityTarget::SelfTarget{};

		if (auto target_parse_result = EntityTarget::parse_type(value))
		{
			target = { std::move(std::get<0>(*target_parse_result)) };
			target_parse_offset = std::get<1>(*target_parse_result);
		}

		auto member_reference_data = std::string_view { (value.data() + target_parse_offset), (value.size() - target_parse_offset) };

		if (auto as_target = meta_data_member_from_string(member_reference_data))
		{
			return IndirectMetaDataMember
			{
				std::move(target),
				std::move(*as_target)
			};
		}

		return std::nullopt;
	}

	EntityTarget::TargetType parse_target_type(const util::json& target_data)
	{
		using namespace engine::literals;

		using Target = EntityTarget;

		switch (target_data.type())
		{
			case util::json::value_t::string:
			{
				auto raw_value = target_data.get<std::string>();
				
				if (auto result = EntityTarget::parse_type(raw_value))
				{
					return std::get<0>(*result);
				}
				
				break;
			}

			case util::json::value_t::number_integer:
			case util::json::value_t::number_unsigned:
				return Target::ExactEntityTarget { static_cast<Entity>(target_data.get<entt::id_type>()) };
		}

		return Target::SelfTarget {};
	}

	std::optional<MetaVariableScope> parse_variable_scope(std::string_view scope_qualifier)
	{
		using namespace engine::literals;

		if (scope_qualifier.empty())
		{
			return std::nullopt;
		}

		auto scope_symbol_id = hash(scope_qualifier);

		switch (scope_symbol_id)
		{
			case "var"_hs:
			case "auto"_hs:
			case "field"_hs:
			case "local"_hs:
				return MetaVariableScope::Local;
			case "global"_hs:
				return MetaVariableScope::Global;
			case "context"_hs:
				return MetaVariableScope::Context;
			case "universal"_hs:
				return MetaVariableScope::Universal;
			case "shared"_hs:
				return MetaVariableScope::Shared; // Context;
		}

		return std::nullopt;
	}

	void read_type_context(MetaTypeResolutionContext& context, const util::json& data)
	{
		auto read_aliases = [](const util::json& aliases, auto& alias_container_out)
		{
			for (const auto& proxy : aliases.items())
			{
				const auto& raw_value = proxy.value();

				if (!raw_value.is_string())
				{
					continue;
				}

				const auto& alias = proxy.key();

				auto type_name = raw_value.get<std::string>();

				const auto type_id = hash(type_name);

				const auto type = resolve(type_id);

				if (!type)
				{
					continue;
				}

				alias_container_out[alias] = type.info().name(); // std::string(...);
			}
		};

		if (auto aliases = data.find("command_aliases"); aliases != data.end())
		{
			read_aliases(*aliases, context.command_aliases);
		}

		if (auto aliases = data.find("component_aliases"); aliases != data.end())
		{
			read_aliases(*aliases, context.component_aliases);
		}

		if (auto aliases = data.find("instruction_aliases"); aliases != data.end())
		{
			read_aliases(*aliases, context.instruction_aliases);
		}
	}

	MetaAny load
	(
		MetaAny out,
		
		const util::json& data,
		
		bool modify_in_place,

		const MetaParsingInstructions& parse_instructions,
		const MetaTypeDescriptorFlags& descriptor_flags
	)
	{
		auto descriptor = load_descriptor
		(
			out.type(),
			data,
			parse_instructions,
			descriptor_flags
		);

		if (modify_in_place)
		{
			descriptor.apply_fields(out);
		}
		else
		{
			return descriptor.instance();
		}

		return out;
	}

	MetaAny load
	(
		MetaType type,
		
		const util::json& data,

		const MetaParsingInstructions& parse_instructions,
		const MetaTypeDescriptorFlags& descriptor_flags
	)
	{
		auto descriptor = load_descriptor
		(
			type,
			data,
			parse_instructions,
			descriptor_flags
		);

		return descriptor.instance(); // false
	}
}