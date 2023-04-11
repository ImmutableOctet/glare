#pragma once

#include "system_manager_interface.hpp"

#include <engine/service.hpp>
#include <engine/world/behaviors/bridge.hpp>

namespace engine
{
	class WorldSystem;

	// Handles persistent opaque storage of systems and behaviors, allowing for systems to run
	// and events to be dispatched, without the need to manage individual object lifetimes.
	template <typename ServiceType>
	class SystemManager : public SystemManagerInterface
	{
		public:
			//using ServiceType = ServiceType;

			inline SystemManager(ServiceType& service)
				: service(service)
			{
				systems.reserve(32); // 64
			}

			SystemManager(const SystemManager&) = delete;
			SystemManager(SystemManager&&) noexcept = delete;

			SystemManager& operator=(const SystemManager&) = delete;
			SystemManager& operator=(SystemManager&&) noexcept = delete;

			/*
				Constructs a `SystemType` object using the arguments provided, then adds the
				newly allocated object to the `systems` collection to ensure its lifetime.
				
				This function returns a reference to the allocated `SystemType` object.
				The lifetime of this object is managed internally.
			*/
			template <typename SystemType, typename...Args>
			SystemType& emplace_system(Args&&... args)
			{
				auto& system = SystemManagerInterface::emplace_system<SystemType, Args...>(std::forward<Args>(args)...);

				if constexpr (has_subscribe<SystemType, ServiceType>())
				{
					system.subscribe(service);
				}

				return system;
			}

			// Destroys a system instance, unsubscribing it from `service` if possible.
			template <typename SystemType>
			bool destroy_system()
			{
				return SystemManagerInterface::destroy_system<SystemType, ServiceType>(service);
			}

			// TODO: Look into rolling `register_behavior` and `unregister_behavior` into one routine.
			// (Maybe through an RAII-based approach...?)
			// 
			// TODO: Look into changing the registration methods into free functions,
			// so we can encapsulating them within the `behaviors` submodule.
			template <typename BehaviorType>
			void register_behavior()
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
			void unregister_behavior()
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
			// The service this system-manager is linked to.
			ServiceType& service;
	};
}