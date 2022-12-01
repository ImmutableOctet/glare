#pragma once

//#include "meta.hpp"

#include <engine/types.hpp>
#include <engine/service.hpp>

#include <util/small_vector.hpp>

//#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>

namespace engine
{
	//class Service;

	// Abstract base-type used to build opaque event listeners via `entt::meta_any`.
	class MetaEventListener
	{
		public:
			template <typename EventType>
			static void connect(MetaEventListener& listener, Service* service) // const MetaEventListener&
			{
				if (!service)
				{
					return;
				}

				// Debugging related:
				//EventType* _test = nullptr;

				auto type = entt::resolve<EventType>();

				if (type)
				{
					if (listener.on_connect(service, type))
					{
						if (!listener.service)
						{
							listener.service = service;
						}

						service->register_event<EventType, &MetaEventListener::event_callback<EventType>>(listener);
					}
				}
			}

			template <typename EventType>
			static void disconnect(MetaEventListener& listener, Service* service) // const MetaEventListener&
			{
				if (!service) // (!listener.service)
				{
					return;
				}

				auto type = entt::resolve<EventType>();

				if (type)
				{
					if (listener.on_disconnect(service, type))
					{
						service->unregister_event<EventType, &MetaEventListener::event_callback<EventType>>(listener);

						if (service == listener.service)
						{
							listener.service = nullptr;
						}
					}
				}
			}

			template <typename EventType>
			static void disconnect_existing(MetaEventListener& listener) // const MetaEventListener&
			{
				disconnect_from<EventType>(listener, listener.service);
			}

			virtual ~MetaEventListener();

			inline bool is_active() const
			{
				return static_cast<bool>(service);
			}
		protected:
			MetaEventListener(Service* service=nullptr);

			template <typename EventType>
			void event_callback(const EventType& event_instance)
			{
				auto type = entt::resolve<EventType>();

				if (type)
				{
					on_event(type, entt::forward_as_meta(event_instance));
				}
			}

			Service* service = nullptr;

			virtual bool on_connect(Service* service, const entt::meta_type& type);
			virtual bool on_disconnect(Service* service, const entt::meta_type& type);

			virtual void on_event(const entt::meta_type& type, entt::meta_any event_instance);
		private:
			bool unregister();
	};
}