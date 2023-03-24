#include "meta_value_operation.hpp"

#include "meta.hpp"
#include "hash.hpp"
#include "indirect_meta_any.hpp"
#include "meta_evaluation_context.hpp"
#include "apply_operation.hpp"

#include <engine/entity/components/instance_component.hpp>

#include <util/log.hpp>
#include <util/algorithm.hpp>
#include <util/parse.hpp>

#include <utility>
#include <type_traits>
#include <array>

//#include <limits>

namespace engine
{
	// MetaValueOperation:
	std::tuple
	<
		std::size_t, // symbol_position
		std::string_view // symbol
	>
	MetaValueOperation::find_operator(std::string_view expr)
	{
		return util::find_operator(expr);
	}

	bool MetaValueOperation::contains_operator(std::string_view expr)
	{
		return (std::get<0>(find_operator(expr)) != std::string_view::npos);
	}

	std::optional<std::tuple<MetaValueOperator, MetaOperatorPrecedence>>
	MetaValueOperation::get_operation(std::string_view symbol, bool symbol_is_leading, bool resolve_non_standard_symbols)
	{
		return parse_value_operator(symbol, symbol_is_leading, resolve_non_standard_symbols);
	}

	MetaAny MetaValueOperation::get() const
	{
		// NOTE: Const-cast used here to avoid sharing const-qualification on references to non-const `MetaAny` objects.
		// (i.e. If a value isn't stored as const in `segments`, we shouldn't retroactively apply const)
		return std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true));
	}

	MetaAny MetaValueOperation::get(const MetaAny& instance) const
	{
		// NOTE: Const-cast performed to avoid imposing transitive const in scenarios where it is unwanted.
		// `evaluate_impl` internally performs const-reference casts to attempt to preserve const-ness, where applicable.
		auto as_ref = const_cast<MetaAny&>(instance).as_ref();

		// NOTE: See parameterless overload for details on const-cast.
		return std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, as_ref));
	}

	MetaAny MetaValueOperation::get(const MetaAny& instance, Registry& registry, Entity entity) const
	{
		// NOTE: See basic chain-getter implementation for details on const-cast usage.
		auto as_ref = const_cast<MetaAny&>(instance).as_ref();

		// NOTE: See parameterless overload for details on const-cast.
		return std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, as_ref, registry, entity));
	}

	MetaAny MetaValueOperation::get(const MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		// NOTE: See basic chain-getter implementation for details on const-cast usage.
		auto as_ref = const_cast<MetaAny&>(instance).as_ref();

		// NOTE: See parameterless overload for details on const-cast.
		return std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, as_ref, registry, entity, context));
	}

	MetaAny MetaValueOperation::get(Registry& registry, Entity entity) const
	{
		// NOTE: See parameterless overload for details on const-cast.
		return std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, registry, entity));
	}

	MetaAny MetaValueOperation::get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		// NOTE: See parameterless overload for details on const-cast.
		return std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, registry, entity, context));
	}

	template <typename ...Args>
	MetaAny MetaValueOperation::set_impl(MetaAny& value, Args&&... args)
	{
		if (auto direct_evaluation_result = std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, value, args...))) // get(value, args...)
		{
			return direct_evaluation_result;
		}

		auto evaluation_result = std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, args...)); // get(args...);

		if (auto assignment_result = try_get_underlying_value(evaluation_result, value, args...))
		{
			return assignment_result;
		}

		return evaluation_result;
	}

	MetaAny MetaValueOperation::set(MetaAny& value)
	{
		return set_impl(value);
	}

	MetaAny MetaValueOperation::set(MetaAny& value, Registry& registry, Entity entity)
	{
		return set_impl(value, registry, entity);
	}

	MetaAny MetaValueOperation::set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set_impl(value, registry, entity, context);
	}

	template <typename ...Args>
	MetaAny MetaValueOperation::chain_set_impl(MetaAny& source, MetaAny& destination, Args&&... args)
	{
		if (source)
		{
			if (value_has_indirection(source))
			{
				if (auto source_resolved = try_get_underlying_value(source, args...))
				{
					if (auto evaluation_result = std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, source_resolved, destination, args...))) // get(source, args...)
					{
						return evaluation_result;
					}
					else
					{
						if (auto destination_result = try_get_underlying_value(destination, source_resolved, args...))
						{
							return destination_result;
						}
						else
						{
							destination = std::move(source_resolved);
						}
					}
				}
			}
			else
			{
				if (auto evaluation_result = std::get<0>(evaluate_impl(const_cast<MetaValueOperation&>(*this), true, 0, std::nullopt, true, source, destination, args...))) // get(source, args...)
				{
					return evaluation_result;
				}
				else
				{
					if (auto destination_result = try_get_underlying_value(destination, source, args...))
					{
						return destination_result;
					}
					else
					{
						destination = std::move(source);
					}
				}
			}
		}

		//return {};
		return destination.as_ref();
	}

	MetaAny MetaValueOperation::set(MetaAny& source, MetaAny& destination)
	{
		return chain_set_impl(source, destination);
	}

	MetaAny MetaValueOperation::set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity)
	{
		return chain_set_impl(source, destination, registry, entity);
	}

	MetaAny MetaValueOperation::set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return chain_set_impl(source, destination, registry, entity, context);
	}

	bool MetaValueOperation::contains_function_call(bool recursive) const
	{
		return contains_function_call_impl(recursive, nullptr);
	}

	bool MetaValueOperation::contains_function_call(Registry& registry, Entity entity, bool recursive) const
	{
		if (const auto instance_comp = registry.try_get<InstanceComponent>(entity))
		{
			const auto& storage = instance_comp->get_storage();

			return contains_function_call_impl(recursive, &storage);
		}

		return contains_function_call_impl(recursive);
	}

	bool MetaValueOperation::contains_function_call(const SharedStorageInterface& storage, bool recursive) const
	{
		return contains_function_call_impl(recursive, &storage);
	}

	bool MetaValueOperation::contains_function_call_impl(bool recursive, const SharedStorageInterface* opt_storage) const
	{
		using namespace engine::literals;

		auto on_entry = [recursive, &opt_storage](const MetaAny& value, MetaTypeID entry_type_id) -> bool
		{
			switch (entry_type_id)
			{
				case "MetaFunctionCall"_hs:
					return true;

				case "MetaValueOperation"_hs:
					if (recursive)
					{
						if (auto as_value_operation = value.try_cast<MetaValueOperation>())
						{
							if (as_value_operation->contains_function_call_impl(true, opt_storage)) // recursive
							{
								return true;
							}
						}
					}

					break;
			}

			return false;
		};

		for (const auto& entry : segments)
		{
			const auto& entry_value = entry.value;
			const auto entry_type_id = entry.get_type_id();

			switch (entry_type_id)
			{
				case "IndirectMetaAny"_hs:
					if (opt_storage)
					{
						if (auto as_indirect = entry_value.try_cast<IndirectMetaAny>())
						{
							if (auto underlying = as_indirect->get(*opt_storage))
							{
								const auto underlying_type_id = underlying.type().id();

								if (on_entry(underlying, underlying_type_id))
								{
									return true;
								}
							}
						}
					}

					break;
				default:
					if (on_entry(entry_value, entry_type_id))
					{
						return true;
					}

					break;
			}
		}

		return false;
	}

	template <typename SelfType, typename ...Args>
	std::tuple<MetaAny, std::size_t> MetaValueOperation::evaluate_impl
	(
		SelfType&& self,
		bool allow_ref_as_return_value,
		std::size_t offset,
		std::optional<Operation> target_operation,
		bool allow_look_ahead,
		Args&&... args
	)
	{
		if (self.segments.empty())
		{
			return {};
		}

		if (offset >= self.segments.size())
		{
			return {};
		}

		Segment result;

		const auto remaining_entries = (self.segments.size() - offset);

		auto& first = self.segments[offset];

		std::size_t concatenation_offset = 1;

		std::size_t segments_processed = 1;

		if (remaining_entries == 1)
		{
			if (is_unary_operation(first.operation))
			{
				result = { {}, first.operation };
				concatenation_offset = 0;
			}
			else if (first.operation == MetaValueOperator::Subscript)
			{
				if constexpr (sizeof...(args) > 0)
				{
					if constexpr (std::is_same_v<std::decay_t<util::get_first_type<Args...>>, MetaAny>)
					{
						result = { util::get_first(args...).as_ref(), first.operation};
						concatenation_offset = 0;
					}
					else
					{
						return { {}, 0 }; // segments_processed
					}
				}
				else
				{
					return { {}, 0 }; // segments_processed
				}
			}
			else
			{
				if (auto underlying = try_get_underlying_value(first.value, std::forward<Args>(args)...))
				{
					return { std::move(underlying), segments_processed };
				}

				if (allow_ref_as_return_value)
				{
					return { first.value.as_ref(), segments_processed };
				}

				return { first.value, segments_processed };
			}
		}
		else
		{
			result = { first.value.as_ref(), first.operation};
		}

		for (std::size_t i = (offset + concatenation_offset); i < self.segments.size(); i++)
		{
			auto& next_segment = self.segments[i];

			if (target_operation)
			{
				if (next_segment.operation != *target_operation)
				{
					break;
				}
			}

			if (!next_segment.value)
			{
				if ((result.operation == MetaValueOperator::Get) && (next_segment.operation == MetaValueOperator::Get))
				{
					continue;
				}

				break;
			}

			auto operation_result = apply_operation
			(
				result.value, next_segment.value,
				result.operation,

				std::forward<Args>(args)...
			);

			if (operation_result)
			{
				result =
				{
					std::move(operation_result),

					next_segment.operation
				};

				segments_processed++;
			}
			else
			{
				const auto look_ahead_next_index = (i + 1);

				if (!allow_look_ahead || look_ahead_next_index >= self.segments.size())
				{
					break;
				}

				auto& look_ahead_segment = self.segments[look_ahead_next_index];

				auto [look_ahead_processed_value, look_ahead_processed] = evaluate_impl(self, false, i, look_ahead_segment.operation, false, std::forward<Args>(args)...);

				if ((!look_ahead_processed_value) || (look_ahead_processed == 1))
				{
					break;
				}

				auto look_ahead_operation_result = apply_operation
				(
					result.value, look_ahead_processed_value,
					result.operation,

					std::forward<Args>(args)...
				);

				if (!look_ahead_operation_result)
				{
					break;
				}

				segments_processed += look_ahead_processed;

				// NOTE: We subtract one here, since this loop implicitly
				// increments by one after each iteration.
				i += (look_ahead_processed - 1);
			}
		}

		return { std::move(result.value), segments_processed };
	}

	// MetaValueOperation::Segment:
	MetaAny MetaValueOperation::Segment::get() const
	{
		return get_indirect_value_or_ref(value);
	}

	MetaAny MetaValueOperation::Segment::get(const MetaAny& instance) const
	{
		return get_indirect_value_or_ref(value, instance);
	}

	MetaAny MetaValueOperation::Segment::get(const MetaAny& instance, Registry& registry, Entity entity) const
	{
		return get_indirect_value_or_ref(value, instance, registry, entity);
	}

	MetaAny MetaValueOperation::Segment::get(const MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return get_indirect_value_or_ref(value, instance, registry, entity, context);
	}

	MetaAny MetaValueOperation::Segment::get(Registry& registry, Entity entity) const
	{
		return get_indirect_value_or_ref(value, registry, entity);
	}

	MetaAny MetaValueOperation::Segment::get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return get_indirect_value_or_ref(value, registry, entity, context);
	}

	MetaType MetaValueOperation::Segment::get_type() const
	{
		return value.type();
	}

	MetaTypeID MetaValueOperation::Segment::get_type_id() const
	{
		//return get_type().id();
		return value.type().id();
	}
}