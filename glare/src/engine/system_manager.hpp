#pragma once

#include <util/memory.hpp>

#include <engine/service.hpp>
#include <engine/world/behaviors/meta.hpp>

#include <vector>

namespace engine
{
	// Handles persistent opaque storage of systems and behaviors, allowing for systems to run
	// and events to be dispatched, without the need to manage individual object lifetimes.
	class SystemManager
	{
		public:
			using Opaque = memory::opaque_unique_ptr; // entt::any;
			using RawOpaquePointer = void*; // entt::any;

			// TODO: Look into whether we should replace `opaque_unique_ptr` with `std::any` or `entt::any`.
			// (Has storage and copy/move assignment implications)
			// Represents an automatically managed unique allocation of any type.
			using System = memory::opaque_unique_ptr;

			SystemManager(Service& service);

			/*
				Adds `system` to an internal list of active `System` objects,
				then returns a (void) pointer to the underlying opaque object.
				
				If the underlying object-type of `system` is known, then it is safe
				to `reinterpret_cast` this to the appropriate pointer-type.
			*/
			RawOpaquePointer add_system(System&& system);

			/*
				Constructs a `SystemType` object using the arguments provided, then adds the
				newly allocated object to the `systems` collection to ensure its lifetime.
				
				This function returns a reference to the allocated `SystemType` object.
				The lifetime of this object is managed internally.
			*/
			template <typename SystemType, typename...Args>
			inline SystemType& emplace_system(Args&&... args)
			{
				auto ptr = add_system(memory::make_opaque<SystemType>(std::forward<Args>(args)...));

				assert(ptr);

				return *reinterpret_cast<SystemType*>(ptr);
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
			Service& service;

			// Opaque vector of smart pointers to systems and their resources.
			std::vector<System> systems;
	};
}