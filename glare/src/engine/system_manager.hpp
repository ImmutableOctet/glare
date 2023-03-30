#pragma once

#include "meta.hpp"

#include <util/memory.hpp>

#include <engine/service.hpp>
#include <engine/world/behaviors/bridge.hpp>

//#include <vector>
#include <unordered_map>
#include <type_traits>
#include <tuple>

namespace engine
{
	class WorldSystem;

	// Handles persistent opaque storage of systems and behaviors, allowing for systems to run
	// and events to be dispatched, without the need to manage individual object lifetimes.
	template <typename ServiceType>
	class SystemManager
	{
		private:
			template <typename SystemType>
			inline static constexpr auto key() { return entt::type_hash<SystemType>::value(); }

			template <template<typename, typename, typename...> typename TraitType, typename SystemType>
			inline static constexpr bool has_method()
			{
				using Type = std::decay_t<SystemType>;

				if constexpr (TraitType<Type, bool, ServiceType&>::value)
				{
					return true;
				}
				else if constexpr (TraitType<Type, bool, Service&>::value)
				{
					return true;
				}

				//return (std::is_base_of_v<WorldSystem, SystemType> && std::is_base_of_v<World, ServiceType>);

				return false;
			}

			template <typename SystemType>
			inline static constexpr bool has_subscribe()
			{
				return has_method<engine::has_method_subscribe, SystemType>();
			}

			template <typename SystemType>
			inline static constexpr bool has_unsubscribe()
			{
				//return has_subscribe<SystemType>();
				return has_method<engine::has_method_unsubscribe, SystemType>();
			}

		public:
			//using ServiceType = ServiceType;

			using Opaque           = entt::any;  // std::any;
			using RawOpaquePointer = void*;

			using System           = entt::any; // std::any;
			using SystemID         = entt::id_type;

			using SystemCollection = std::unordered_map<SystemID, System>;
			using SystemIterator   = SystemCollection::iterator;

			inline SystemManager(ServiceType& service)
				: service(service)
			{}

			// TODO: Look into scenarios where the underlying object of `system` could be moved as well.
			// Such scenarios are usually unsafe, since the underlying object is likely subscribed to one or more event sinks.
			// 
			// Moves `system` into an internal container, associated by its `entt::type_hash`.
			// This routine does not subscribe the `system` object to `service`.
			// For general usage, see `emplace_system`.
			template <typename SystemType>
			inline System& add_system(System&& system)
			{
				// Associate `system` with the `entt::type_hash` of `SystemType`.
				return add_system(key<SystemType>(), std::move(system));
			}

			/*
				Constructs a `SystemType` object using the arguments provided, then adds the
				newly allocated object to the `systems` collection to ensure its lifetime.
				
				This function returns a reference to the allocated `SystemType` object.
				The lifetime of this object is managed internally.
			*/
			template <typename SystemType, typename...Args>
			inline SystemType& emplace_system(Args&&... args)
			{
				// Add an empty opaque `System` reference to allocate space for `SystemType`.
				auto& system = add_system<SystemType>({});

				// Emplace a new object inside of the `System` object we allocated.
				system.emplace<SystemType>(std::forward<Args>(args)...);

				// Retrieve a raw/opaque pointer to the new `SystemType` instance.
				auto raw_ptr = system.data();

				// Ensure `ptr` is valid.
				assert(raw_ptr);

				// Reinterpret `ptr` as the `SystemType` instance we created.
				auto ptr = reinterpret_cast<SystemType*>(raw_ptr);

				if constexpr (has_subscribe<SystemType>())
				{
					ptr->subscribe(service);
				}

				return *ptr;
			}

			// Destroys a system instance, unsubscribing it from `service` if possible.
			template <typename SystemType>
			inline bool destroy_system()
			{
				auto& [it, system] = get_system_ex<SystemType>();

				if (it != systems.end() && system)
				{
					if constexpr (has_unsubscribe<SystemType>())
					{
						// Guarantee that `system` unsubscribes from `service`.
						system.unsubscribe(service);
					}

					// Remove the underlying `Service` instance internally.
					systems.erase(it);

					return true;
				}

				return false;
			}

			// Retrieves an instance of `SystemType`, if one exists, as well as its iterator in the internal container.
			// If a `SystemType` object has not been registered, this will return `nullptr`.
			template <typename SystemType>
			inline std::tuple<SystemIterator, SystemType*> get_system_ex()
			{
				// Check if an instance of `SystemType` has been registered:
				auto it = systems.find(key<SystemType>());

				if (it == systems.end())
				{
					return nullptr;
				}

				auto& system = it->second;

				auto ptr = system.data();

				assert(ptr);

				if (!ptr)
				{
					return { systems.end(), nullptr};
				}

				return { std::move(it), reinterpret_cast<SystemType*>(ptr) };
			}

