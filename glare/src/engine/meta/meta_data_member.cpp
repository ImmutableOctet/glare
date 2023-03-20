#include "meta_data_member.hpp"
#include "meta.hpp"

namespace engine
{
	MetaAny MetaDataMember::get_instance(Registry& registry, Entity entity) const
	{
		using namespace engine::literals;

		auto type = resolve(type_id);

		if (!type)
		{
			return {};
		}

		auto get_fn = type.func("get_component"_hs);

		if (!get_fn)
		{
			return {};
		}

		auto instance_ptr = MetaAny {};

		do
		{
			instance_ptr = get_fn.invoke
			(
				{},
				entt::forward_as_meta(registry),
				entt::forward_as_meta(entity)
			);

			if (instance_ptr)
			{
				break;
			}

			get_fn = get_fn.next();
		} while (get_fn);

		if (!instance_ptr)
		{
			return {};
		}

		auto instance = *instance_ptr;

		if (!instance)
		{
			return {};
		}

		return instance;
	}

	MetaAny MetaDataMember::get(const MetaAny& instance) const
	{
		if (!instance)
		{
			return {};
		}

		auto type = instance.type();

		if (!type)
		{
			return {};
		}

		if (this->type_id)
		{
			if (type.id() != this->type_id)
			{
				return {};
			}
		}

		auto data_member = resolve_data_member_by_id(type, true, data_member_id);

		if (!data_member)
		{
			return {};
		}

		return data_member.get(instance);
	}

	MetaAny MetaDataMember::get(Registry& registry, Entity entity) const
	{
		auto instance = get_instance(registry, entity);

		return get(instance.as_ref());
	}

	MetaAny MetaDataMember::get(Entity target, Registry& registry, Entity context_entity) const
	{
		if (auto result = get(registry, target))
		{
			return result;
		}

		//return get(registry, context_entity);
		return {};
	}

	MetaAny MetaDataMember::get(const MetaAny& instance, Registry& registry, Entity context_entity) const
	{
		if (!instance)
		{
			return {};
		}

		if (auto result = get(instance))
		{
			return result;
		}

		if (this->type_id)
		{
			const auto type = instance.type();

			if (type.id() != this->type_id)
			{
				switch (type.id())
				{
					case entt::type_hash<Entity>::value(): // resolve<Entity>().id():
					{
						if (const auto* as_entity = instance.try_cast<Entity>())
						{
							if (auto component = get_instance(registry, *as_entity))
							{
								// NOTE: Recursion.
								return get(component, registry, context_entity);
							}
						}

						//return {};

						break;
					}
				}
			}
		}

		return {}; // get(context_entity, registry, context_entity);
	}

	MetaAny MetaDataMember::get(const MetaAny& instance, Registry& registry, Entity context_entity, const MetaEvaluationContext& context) const
	{
		return get(instance, registry, context_entity);
	}

	MetaAny MetaDataMember::set(MetaAny& source, MetaAny& destination)
	{
		if (!destination)
		{
			//return entt::forward_as_meta(*this);
			return {};
		}

		if (!source)
		{
			//return destination.as_ref();
			//return entt::forward_as_meta(*this);
			return {};
		}

		auto type = destination.type();

		if (!type)
		{
			//return destination.as_ref();
			//return entt::forward_as_meta(*this);
			return {};
		}

		//assert(type.id() == type_id);

		if (type.id() != type_id)
		{
			//return destination.as_ref();
			//return entt::forward_as_meta(*this);
			return {};
		}

		if (auto data_member = resolve_data_member_by_id(type, true, data_member_id))
		{
			if (data_member.set(destination, source)) // std::move(source)
			{
				//return destination.as_ref();
				return entt::forward_as_meta(*this);
			}
		}

		return {};
	}
	
	MetaAny MetaDataMember::set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set(source, destination, registry, entity);
	}
	
	MetaAny MetaDataMember::set(MetaAny& source, MetaAny& destination, Registry& registry, Entity entity)
	{
		return set(source, destination);
	}

	MetaAny MetaDataMember::set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set(value, registry, entity);
	}
	
	MetaAny MetaDataMember::set(MetaAny& value, Registry& registry, Entity entity)
	{
		const auto type = value.type();

		if (type.id() == entt::type_hash<Entity>::value())
		{
			return {};
		}

		if (auto instance = get_instance(registry, entity))
		{
			return set(value, instance);
		}

		return {};
	}

	bool MetaDataMember::has_member() const
	{
		return static_cast<bool>(data_member_id);
	}

	entt::meta_data MetaDataMember::get_data() const
	{
		if (has_member())
		{
			if (auto type = get_type())
			{
				return type.data(data_member_id);
			}
		}

		return {};
	}

	bool MetaDataMember::has_type() const
	{
		return static_cast<bool>(type_id);
	}

	MetaType MetaDataMember::get_type() const
	{
		if (has_type())
		{
			return resolve(type_id);
		}

		return {};
	}

	MetaType MetaDataMember::get_member_type() const
	{
		if (auto data = get_data())
		{
			return data.type();
		}

		return {};
	}

	bool MetaDataMember::has_member_type() const
	{
		//return static_cast<bool>(get_member_type());

		return (has_type() && has_member());
	}
}