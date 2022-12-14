#include "meta_event_listener.hpp"

namespace engine
{
	MetaEventListener::MetaEventListener(Service* service, Flags flags, MetaType type)
		: service(service), flags(flags), type(type)
	{}

	MetaEventListener::~MetaEventListener()
	{
		unregister();
	}

	bool MetaEventListener::unregister()
	{
		using namespace entt::literals;

		if (!service)
		{
			return false;
		}

		service->unregister(*this);

		if (type)
		{
			if (auto component_disconnect_fn = type.func("disconnect_component_meta_events"_hs))
			{
				auto& registry = service->get_registry();

				auto result = component_disconnect_fn.invoke
				(
					{},

					entt::forward_as_meta(*this),
					entt::forward_as_meta(registry),
					entt::forward_as_meta(flags)
				);

				assert(result);
			}
		}

		//service = nullptr;

		return true;
	}

	bool MetaEventListener::on_connect(Service* service, const entt::meta_type& type)
	{
		//return true;
		return (!this->service); // || (this->service == service)
	}

	bool MetaEventListener::on_disconnect(Service* service, const entt::meta_type& type)
	{
		//return true;
		return (this->service == service);
	}

	void MetaEventListener::on_event(const entt::meta_type& type, entt::meta_any event_instance) {}

	void MetaEventListener::on_component_create(Registry& registry, Entity entity, const entt::meta_any& component) {}
	void MetaEventListener::on_component_update(Registry& registry, Entity entity, const entt::meta_any& component) {}
	void MetaEventListener::on_component_destroy(Registry& registry, Entity entity, const entt::meta_any& component) {}
}