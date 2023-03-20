#include "meta_event_listener.hpp"

#include "meta.hpp"

namespace engine
{
	MetaEventListener::MetaEventListener(Service* service, Flags flags, MetaTypeID type_id)
		: service(service), flags(flags), type_id(type_id)
	{}

	MetaEventListener::MetaEventListener(Service* service, Flags flags, MetaType type)
		: MetaEventListener(service, flags, type.id())
	{}

	MetaEventListener::~MetaEventListener()
	{
		unregister();
	}

	bool MetaEventListener::unregister()
	{
		using namespace engine::literals;

		if (!service)
		{
			return false;
		}

		service->unregister(*this);

		if (auto type = this->type())
		{
			if (auto component_disconnect_fn = type.func("disconnect_component_meta_events"_hs))
			{
				auto& registry = service->get_registry();

				auto result = component_disconnect_fn.invoke
				(
					{},

					entt::forward_as_meta(*this),
					entt::forward_as_meta(registry),
					entt::forward_as_meta(std::optional<Flags>(std::nullopt))
				);

				assert(result);
			}
		}

		//service = nullptr;

		return true;
	}

	bool MetaEventListener::on_connect(Service* service, const MetaType& type)
	{
		//return true;
		return (!this->service); // || (this->service == service)
	}

	bool MetaEventListener::on_disconnect(Service* service, const MetaType& type)
	{
		//return true;
		return (this->service == service);
	}

	MetaType MetaEventListener::type() const
	{
		return resolve(type_id);
	}

	void MetaEventListener::on_event(const MetaType& type, MetaAny event_instance) {}

	void MetaEventListener::on_component_create(Registry& registry, Entity entity, const MetaAny& component) {}
	void MetaEventListener::on_component_update(Registry& registry, Entity entity, const MetaAny& component) {}
	void MetaEventListener::on_component_destroy(Registry& registry, Entity entity, const MetaAny& component) {}
}