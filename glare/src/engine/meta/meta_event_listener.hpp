#pragma once

//#include "meta.hpp"

#include <engine/types.hpp>
#include <engine/service.hpp>

#include <util/small_vector.hpp>

//#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>

#include <optional>

namespace engine
{
	//class Service;

	struct MetaEventListenerFlags
	{
		bool events                : 1 = true;
		bool component_creation    : 1 = true;
		bool component_update      : 1 = true;
		bool component_destruction : 1 = true;
	};

	// Abstract base-type used to build opaque event listeners via `entt::meta_any`.
	class MetaEventListener
	{
		public:
			using Flags = MetaEventListenerFlags;

			// Manually connect component listeners in `registry`.
			template <typename EventType>
			static void connect_component_listeners(MetaEventListener& listener, Registry& registry, std::optional<Flags> flags=std::nullopt)
			{
				if (!flags)
				{
					flags = listener.flags;
				}

				if (flags->component_creation)
				{
					registry.on_construct<EventType>().connect<&MetaEventListener::component_creation_callback<EventType>>(listener);
				}

				if (flags->component_update)
				{
					registry.on_update<EventType>().connect<&MetaEventListener::component_update_callback<EventType>>(listener);
				}

				if (flags->component_destruction)
				{
					registry.on_destroy<EventType>().connect<&MetaEventListener::component_destruction_callback<EventType>>(listener);
				}
			}

			// Manually disconnect component listeners from `registry`.
			template <typename EventType>
			static void disconnect_component_listeners(MetaEventListener& listener, Registry& registry, std::optional<Flags> flags=std::nullopt)
			{
				if (!flags)
				{
					flags = listener.flags;
				}

				if (flags->component_creation)
				{
					registry.on_construct<EventType>().disconnect<&MetaEventListener::component_creation_callback<EventType>>(listener);
				}

				if (flags->component_update)
				{
					registry.on_update<EventType>().disconnect<&MetaEventListener::component_update_callback<EventType>>(listener);
				}

				if (flags->component_destruction)
				{
					registry.on_destroy<EventType>().disconnect<&MetaEventListener::component_destruction_callback<EventType>>(listener);
				}
			}

			template <typename EventType>
			static void connect(MetaEventListener& listener, Service* service, std::optional<Flags> flags=std::nullopt) // const MetaEventListener&
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

						if (!flags)
						{
							flags = listener.flags;
						}

						if (flags->events)
						{
							service->register_event<EventType, &MetaEventListener::event_callback<EventType>>(listener);
						}

						auto& registry = service->get_registry();

						connect_component_listeners<EventType>(listener, registry, flags);
					}
				}
			}

			template <typename EventType>
			static void disconnect(MetaEventListener& listener, Service* service, std::optional<Flags> flags=std::nullopt) // const MetaEventListener&
			{
				if (!service) // (!listener.service)
				{
					return;
				}

				/*
				if (listener.type)
				{
					return;
				}
				*/

				auto type = entt::resolve<EventType>();

				if (type)
				{
					if (listener.on_disconnect(service, type))
					{
						if (!flags)
						{
							flags = listener.flags;
						}

						if (flags->events)
						{
							service->unregister_event<EventType, &MetaEventListener::event_callback<EventType>>(listener);
						}

						auto& registry = service->get_registry();

						disconnect_component_listeners<EventType>(listener, registry, flags);

						if (service == listener.service)
						{
							listener.service = nullptr;
						}

						if (!listener.type)
						{
							listener.type = type;
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

			inline Flags get_flags() const
			{
				return flags;
			}
		protected:
			Service* service = nullptr;
			MetaType type;
			Flags flags;

			MetaEventListener(Service* service=nullptr, Flags flags={}, MetaType type={});

			template <typename EventType>
			void event_callback(const EventType& event_instance)
			{
				auto type = entt::resolve<EventType>();

				if (type)
				{
					on_event(type, entt::forward_as_meta(event_instance));
				}
			}

			template <typename ComponentType, typename Callback>
			bool component_event_callback(Registry& registry, Entity entity, Callback&& callback)
			{
				const auto* component = registry.try_get<ComponentType>(entity);

				if (!component)
				{
					return false;
				}

				callback(registry, entity, entt::forward_as_meta(*component));

				return true;
			}

			template <typename ComponentType>
			void component_creation_callback(Registry& registry, Entity entity)
			{
				component_event_callback<ComponentType>
				(
					registry, entity,

					[this](Registry& registry, Entity entity, const entt::meta_any& component)
					{
						on_component_create(registry, entity, component);
					}
				);
			}

			template <typename ComponentType>
			void component_destruction_callback(Registry& registry, Entity entity)
			{
				component_event_callback<ComponentType>
				(
					registry, entity,

					[this](Registry& registry, Entity entity, const entt::meta_any& component)
					{
						on_component_destroy(registry, entity, component);
					}
				);
			}

			template <typename ComponentType>
			void component_update_callback(Registry& registry, Entity entity)
			{
				component_event_callback<ComponentType>
				(
					registry, entity,

					[this](Registry& registry, Entity entity, const entt::meta_any& component)
					{
						on_component_update(registry, entity, component);
					}
				);
			}

			virtual bool on_connect(Service* service, const entt::meta_type& type);
			virtual bool on_disconnect(Service* service, const entt::meta_type& type);

			virtual void on_event(const entt::meta_type& type, entt::meta_any event_instance);

			virtual void on_component_create(Registry& registry, Entity entity, const entt::meta_any& component);
			virtual void on_component_update(Registry& registry, Entity entity, const entt::meta_any& component);
			virtual void on_component_destroy(Registry& registry, Entity entity, const entt::meta_any& component);
		private:
			bool unregister();
	};
}