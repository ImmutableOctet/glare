#pragma once

#include "types.hpp"
//#include <engine/types.hpp>

#include <engine/service.hpp>

#include <util/small_vector.hpp>

//#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>

#include <optional>

namespace engine
{
	class Service;
	class SystemManagerInterface;

	struct MetaEventListenerFlags
	{
		bool events                : 1 = true;
		bool component_creation    : 1 = true;
		bool component_update      : 1 = true;
		bool component_destruction : 1 = true;
	};

	// Abstract base-type used to build opaque event listeners via `MetaAny`.
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
			static void connect
			(
				MetaEventListener& listener,
				Service* service,
				SystemManagerInterface* system_manager=nullptr,
				std::optional<Flags> flags=std::nullopt
			)
			{
				if (!service)
				{
					return;
				}

				auto type = entt::resolve<EventType>();

				if (type)
				{
					if (listener.on_connect(service, type))
					{
						if (!listener.service)
						{
							listener.service = service;
						}

						if (!listener.system_manager)
						{
							listener.system_manager = system_manager;
						}

						if (!listener.type_id)
						{
							listener.type_id = type.id();
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
			static void disconnect
			(
				MetaEventListener& listener,
				Service* service,
				SystemManagerInterface* system_manager=nullptr,
				std::optional<Flags> flags=std::nullopt
			) // const MetaEventListener&
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

						if ((!system_manager) || (system_manager == listener.system_manager))
						{
							listener.system_manager = nullptr;
						}

						listener.type_id = {};
					}
				}
			}

			template <typename EventType>
			static void disconnect_existing(MetaEventListener& listener) // const MetaEventListener&
			{
				disconnect_from<EventType>(listener, listener.service, listener.system_manager);
			}

			virtual ~MetaEventListener();

			inline bool is_active() const
			{
				return static_cast<bool>(service);
			}

			inline bool has_system_manager() const
			{
				return static_cast<bool>(system_manager);
			}

			inline Flags get_flags() const
			{
				return flags;
			}
		protected:
			// Weak pointer to the service owning this event listener.
			Service* service = nullptr;

			// Optional weak pointer to the system manager tied to `service`.
			SystemManagerInterface* system_manager = nullptr;
			
			// The ID of the type this event is listening for, if any.
			MetaTypeID type_id = {};

			// Flags indicating which events are to be listened for.
			Flags flags;

			MetaEventListener
			(
				Service* service, SystemManagerInterface* system_manager,
				Flags flags, MetaType type
			);
			
			MetaEventListener
			(
				Service* service=nullptr, SystemManagerInterface* system_manager=nullptr,
				Flags flags={}, MetaTypeID type_id={}
			);

			MetaType type() const;

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

					[this](Registry& registry, Entity entity, const MetaAny& component)
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

					[this](Registry& registry, Entity entity, const MetaAny& component)
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

					[this](Registry& registry, Entity entity, const MetaAny& component)
					{
						on_component_update(registry, entity, component);
					}
				);
			}

			virtual bool on_connect(Service* service, const MetaType& type);
			virtual bool on_disconnect(Service* service, const MetaType& type);

			virtual void on_event(const MetaType& type, MetaAny event_instance);

			virtual void on_component_create(Registry& registry, Entity entity, const MetaAny& component);
			virtual void on_component_update(Registry& registry, Entity entity, const MetaAny& component);
			virtual void on_component_destroy(Registry& registry, Entity entity, const MetaAny& component);
		private:
			bool unregister();
	};
}