			// The `end` iterator of the internal `systems` container.
			inline auto end_iterator() { return systems.end(); }

			// Retrieves an instance of `SystemType`, if one exists.
			// If a `SystemType` object has not been registered, this will return `nullptr`.
			template <typename SystemType>
			inline SystemType* get_system()
			{
				return std::get<1>(get_system_ex());
			}
			
			// TODO: Look into rolling `register_behavior` and `unregister_behavior` into one routine.
			// (Maybe through an RAII-based approach...?)
			// 
			// TODO: Look into changing the registration methods into free functions,
			// so we can encapsulating them within the `behaviors` submodule.
			template <typename BehaviorType>
			inline void register_behavior()
			{
				// Check for all possible behavior event-handlers:
				if constexpr (engine::has_on_update<BehaviorType>())
				{
					service.register_free_function<engine::OnServiceUpdate, engine::behavior_impl::bridge_on_update<BehaviorType>>();
				}

				if constexpr (engine::has_on_mouse<BehaviorType>())
				{
					service.register_free_function<engine::OnMouseState, engine::behavior_impl::bridge_on_mouse<BehaviorType>>();
				}

				if constexpr (engine::has_on_keyboard<BehaviorType>())
				{
					service.register_free_function<engine::OnKeyboardState, engine::behavior_impl::bridge_on_keyboard<BehaviorType>>();
				}

				if constexpr (engine::has_on_input_update<BehaviorType>())
				{
					service.register_free_function<engine::OnInput, engine::behavior_impl::bridge_on_input_update<BehaviorType>>();
				}

				if constexpr (engine::has_on_button_down<BehaviorType>())
				{
					service.register_free_function<engine::OnButtonDown, engine::behavior_impl::bridge_on_button_down<BehaviorType>>();
				}

				if constexpr (engine::has_on_button_up<BehaviorType>())
				{
					service.register_free_function<engine::OnButtonReleased, engine::behavior_impl::bridge_on_button_up<BehaviorType>>();
				}

				if constexpr (engine::has_on_button_pressed<BehaviorType>())
				{
					service.register_free_function<engine::OnButtonPressed, engine::behavior_impl::bridge_on_button_pressed<BehaviorType>>();
				}
			}

			template <typename BehaviorType>
			inline void unregister_behavior()
			{
				// Check for all possible behavior event-handlers:
				if constexpr (engine::has_on_update<BehaviorType>())
				{
					service.unregister_free_function<engine::OnServiceUpdate, engine::behavior_impl::bridge_on_update<BehaviorType>>();
				}

				if constexpr (engine::has_on_mouse<BehaviorType>())
				{
					service.unregister_free_function<engine::OnMouseState, engine::behavior_impl::bridge_on_mouse<BehaviorType>>();
				}

				if constexpr (engine::has_on_keyboard<BehaviorType>())
				{
					service.unregister_free_function<engine::OnKeyboardState, engine::behavior_impl::bridge_on_keyboard<BehaviorType>>();
				}

				if constexpr (engine::has_on_input_update<BehaviorType>())
				{
					service.unregister_free_function<engine::OnInput, engine::behavior_impl::bridge_on_input_update<BehaviorType>>();
				}

				if constexpr (engine::has_on_button_down<BehaviorType>())
				{
					service.unregister_free_function<engine::OnButtonDown, engine::behavior_impl::bridge_on_button_down<BehaviorType>>();
				}

				if constexpr (engine::has_on_button_up<BehaviorType>())
				{
					service.unregister_free_function<engine::OnButtonReleased, engine::behavior_impl::bridge_on_button_up<BehaviorType>>();
				}

				if constexpr (engine::has_on_button_pressed<BehaviorType>())
				{
					service.unregister_free_function<engine::OnButtonPressed, engine::behavior_impl::bridge_on_button_pressed<BehaviorType>>();
				}
			}

		protected:
			/*
				Moves `system` into an internal container of active `System` objects,
				then returns a reference at the new memory location within the container.

				If the underlying object-type of `system` is known, then it is safe
				to `reinterpret_cast` back to the appropriate pointer-type from `System::data`.
			*/
			inline System& add_system(SystemID system_id, System&& system)
			{
				auto& any_instance = (systems[system_id] = std::move(system));

				return any_instance;
			}

			// The service this system-manager is linked to.
			ServiceType& service;

			// Opaque map of type-hashes to systems and their resources.
			SystemCollection systems;
	};
}