#pragma once

#include "types.hpp"
#include "meta_value_operator.hpp"

#include <util/small_vector.hpp>

#include <optional>
#include <string_view>
#include <tuple>

namespace engine
{
	class SharedStorageInterface;
	struct MetaEvaluationContext;

	// Encapsulates use of an operator on up to two opaque values.
	struct MetaValueOperation
	{
		public:
			using Operation = MetaValueOperator;
			using OperationPrecedence = MetaOperatorPrecedence;

			struct Segment
			{
				MetaAny value;
				Operation operation; // std::optional<Operation>

				MetaAny get() const;
				
				MetaAny get(const MetaAny& instance) const;
				MetaAny get(const MetaAny& instance, Registry& registry, Entity entity) const;
				MetaAny get(const MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

				MetaAny get(Registry& registry, Entity entity) const;
				MetaAny get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

				MetaType get_type() const;
				MetaTypeID get_type_id() const;
			};

			//using SegmentContainer = std::vector<Segment>;
			using SegmentContainer = util::small_vector<Segment, 4>;

			static bool contains_operator(std::string_view expr);
			
			static std::tuple
			<
				std::size_t, // symbol_position
				std::string_view // symbol
			>
			find_operator(std::string_view expr);

			static std::optional<std::tuple<Operation, OperationPrecedence>>
			get_operation(std::string_view symbol, bool symbol_is_leading=false, bool resolve_non_standard_symbols=false);

			MetaAny get() const;
			
			MetaAny get(const MetaAny& instance) const;
			MetaAny get(const MetaAny& instance, Registry& registry, Entity entity) const;
			MetaAny get(const MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

			MetaAny get(Registry& registry, Entity entity) const;
			MetaAny get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const;

			// Assigns the object produced (from evaluating this operation) to the `value` specified.
			MetaAny set(MetaAny& value);

			// Assigns the object produced (from evaluating this operation) to the `value` specified.
			MetaAny set(MetaAny& value, Registry& registry, Entity entity);

			// Assigns the object produced (from evaluating this operation) to the `value` specified.
			MetaAny set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			// Evaluates this operation with `source`, then assigns `destination` to the result.
			MetaAny set(MetaAny& source, MetaAny& destination);

			// Evaluates this operation with `source`, then assigns `destination` to the result.
			MetaAny set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity);

			// Evaluates this operation with `source`, then assigns `destination` to the result.
			MetaAny set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context);

			// Searches for an instance of `MetaFunctionCall` within `segments`.
			// 
			// If `recursive` is enabled, sub-operations will be checked as well.
			// 
			// `MetaAny` values are currently only checked if their type is
			// `MetaValueOperation` (hence `recursive`) or `MetaFunctionCall`.
			// 
			// The `IndirectMetaAny` type can also be checked against if
			// its underlying storage is provided. (See other overloads)
			bool contains_function_call(bool recursive=true) const;

			// Searches for an instance of `MetaFunctionCall` within `segments`.
			// 
			// If `recursive` is enabled, sub-operations will be checked as well.
			// 
			// `MetaAny` values are currently only checked if their type is
			// `MetaValueOperation`, `MetaFunctionCall`, or `IndirectMetaAny` (loaded from `entity`'s descriptor).
			bool contains_function_call(Registry& registry, Entity entity, bool recursive=true) const;

			// Searches for an instance of `MetaFunctionCall` within `segments`.
			// 
			// If `recursive` is enabled, sub-operations will be checked as well.
			// 
			// `MetaAny` values are currently only checked if their type is
			// `MetaValueOperation`, `MetaFunctionCall`, or `IndirectMetaAny` (loaded from `storage`).
			bool contains_function_call(const SharedStorageInterface& storage, bool recursive=true) const;

			// Returns true if there are no `segments` attached to this operation.
			inline bool empty() const
			{
				return segments.empty();
			}
			
			SegmentContainer segments;

		private:
			// Assigns the object produced (from evaluating this operation) to the `value` specified.
			template <typename ...Args>
			MetaAny set_impl(MetaAny& value, Args&&... args);

			// Evaluates this operation with `source`, then assigns `destination` to the result.
			template <typename ...Args>
			MetaAny chain_set_impl(MetaAny& source, MetaAny& destination, Args&&... args);

			bool contains_function_call_impl(bool recursive=true, const SharedStorageInterface* opt_storage=nullptr) const;

			template <typename SelfType, typename ...Args>
			static std::tuple
			<
				MetaAny,    // result
				std::size_t // entries_processed
			>
			evaluate_impl
			(
				SelfType&& self,
				bool allow_ref_as_return_value,
				std::size_t offset,
				std::optional<Operation> target_operation,
				bool allow_look_ahead,
				Args&&... args
			);
		};
}