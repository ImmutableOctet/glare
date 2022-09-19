#pragma once

#include <util/memory.hpp>

#include <engine/service.hpp>
#include <engine/world/behaviors/meta.hpp>

//#include <vector>
#include <unordered_map>

namespace engine
{
	// Handles persistent opaque storage of systems and behaviors, allowing for systems to run
	// and events to be dispatched, without the need to manage individual object lifetimes.
	class SystemManager
	{
		public:
			using Opaque           = entt::any;  // std::any;
			using RawOpaquePointer = void*;

			using System           = entt::any; // std::any;
			using SystemID         = entt::id_type;

			SystemManager(Service& service);

			// TODO: Look into scenarios where the underlying object of `system` could be moved as well.
			// Such scenarios are usually unsafe, since the underlying object is likely subscribed to one or more event sinks.
			// 
			// Moves `system` into an internal container, associated by its `entt::type_hash`.
			template <typename SystemType>
			inline System& add_system(System&& system)
			{
				// Associate `system` with the `entt::type_hash` of `SystemType`.
				return add_system(entt::type_hash<SystemType>::value(), std::move(system));
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
				auto ptr = system.data();

				// Ensure `ptr` is valid.
				assert(ptr);

				// Reinterpret `ptr` as the `SystemType` instance we created.
				return *reinterpret_cast<SystemType*>(ptr);
			}

			// Retrieves an instance of `SystemType`, if one exists.
			// If a `SystemType` object has not been registered, this will return `nullptr`.
			template <typename SystemType>
			inline SystemType* get_system()
			{
				// Check if an instance of `SystemType` has been registered:
				auto it = systems.find(entt::type_hash<SystemType>::value());

				if (it == systems.end())
				{
					return nullptr;
				}

				auto& system = it->second;

				auto ptr = system.data();

				assert(ptr);

				if (!ptr)
				{
					return nullptr;
				}

				return reinterpret_cast<SystemType*>(ptr);
			}
			
			// TODO: Look into rolling `register_behavior` and `unregister_behavior` into one routine.
			// (Maybe through an RAII-based approach...?)
			template <typename BehaviorType>
			inline void register_behavior()
			{
				// Check for all possible behavior event-handlers:
				if constexpr (engine::HAS_STATIC_MEMBER_FUNCTION(BehaviorType, on_update))
				{
					service.register_free_function<engine::OnServiceUpdate, engine::behavior_impl::bridge_on_update<BehaviorType>>();
				}

				if constexpr (engine::HAS_STATIC_MEMBER_FUNCTION(BehaviorType, on_mouse))
				{
					service.register_free_function<engine::OnMouseState, engine::behavior_impl::bridge_on_mouse<BehaviorType>>();
				}

				if constexpr (engine::HAS_STATIC_MEMBER_FUNCTION(BehaviorType, on_keyboard))
				{
					service.register_free_function<engine::OnKeyboardState, engine::behavior_impl::bridge_on_keyboard<BehaviorType>>();
				}
			}

			template <typename BehaviorType>
			inline void unregister_behavior()
			{
				// Check for all possible behavior event-handlers:
				if constexpr (engine::HAS_STATIC_MEMBER_FUNCTION(BehaviorType, on_update))
				{
					service.unregister_free_function<engine::OnServiceUpdate, engine::behavior_impl::bridge_on_update<BehaviorType>>();
				}

				if constexpr (engine::HAS_STATIC_MEMBER_FUNCTION(BehaviorType, on_mouse))
				{
					service.unregister_free_function<engine::OnMouseState, engine::behavior_impl::bridge_on_mouse<BehaviorType>>();
				}

				if constexpr (engine::HAS_STATIC_MEMBER_FUNCTION(BehaviorType, on_keyboard))
				{
					service.unregister_free_function<engine::OnKeyboardState, engine::behavior_impl::bridge_on_keyboard<BehaviorType>>();
				}
			}

		protected:
			/*
				Moves `system` into an internal container of active `System` objects,
				then returns a reference at the new memory location within the container.

				If the underlying object-type of `system` is known, then it is safe
				to `reinterpret_cast` back to the appropriate pointer-type from `System::data`.
			*/
			System& add_system(SystemID system_id, System&& system);

			// The service this system-manager is linked to.
			Service& service;

			// Opaque map of type-hashes to systems and their resources.
			std::unordered_map<SystemID, System> systems;
	};
}