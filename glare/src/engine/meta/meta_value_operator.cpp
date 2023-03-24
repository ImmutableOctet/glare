#include "meta_value_operator.hpp"
#include "hash.hpp"

namespace engine
{
	bool is_unary_operation(MetaValueOperator operation)
	{
		switch (operation)
		{
			case MetaValueOperator::UnaryPlus:
				return true;
			case MetaValueOperator::UnaryMinus:
				return true;
			case MetaValueOperator::Boolean:
				return true;
			case MetaValueOperator::LogicalNot:
				return true;
			case MetaValueOperator::BitwiseNot:
				return true;
			case MetaValueOperator::Dereference:
				return true;
		}

		return false;
	}

	bool is_assignment_operation(MetaValueOperator operation)
	{
		switch (operation)
		{
			case MetaValueOperator::Assign:
			case MetaValueOperator::AddAssign:
			case MetaValueOperator::SubtractAssign:
			case MetaValueOperator::MultiplyAssign:
			case MetaValueOperator::DivideAssign:
			case MetaValueOperator::ModulusAssign:
			case MetaValueOperator::ShiftLeftAssign:
			case MetaValueOperator::ShiftRightAssign:
			case MetaValueOperator::BitwiseAndAssign:
			case MetaValueOperator::BitwiseXORAssign:
			case MetaValueOperator::BitwiseOrAssign:
				return true;
		}

		return false;
	}

	bool is_comparison_operation(MetaValueOperator operation)
	{
		switch (operation)
		{
			case MetaValueOperator::LessThan:
			case MetaValueOperator::LessThanOrEqual:
			case MetaValueOperator::GreaterThan:
			case MetaValueOperator::GreaterThanOrEqual:
			case MetaValueOperator::Equal:
			case MetaValueOperator::NotEqual:
				return true;
		}

		return false;
	}

	std::optional<std::tuple<MetaValueOperator, MetaOperatorPrecedence>>
	parse_value_operator(std::string_view symbol, bool symbol_is_leading, bool resolve_non_standard_symbols)
	{
		using namespace engine::literals;

		if (symbol.empty())
		{
			return std::nullopt;
		}

		const auto symbol_id = hash(symbol).value();

		if (symbol_is_leading)
		{
			// TODO: Add explicit boolean conversion symbol for `MetaValueOperator::Boolean`.
			switch (symbol_id)
			{
				case "+"_hs:
					return {{ MetaValueOperator::UnaryPlus, 3 }};
				case "-"_hs:
					return {{ MetaValueOperator::UnaryMinus, 3 }};
				case "~"_hs:
					return {{ MetaValueOperator::BitwiseNot, 3 }};
				case "!"_hs:
					return {{ MetaValueOperator::LogicalNot, 3 }};
				case "*"_hs:
					return {{ MetaValueOperator::Dereference, 3 }};
			}
		}
		else
		{
			switch (symbol_id)
			{
				case "*"_hs:
					return {{ MetaValueOperator::Multiply, 5 }};
				case "/"_hs:
					return {{ MetaValueOperator::Divide, 5 }};
				case "%"_hs:
					return {{ MetaValueOperator::Modulus, 5 }};

				case "+"_hs:
					return {{ MetaValueOperator::Add, 6 }};
				case "-"_hs:
					return {{ MetaValueOperator::Subtract, 6 }};

				case "<<"_hs:
					return {{ MetaValueOperator::ShiftLeft, 7 }};
				case ">>"_hs:
					return {{ MetaValueOperator::ShiftRight, 7 }};
				
				case "<"_hs:
					return {{ MetaValueOperator::LessThan, 9 }};
				case "<="_hs:
					return {{ MetaValueOperator::LessThanOrEqual, 9 }};
				case ">"_hs:
					return {{ MetaValueOperator::GreaterThan, 9 }};
				case ">="_hs:
					return {{ MetaValueOperator::GreaterThanOrEqual, 9 }};

				case "=="_hs:
					return {{ MetaValueOperator::Equal, 10 }};
				case "!="_hs:
				case "<>"_hs:
					return {{ MetaValueOperator::NotEqual, 10 }};

				case "&"_hs:
					return {{ MetaValueOperator::BitwiseAnd, 11 }};
				
				case "^"_hs:
					return {{ MetaValueOperator::BitwiseXOR, 12 }};
				
				case "|"_hs:
					return {{ MetaValueOperator::BitwiseOr, 13 }};

				case "&&"_hs:
					return {{ MetaValueOperator::LogicalAnd, 14 }};

				case "||"_hs:
					return {{ MetaValueOperator::LogicalOr, 15 }};

				case "="_hs:
					return {{ MetaValueOperator::Assign, 16 }};
				case "+="_hs:
					return {{ MetaValueOperator::AddAssign, 16 }};
				case "-="_hs:
					return {{ MetaValueOperator::SubtractAssign, 16 }};
				case "*="_hs:
					return {{ MetaValueOperator::MultiplyAssign, 16 }};
				case "/="_hs:
					return {{ MetaValueOperator::DivideAssign, 16 }};
				case "%="_hs:
					return {{ MetaValueOperator::ModulusAssign, 16 }};
				case "<<="_hs:
					return {{ MetaValueOperator::ShiftLeftAssign, 16 }};
				case ">>="_hs:
					return {{ MetaValueOperator::ShiftRightAssign, 16 }};
				case "&="_hs:
					return {{ MetaValueOperator::BitwiseAndAssign, 16 }};
				case "^="_hs:
					return {{ MetaValueOperator::BitwiseXORAssign, 16 }};
				case "|="_hs:
					return {{ MetaValueOperator::BitwiseOrAssign, 16 }};
			}
		}

		if (resolve_non_standard_symbols) // && !symbol_is_leading
		{
			switch (symbol_id)
			{
				case "::"_hs:
					return {{ MetaValueOperator::Get, 1 }};
				case "."_hs:
					return {{ MetaValueOperator::Get, 2 }};
				case "->"_hs:
					return {{ MetaValueOperator::Get, 2 }};

				case "["_hs:
				case "]"_hs:
				case "[]"_hs:
					return {{ MetaValueOperator::Subscript, 2 }};
			}
		}

		return std::nullopt;
	}

