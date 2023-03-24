#pragma once

#include "types.hpp"

#include <cstdint>
#include <string_view>
#include <tuple>
#include <optional>

namespace engine
{
	using MetaOperatorPrecedence = std::uint8_t;

	enum class MetaValueOperator : std::uint8_t
	{
		Get,
		Subscript,
		Dereference,

		UnaryPlus,
		UnaryMinus,

		Boolean,
		LogicalNot,
		BitwiseNot,

		Multiply,
		Divide,
		Modulus,
		Add,
		Subtract,

		ShiftLeft,
		ShiftRight,

		LessThan,
		LessThanOrEqual,
		GreaterThan,
		GreaterThanOrEqual,

		Equal,
		NotEqual,

		BitwiseAnd,
		BitwiseXOR,
		BitwiseOr,

		LogicalAnd,
		LogicalOr,

		Assign,

		AddAssign,
		SubtractAssign,
		MultiplyAssign,
		DivideAssign,
		ModulusAssign,
		ShiftLeftAssign,
		ShiftRightAssign,
		BitwiseAndAssign,
		BitwiseXORAssign,
		BitwiseOrAssign,

		// Currently handled elsewhere.
		//Comma
	};

	bool is_unary_operation(MetaValueOperator operation);
	bool is_assignment_operation(MetaValueOperator operation);
	bool is_comparison_operation(MetaValueOperator operation);

	// For operation precedence rules, see: https://en.cppreference.com/w/cpp/language/operator_precedence
	std::optional<std::tuple<MetaValueOperator, MetaOperatorPrecedence>>
	parse_value_operator(std::string_view symbol, bool symbol_is_leading=false, bool resolve_non_standard_symbols=false);

	// Attempts to decay `operation` to an underlying (non-compound) operation.
	// (i.e. `Operation::MultiplyAssign` -> `Operation::Multiply`)
	MetaValueOperator decay_operation(MetaValueOperator operation);

	// Retrieves the name (hash) of a function to perform the operation.
	// TODO: Look into making this constexpr.
	StringHash get_operator_name(MetaValueOperator operation);

	// Retrieves the decayed form of an operator function. (i.e. `operator+=` -> `operator+`)
	// If `operator_name` is already decayed, this will return `operator_name` back to the caller.
	StringHash decay_operator_name(StringHash operator_name);

	// Shorthand for calling `get_operator_name`, then `decay_operator_name` on `operation`.
	// (e.g. `Operation::AddAssign` -> `operator+=` -> `operator+`)
	StringHash decay_operator_name(MetaValueOperator operation);
}