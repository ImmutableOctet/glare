#include "indirection.hpp"
#include "indirect_meta_any.hpp"
#include "meta_evaluation_context.hpp"

//#include "meta.hpp"

namespace engine
{
	bool value_has_indirection(const MetaAny& value, bool bypass_indirect_meta_any)
	{
		using namespace engine::literals;

		if (!value)
		{
			return {};
		}

		auto type = value.type();

		if (bypass_indirect_meta_any)
		{
			// TODO: Look into adding support for `MetaValueOperation` as well. (May need to be a shallow check)
			if (type.id() == "IndirectMetaAny"_hs)
			{
				if (const auto* as_indirect = value.try_cast<IndirectMetaAny>())
				{
					return type_has_indirection(as_indirect->get_type());
				}
			}
		}

		return type_has_indirection(type);
	}

	bool type_has_indirection(const MetaType& type)
	{
		using namespace engine::literals;

		if (!type)
		{
			return false;
		}

		if (type.func("operator()"_hs))
		{
			return true;
		}

		if (type.func("operator="_hs))
		{
			return true;
		}

		return false;
	}

	bool type_has_indirection(MetaTypeID type_id)
	{
		auto type = resolve(type_id);

		if (!type)
		{
			return false;
		}

		return type_has_indirection(type);
	}

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
}