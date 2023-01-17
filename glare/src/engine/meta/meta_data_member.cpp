#include "meta_data_member.hpp"
#include "meta.hpp"

namespace engine
{
	MetaAny MetaDataMember::get(const MetaAny& instance) const
	{
		assert(instance);

		auto type = instance.type();

		if (!type)
		{
			return {};
		}

		//assert(type.id() == type_id);

		if (type.id() != type_id)
		{
			return {};
		}

		auto data_member = type.data(data_member_id);

		if (!data_member)
		{
			return {};
		}

		return data_member.get(instance);
	}

	MetaAny MetaDataMember::get(Registry& registry, Entity entity) const
	{
		using namespace entt::literals;

		auto type = resolve(type_id);
		auto get_fn = type.func("get_component"_hs);

		if (!get_fn)
		{
			return {};
		}

		auto instance_ptr = get_fn.invoke
		(
			{},
			entt::forward_as_meta(registry),
			entt::forward_as_meta(entity)
		);

		if (!instance_ptr)
		{
			return {};
		}

		const auto instance = *instance_ptr;

		if (!instance)
		{
			return {};
		}

		return get(instance);
	}
}