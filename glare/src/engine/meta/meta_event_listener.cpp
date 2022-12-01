#include "meta_event_listener.hpp"

namespace engine
{
	MetaEventListener::MetaEventListener(Service* service)
		: service(service)
	{}

	MetaEventListener::~MetaEventListener()
	{
		unregister();
	}

	bool MetaEventListener::unregister()
	{
		if (!service)
		{
			return false;
		}

		service->unregister(*this);

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
}