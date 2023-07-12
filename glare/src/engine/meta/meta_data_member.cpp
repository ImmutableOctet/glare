#include "meta_data_member.hpp"

#include "hash.hpp"
#include "data_member.hpp"
#include "indirection.hpp"
//#include "function.hpp"
#include "component.hpp"

namespace engine
{
	MetaAny MetaDataMember::get_instance(const MetaType& type, Registry& registry, Entity entity)
	{
		return get_component_ref(registry, entity, type);
	}

	MetaAny MetaDataMember::get_instance(Registry& registry, Entity entity) const
	{
		return get_instance(get_type(), registry, entity);
	}

	MetaAny MetaDataMember::get(const MetaAny& instance) const
	{
		if (!instance)
		{
			return {};
		}

		auto instance_type = instance.type();

		if (!instance_type)
		{
			return {};
		}

		if (this->type_id)
		{
			const auto instance_type_id = instance_type.id();

			if (instance_type_id != this->type_id)
			{
				/*
				// Disabled for now; should be handled prior to calling this routine.
				if (auto resolved_intended_type = get_underlying_or_direct_type(this->get_type()))
				{
					if (instance_type_id != resolved_intended_type.id())
					{
						return {};
					}
				}
				else
				{
					return {};
				}
				*/

				return {};
			}
		}

		auto data_member = resolve_data_member_by_id(instance_type, true, data_member_id);

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
		using namespace engine::literals;

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
					case "Entity"_hs: // entt::type_hash<Entity>::value(): // resolve<Entity>().id():
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

		if (this->type_id)
		{
			//assert(type.id() == this->type_id);

			if (type.id() != this->type_id)
			{
				//return destination.as_ref();
				//return entt::forward_as_meta(*this);
				return {};
			}
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
		using namespace engine::literals;

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

		auto destination_type = destination.type();

		if (!destination_type)
		{
			//return destination.as_ref();
			//return entt::forward_as_meta(*this);
			return {};
		}

		if (this->type_id)
		{
			if (destination_type.id() != this->type_id)
			{
				switch (destination_type.id())
				{
					case "Entity"_hs: // entt::type_hash<Entity>::value(): // resolve<Entity>().id():
					{
						if (const auto* as_entity = destination.try_cast<Entity>())
						{
							if (auto component = get_or_emplace_component(registry, *as_entity, get_type())) // get_instance(registry, *as_entity)
							{
								return set(source, component);
							}
						}

						//return {};

						break;
					}
				}
			}
		}

		return set(source, destination);
	}

	MetaAny MetaDataMember::set(MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context)
	{
		return set(value, registry, entity);
	}
	
	MetaAny MetaDataMember::set(MetaAny& value, Registry& registry, Entity entity)
	{
		using namespace engine::literals;

		const auto type = value.type();

		if (type.id() == "Entity"_hs) // entt::type_hash<Entity>::value()
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
				return resolve_data_member_by_id(type, true, data_member_id);
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