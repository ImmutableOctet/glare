#include "function.hpp"

namespace engine
{
	bool function_overload_has_meta_any_argument(const MetaFunction& function)
	{
		if (!function)
		{
			return false;
		}

		for (std::size_t i = 0; i < function.arity(); i++)
		{
			const auto arg_type = function.arg(i);

			if (arg_type.id() == entt::type_hash<MetaAny>::value()) // resolve<MetaAny>().id()
			{
				return true;
			}
		}

		return false;
	}

	bool function_has_meta_any_argument(const MetaFunction& function, bool check_overloads)
	{
		if (!check_overloads)
		{
			return function_overload_has_meta_any_argument(function);
		}

		if (!function)
		{
			return false;
		}

		auto target_overload = function;

		do
		{
			if (function_overload_has_meta_any_argument(target_overload))
			{
				return true;
			}

			target_overload = target_overload.next();
		} while (target_overload);

		return false;
	}

	bool function_has_static_overload(const MetaFunction& function_overloads)
	{
		if (!function_overloads)
		{
			return false;
		}

		auto target_overload = function_overloads;

		do
		{
			if (target_overload.is_static())
			{
				return true;
			}

			target_overload = target_overload.next();
		} while (target_overload);

		return false;
	}

	bool function_has_const_overload(const MetaFunction& function_overloads)
	{
		if (!function_overloads)
		{
			return false;
		}

		auto target_overload = function_overloads;

		do
		{
			if (target_overload.is_const())
			{
				return true;
			}

			target_overload = target_overload.next();
		} while (target_overload);

		return false;
	}
}