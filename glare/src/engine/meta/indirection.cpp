#include "indirection.hpp"

#include "hash.hpp"
#include "function.hpp"
#include "indirect_meta_any.hpp"
#include "meta_evaluation_context.hpp"

namespace engine
{
	template <typename ValueType, typename ...Args>
	static MetaAny try_get_underlying_value_impl(const MetaFunction& resolution_fn, ValueType&& value, Args&&... args)
	{
		assert(value);
		//assert(resolution_fn);

		if (!resolution_fn)
		{
			return {};
		}

		auto fn = resolution_fn;

		while (fn)
		{
			// NOTE: Possible recursion.
			if (auto result = fn.invoke(value, entt::forward_as_meta<Args>(args)...))
			{
				return result;
			}

			fn = fn.next();
		}

		return {};
	}

	MetaAny try_get_underlying_value(const MetaAny& value)
	{
		using namespace engine::literals;

		if (!value)
		{
			return {};
		}

		auto type = value.type();

		auto get_fn = type.func("operator()"_hs);

		return try_get_underlying_value_impl(get_fn, value);
	}

	MetaAny try_get_underlying_value(const MetaAny& value, Registry& registry, Entity entity)
	{
		using namespace engine::literals;

		if (!value)
		{
			return {};
		}

		auto type = value.type();

		auto get_fn = type.func("operator()"_hs);

		if (auto result = try_get_underlying_value_impl(get_fn, value, registry, entity))
		{
			return result;
		}

		return try_get_underlying_value_impl(get_fn, value);
	}

	MetaAny try_get_underlying_value(const MetaAny& value, const MetaEvaluationContext& context)
	{
		using namespace engine::literals;

		if (!value)
		{
			return {};
		}

		auto type = value.type();

		auto get_fn = type.func("operator()"_hs);

		if (auto result = try_get_underlying_value_impl(get_fn, value, context))
		{
			return result;
		}

		return try_get_underlying_value_impl(get_fn, value);
	}

	MetaAny try_get_underlying_value(const MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		using namespace engine::literals;

		if (!value)
		{
			return {};
		}
		
		auto type = value.type();

		auto get_fn = type.func("operator()"_hs);

		if (auto result = try_get_underlying_value_impl(get_fn, value, registry, entity, context))
		{
			return result;
		}

		if (auto result = try_get_underlying_value_impl(get_fn, value, registry, entity))
		{
			return result;
		}

		if (auto result = try_get_underlying_value_impl(get_fn, value, context))
		{
			return result;
		}
		
		return try_get_underlying_value_impl(get_fn, value);
	}

	MetaType try_get_underlying_type(const MetaType& type)
	{
		if (!type)
		{
			return {};
		}

		using namespace engine::literals;

		if (type.prop("optional"_hs))
		{
			if (auto underlying_type_fn = type.func("type_from_optional"_hs))
			{
				if (auto underlying_type_opaque = underlying_type_fn.invoke({}))
				{
					if (auto underlying_type_raw = underlying_type_opaque.try_cast<MetaType>())
					{
						return get_underlying_or_direct_type(*underlying_type_raw);
					}
				}
			}
		}

		return {};
	}

	MetaType try_get_underlying_type(const MetaAny& type_reference_value)
	{
		using namespace engine::literals;

		if (!type_reference_value)
		{
			return {};
		}
		
		const auto reference_type = type_reference_value.type();

		if (auto type_fn = reference_type.func("try_get_underlying_type"_hs))
		{
			if (auto result = invoke_any_overload(type_fn, type_reference_value))
			{
				if (auto result_as_type = result.try_cast<MetaType>())
				{
					return *result_as_type; // std::move(*result_as_type);
				}
				
				if (auto result_as_type_id = result.try_cast<MetaTypeID>())
				{
					if (auto result_as_type = resolve(*result_as_type_id))
					{
						return result_as_type;
					}
				}
			}
		}

		return {};
	}

	MetaType get_underlying_or_direct_type(const MetaType& type)
	{
		if (auto underlying_type = try_get_underlying_type(type))
		{
			return underlying_type;
		}

		return type;
	}

	MetaType get_underlying_or_direct_type(const MetaAny& direct_or_indirect_value)
	{
		if (!direct_or_indirect_value)
		{
			return {};
		}

		if (auto type = try_get_underlying_type(direct_or_indirect_value))
		{
			return type;
		}

		if (!value_has_indirection(direct_or_indirect_value))
		{
			return direct_or_indirect_value.type();
		}

		return {};
	}
}