	MetaValueOperator decay_operation(MetaValueOperator operation)
	{
		switch (operation)
		{
			case MetaValueOperator::AddAssign:
				return MetaValueOperator::Add;

			case MetaValueOperator::SubtractAssign:
				return MetaValueOperator::Subtract;

			case MetaValueOperator::MultiplyAssign:
				return MetaValueOperator::Multiply;

			case MetaValueOperator::DivideAssign:
				return MetaValueOperator::Divide;

			case MetaValueOperator::ModulusAssign:
				return MetaValueOperator::Modulus;

			case MetaValueOperator::ShiftLeftAssign:
				return MetaValueOperator::ShiftLeft;

			case MetaValueOperator::ShiftRightAssign:
				return MetaValueOperator::ShiftRight;

			case MetaValueOperator::BitwiseAndAssign:
				return MetaValueOperator::BitwiseAnd;

			case MetaValueOperator::BitwiseXORAssign:
				return MetaValueOperator::BitwiseXOR;

			case MetaValueOperator::BitwiseOrAssign:
				return MetaValueOperator::BitwiseOr;
		}

		return operation;
	}

	StringHash get_operator_name(MetaValueOperator operation)
	{
		using namespace engine::literals;

		switch (operation)
		{
			case MetaValueOperator::Get:
				return "operator()"_hs;
			case MetaValueOperator::Subscript:
				return "operator[]"_hs;
			case MetaValueOperator::Dereference:
				return "*operator"_hs; // "*operator->"_hs;

			case MetaValueOperator::UnaryPlus:
				return "+operator"_hs;
			case MetaValueOperator::UnaryMinus:
				return "-operator"_hs;
			case MetaValueOperator::Boolean:
				return "operator bool"_hs;
			case MetaValueOperator::LogicalNot:
				return "!operator"_hs;
			case MetaValueOperator::BitwiseNot:
				return "~operator"_hs;

			case MetaValueOperator::Multiply:
				return "operator*"_hs;
			case MetaValueOperator::Divide:
				return "operator/"_hs;
			case MetaValueOperator::Modulus:
				return "operator%"_hs;
			case MetaValueOperator::Add:
				return "operator+"_hs;
			case MetaValueOperator::Subtract:
				return "operator-"_hs;

			case MetaValueOperator::ShiftLeft:
				return "operator<<"_hs;
			case MetaValueOperator::ShiftRight:
				return "operator>>"_hs;

			case MetaValueOperator::LessThan:
				return "operator<"_hs;
			case MetaValueOperator::LessThanOrEqual:
				return "operator<="_hs;
			case MetaValueOperator::GreaterThan:
				return "operator>"_hs;
			case MetaValueOperator::GreaterThanOrEqual:
				return "operator>="_hs;

			case MetaValueOperator::Equal:
				return "operator=="_hs;
			case MetaValueOperator::NotEqual:
				return "operator!="_hs;

			case MetaValueOperator::BitwiseAnd:
				return "operator&"_hs;
			case MetaValueOperator::BitwiseXOR:
				return "operator^"_hs;
			case MetaValueOperator::BitwiseOr:
				return "operator|"_hs;

			case MetaValueOperator::LogicalAnd:
				return "operator&&"_hs;
			case MetaValueOperator::LogicalOr:
				return "operator||"_hs;

			case MetaValueOperator::Assign:
				return "operator="_hs;

			case MetaValueOperator::AddAssign:
				return "operator+="_hs;
			case MetaValueOperator::SubtractAssign:
				return "operator-="_hs;
			case MetaValueOperator::MultiplyAssign:
				return "operator*="_hs;
			case MetaValueOperator::DivideAssign:
				return "operator/="_hs;
			case MetaValueOperator::ModulusAssign:
				return "operator%="_hs;
			case MetaValueOperator::ShiftLeftAssign:
				return "operator<<="_hs;
			case MetaValueOperator::ShiftRightAssign:
				return "operator>>="_hs;
			case MetaValueOperator::BitwiseAndAssign:
				return "operator&="_hs;
			case MetaValueOperator::BitwiseXORAssign:
				return "operator^="_hs;
			case MetaValueOperator::BitwiseOrAssign:
				return "operator|="_hs;
		}

		return {};
	}

	StringHash decay_operator_name(StringHash operator_name)
	{
		using namespace engine::literals;

		switch (operator_name)
		{
			case "operator+="_hs:
				return "operator+"_hs;
			case "operator-="_hs:
				return "operator-"_hs;
			case "operator*="_hs:
				return "operator*"_hs;
			case "operator/="_hs:
				return "operator/"_hs;
			case "operator%="_hs:
				return "operator%"_hs;
			case "operator<<="_hs:
				return "operator<<"_hs;
			case "operator>>="_hs:
				return "operator>>"_hs;
			case "operator&="_hs:
				return "operator&"_hs;
			case "operator^="_hs:
				return "operator^"_hs;
			case "operator|="_hs:
				return "operator|"_hs;
		}

		return operator_name;
	}

	StringHash decay_operator_name(MetaValueOperator operation)
	{
		const auto operator_name = get_operator_name(operation);

		return decay_operator_name(operator_name);
	}
}