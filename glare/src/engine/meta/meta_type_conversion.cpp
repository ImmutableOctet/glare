#include "meta_type_conversion.hpp"
#include "meta_evaluation_context.hpp"

//#include <entt/meta/meta.hpp>

namespace engine
{
	MetaTypeID MetaTypeConversion::get_type_id() const
	{
		return type_id;
	}

	MetaType MetaTypeConversion::get_type() const
	{
		return resolve(get_type_id());
	}

	MetaAny MetaTypeConversion::get() const
	{
		const auto type = get_type();

		if (!type)
		{
			return {};
		}

		return type.construct();
	}

	MetaAny MetaTypeConversion::get(Registry& registry, Entity entity) const
	{
		return get();
	}

	MetaAny MetaTypeConversion::get(Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return get(registry, entity);
	}

	MetaAny MetaTypeConversion::get(const MetaAny& instance) const
	{
		if (!instance)
		{
			return {};
		}

		if (const auto instance_type = instance.type())
		{
			if (instance_type.id() == get_type_id())
			{
				/*
				// Cast optimization; disabled for now due to safety concerns.
				if (instance.owner())
				{

					// NOTE: Const-cast used here to circumvent transitive const behavior.
					// 
					// This does not override existing reference policies, but does allow
					// for 'owned' objects to be treated as-is, without requiring the
					// caller to cast or copy-construct the underlying object.
					return const_cast<MetaAny&>(instance).as_ref();
				}
				*/
				
				return MetaAny { instance };
			}
		}

		const auto type = get_type();

		if (!type)
		{
			return {};
		}

		if (auto cast_result = instance.allow_cast(type))
		{
			return cast_result;
		}

		// TODO: Look into whether unnecessary copies need to be avoided here
		// by using the manual (`MetaAny` pointer) argument interface.
		if (auto construct_from = type.construct(instance))
		{
			return construct_from;
		}

		return {};
	}

	MetaAny MetaTypeConversion::get(const MetaAny& instance, Registry& registry, Entity entity) const
	{
		return get(instance);
	}

	MetaAny MetaTypeConversion::get(const MetaAny& instance, Registry& registry, Entity entity, const MetaEvaluationContext& context) const
	{
		return get(instance, registry, entity);
	}
}