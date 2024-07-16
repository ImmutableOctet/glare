#pragma once

#include <engine/system_manager_interface.hpp>

#include <engine/meta/meta_evaluation_context.hpp>

#include <cassert>

namespace engine
{
	struct MetaEvaluationContext;

	class Service;
	class SystemManagerInterface;
	
	class ContextualInterface
	{
		public:
			ContextualInterface() = default;

			ContextualInterface(const MetaEvaluationContext& context) :
				_context(&context)
			{}

			ContextualInterface(const ContextualInterface&) = delete;
			ContextualInterface(ContextualInterface&&) noexcept = default;

			ContextualInterface& operator=(const ContextualInterface&) = delete;
			ContextualInterface& operator=(ContextualInterface&&) noexcept = default;

			[[nodiscard]] const MetaEvaluationContext& get_context() const
			{
				assert(_context);

				return *_context;
			}

			[[nodiscard]] const Service& get_service() const
			{
				auto& context = get_context();

				assert(context.service);

				return *context.service;
			}

			[[nodiscard]] Service& get_service()
			{
				auto& context = get_context();

				assert(context.service);

				return *context.service;
			}

			[[nodiscard]] SystemManagerInterface& get_system_manager()
			{
				auto& context = get_context();

				assert(context.system_manager);

				return *context.system_manager;
			}

			[[nodiscard]] const SystemManagerInterface& get_system_manager() const
			{
				auto& context = get_context();

				assert(context.system_manager);

				return *context.system_manager;
			}

			template <typename SystemType>
			[[nodiscard]] SystemType* try_get_system()
			{
				auto& system_manager = get_system_manager();

				return system_manager.get_system<SystemType>();
			}

			template <typename SystemType>
			[[nodiscard]] const SystemType* try_get_system() const
			{
				auto& system_manager = get_system_manager();

				return system_manager.get_system<SystemType>();
			}

			template <typename SystemType>
			[[nodiscard]] SystemType& system()
			{
				auto system_instance = try_get_system<SystemType>();

				assert(system_instance);

				return *system_instance;
			}

			template <typename SystemType>
			[[nodiscard]] const SystemType& system() const
			{
				auto system_instance = try_get_system<SystemType>();

				assert(system_instance);

				return *system_instance;
			}

			template <typename EventType>
			bool event(this auto&& self, EventType&& event_instance)
			{
				self.template event_impl<EventType>(std::forward<EventType>(event_instance)...);

				return true;
			}

			template <typename EventType, typename... Args>
			bool event(this auto&& self, Args&&... args)
			{
				self.template event_impl<EventType>(std::forward<Args>(args)...);

				return true;
			}

			template <typename EventType, typename... Args>
			bool event(this auto&& self, Timer timer, Args&&... args)
			{
				self.template event_impl<EventType>(std::move(timer), std::forward<Args>(args)...);

				return true;
			}

			explicit operator bool() const
			{
				return (static_cast<bool>(_context));
			}

			template <typename EventType>
			void event_impl(this auto&& self, EventType&& event_instance)
			{
				auto& service = self.get_service();

				service.template queue_event<EventType>(std::forward<EventType>(event_instance));
			}

			template <typename EventType, typename... Args>
			void event_impl(this auto&& self, Args&&... args)
			{
				auto& service = self.get_service();

				service.template queue_event<EventType>(std::forward<Args>(args)...);
			}

			template <typename EventType, typename... Args>
			void event_impl(this auto&& self, Timer timer, Args&&... args)
			{
				auto& service = self.template get_service();

				service.template timed_event<EventType>(std::move(timer), std::forward<Args>(args)...);
			}

		protected:
			void set_context(const MetaEvaluationContext& context)
			{
				_context = &context;
			}

			// The context with which resolution of indirect values may be performed.
			const MetaEvaluationContext* _context = {};
	};
}