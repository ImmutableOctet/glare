#pragma once

#include "types.hpp"
#include "action.hpp"

#include "service_policy.hpp"
#include "system_manager_interface.hpp"
#include "service_events.hpp"
#include "timed_event.hpp"
#include "timer.hpp"
#include "command.hpp"

#include "event_handler.hpp"

#include "commands/events.hpp"

#include "entity/entity_variables.hpp"
#include "meta/types.hpp"

#include <app/types.hpp>

#include <util/small_vector.hpp>

//#include <entt/signal/fwd.hpp>

#include <utility>
#include <functional>
#include <type_traits>
#include <string_view>
#include <optional>
#include <memory>
#include <string>
//#include <vector>

// Debugging related:
//#include <util/log.hpp>

namespace app
{
	namespace input
	{
		struct MouseState;
		struct KeyboardState;
	}

	namespace graphics
	{
		struct Graphics;
	}
}

namespace engine
{
	class ResourceManager;

	struct IndirectComponentPatchCommand;
	struct ComponentPatchCommand;
	struct ComponentReplaceCommand;
	struct FunctionCommand;
	struct ExprCommand;
	struct SetParentCommand;

	class Service
	{
		public:
			using EventHandler = entt::dispatcher;

			using OpaqueFunction = std::function<void()>;
			using UniversalVariables = EntityVariables<8>;

			Service
			(
				Registry& registry,
				SystemManagerInterface& systems,
				const ServicePolicy& policy={},

				bool register_input_events=true,
				bool register_timed_event_wrapper=false,
				bool register_core_commands=true,
				bool register_evaluation_commands=true,
				bool allocate_root_entity=true,
				bool allocate_universal_variables=false
			);

			Service(const Service&) = delete;
			Service(Service&&) noexcept = delete;

			virtual ~Service();

			/*
			inline Registry& get_registry()
			{
				return registry;
			}

			inline const Registry& get_registry() const
			{
				return registry;
			}
			*/

			inline Registry& get_registry() const
			{
				return registry;
			}

			inline Entity get_root() const
			{
				return root;
			}

			inline operator Entity() const
			{
				return get_root();
			}

			// TODO: Determine if it makes more sense to store the `ResourceManager` inside this class instead.
			virtual ResourceManager& get_resource_manager() = 0;
			virtual const ResourceManager& get_resource_manager() const = 0;

			inline bool has_event_queued() const
			{
				return (standard_event_handler.size()) || (forwarding_event_handler.size());
			}

			template <typename EventType, auto fn, typename obj_type>
			inline void register_event(obj_type& obj)
			{
				register_event<EventType, fn>(obj, false); // has_event_queued()
			}

			template <typename EventType, auto fn, typename obj_type>
			inline void register_event(obj_type& obj, bool defer_operation)
			{
				if constexpr (std::is_base_of_v<ServiceOriginatedEvent, std::decay_t<EventType>>)
				{
					service_event_handler.sink<EventType>().connect<fn>(obj);
				}
				else
				{
					if (defer_operation)
					{
						later
						(
							[this, &obj]()
							{
								register_event<EventType, fn>(obj, false);
							}
						);

						return;
					}

					standard_event_handler.sink<EventType>().connect<fn>(obj);
					forwarding_event_handler.sink<EventType>().connect<fn>(obj);
				}
			}

			template <typename EventType, auto fn, typename obj_type>
			inline void unregister_event(obj_type& obj)
			{
				unregister_event<EventType, fn>(obj, false); // has_event_queued()
			}

			template <typename EventType, auto fn, typename obj_type>
			inline void unregister_event(obj_type& obj, bool defer_operation)
			{
				if constexpr (std::is_base_of_v<ServiceOriginatedEvent, std::decay_t<EventType>>)
				{
					service_event_handler.sink<EventType>().disconnect<fn>(obj);
				}
				else
				{
					if (defer_operation)
					{
						later
						(
							[this, &obj]()
							{
								unregister_event<EventType, fn>(obj, false);
							}
						);

						return;
					}

					standard_event_handler.sink<EventType>().disconnect<fn>(obj);
					forwarding_event_handler.sink<EventType>().disconnect<fn>(obj);
				}
			}

			template <typename EventType, typename... Args>
			inline void queue_event(Args&&... args)
			{
				if constexpr (std::is_same_v<std::decay_t<EventType>, TimedEvent>)
				{
					enqueue_timed_event(TimedEvent { std::forward<Args>(args)... });
				}
				else if constexpr (std::is_base_of_v<ServiceOriginatedEvent, std::decay_t<EventType>>)
				{
					service_event_handler.enqueue<EventType>(std::forward<Args>(args)...);
				}
				else
				{
					on_queue<EventType>(args...);

					active_event_handler->enqueue<EventType>(std::forward<Args>(args)...); // EventType { ... }
				}
			}

			template <typename EventType>
			inline void queue_event(EventType&& event_obj)
			{
				if constexpr (std::is_same_v<std::decay_t<EventType>, TimedEvent>)
				{
					enqueue_timed_event(std::forward<EventType>(event_obj));
				}
				// NOTE: This condition is specific to the forwarding-reference overload of `queue_event`.
				else if constexpr (std::is_same_v<std::decay_t<EventType>, MetaAny>)
				{
					// TODO: Look into implementing 'queue' equivalent for opaque events.
					event<EventType>(std::forward<EventType>(event_obj)); // trigger_opaque_event(std::move(event_obj));
				}
				else if constexpr (std::is_base_of_v<ServiceOriginatedEvent, std::decay_t<EventType>>)
				{
					service_event_handler.enqueue(std::forward<EventType>(event_obj));
				}
				else
				{
					on_queue<EventType>(event_obj);

					active_event_handler->enqueue(std::forward<EventType>(event_obj));
				}
			}

			template <typename EventType, typename... Args>
			inline void event(Args&&... args)
			{
				if constexpr (std::is_same_v<std::decay_t<EventType>, TimedEvent>)
				{
					queue_event<EventType, Args...>(std::forward<Args>(args)...);
				}
				else if constexpr (std::is_base_of_v<ServiceOriginatedEvent, std::decay_t<EventType>>)
				{
					service_event_handler.trigger<EventType>(EventType { std::forward<Args>(args)... });
				}
				else
				{
					on_trigger<EventType>(args...);

					active_event_handler->trigger<EventType>(EventType { std::forward<Args>(args)... });
				}
			}

			template <typename EventType>
			inline void event(EventType&& event_obj)
			{
				if constexpr (std::is_same_v<std::decay_t<EventType>, TimedEvent>)
				{
					//enqueue_timed_event(std::forward<TimedEvent>(event_obj));
					queue_event<TimedEvent>(std::forward<TimedEvent>(event_obj));
				}
				// NOTE: This condition is specific to the forwarding-reference overload of `event`.
				else if constexpr (std::is_same_v<std::decay_t<EventType>, MetaAny>)
				{
					trigger_opaque_event(std::move(event_obj));
				}
				else if constexpr (std::is_base_of_v<ServiceOriginatedEvent, std::decay_t<EventType>>)
				{
					service_event_handler.trigger<EventType>(std::forward<EventType>(event_obj));
				}
				else
				{
					on_trigger<EventType>(event_obj);

					active_event_handler->trigger(std::forward<EventType>(event_obj));
				}
			}

			template <typename EventType, typename... Args>
			inline void timed_event(Timer timer, Args&&... args)
			{
				queue_event<TimedEvent>(std::move(timer), EventType { std::forward<Args>(args)... });
			}

			template <typename EventType>
			inline void timed_event(Timer timer, EventType&& event_obj)
			{
				queue_event<TimedEvent>(TimedEvent { std::move(timer), std::move(event_obj) });
			}

			template <typename EventType, typename... Args>
			inline void timed_event(Timer::Duration timer_duration, Args&&... args)
			{
				timed_event<EventType>(Timer(timer_duration), std::forward<Args>(args)...);
			}

			template <typename EventType, typename... Args>
			inline void timed_event(std::optional<Timer> timer, Args&&... args)
			{
				if (!timer)
				{
					queue_event<EventType>(std::forward<Args>(args)...);

					return;
				}

				timed_event<EventType>(std::move(*timer), std::forward<Args>(args)...);
			}

			template <typename EventType, typename... Args>
			inline void timed_event(std::optional<Timer::Duration> timer_duration, Args&&... args)
			{
				if (!timer_duration)
				{
					queue_event<EventType>(std::forward<Args>(args)...);

					return;
				}

				timed_event<EventType>(Timer(*timer_duration), std::forward<Args>(args)...);
			}

			template <typename TimeType>
			inline void timed_event(std::optional<TimeType> timer_duration, MetaAny&& event_instance)
			{
				if (timer_duration)
				{
					queue_event<TimedEvent>
					(
						TimedEvent
						{
							std::move(*timer_duration),
							std::move(event_instance)
						}
					);
				}
				else
				{
					// TODO: Look into implementing 'queue' equivalent for opaque events.
					trigger_opaque_event(std::move(event_instance));
				}
			}

			// Unregisters all event triggers tied to a given object.
			template <typename obj_type>
			inline void unregister(obj_type& obj)
			{
				standard_event_handler.disconnect(obj);
				forwarding_event_handler.disconnect(obj);
			}

			template <typename EventType, auto fn>
			inline void register_free_function()
			{
				standard_event_handler.sink<EventType>().connect<fn>();
				forwarding_event_handler.sink<EventType>().connect<fn>();
			}

			template <typename EventType, auto fn>
			inline void unregister_free_function()
			{
				standard_event_handler.sink<EventType>().disconnect<fn>();
				forwarding_event_handler.sink<EventType>().disconnect<fn>();
			}

			// TODO: Deprecate.
			// 
			// Queues `fn` to be executed with `args` on the next call to `update`.
			// Deferred calls (`Actions`) are executed in-order for the function-type specified.
			// This means that lambdas can be used to uniquely queue a particular action in FIFO order.
			// For a good example of this, see `World::set_parent`.
			// NOTE: `fn` does not currently support lambdas with captures. Regular lambdas should work fine, though.
			template <typename fn_t, typename... arguments>
			inline void defer(fn_t&& f, arguments&&... args)
			{
				service_event_handler.enqueue(make_action(std::forward<fn_t>(f), std::forward<arguments>(args)...)); // queue_event(...)

				// Debugging related:
				//make_action(std::forward<fn_t>(f), std::forward<arguments>(args)...);
			}

			// New API.
			// 
			// TODO: Remove/replace `defer`.
			template <typename Callback>
			void later(Callback&& callback)
			{
				deferred_operations.emplace_back(callback);
			}

			void update(app::Milliseconds time, float delta);
			void fixed_update(app::Milliseconds time, float delta=1.0f);

			void render(app::Graphics& gfx);

			/*
				Retrieves the current parent of `entity`.

				If `entity` does not have a parent, or if `entity` does not
				have a `RelationshipComponent` attached, this will return `null`.
			*/
			Entity get_parent(Entity entity) const;

			/*
				Sets the parent of `entity` to `parent`.
				
				The return value is `entity`'s previous parent.

				If `entity` did not have a parent prior to this call,
				the return-value will be `null` instead.
			*/
			Entity set_parent(Entity entity, Entity parent);

			/*
				Attempts to remove the relationship between `child` and `entity`.
				
				If `root` exists, rather than orphaning the `child`,
				its parent will instead be set to `root`.

				If there is no parent/child relationship between
				`entity` and `child`, this will return false.
			*/
			bool remove_child(Entity entity, Entity child);

			/*
				Removes the active parent of `entity`,
				replacing it with `root` if possible.

				The return value of this function indicates if
				`entity`'s parent has been changed to `root` (true),
				or if it has been removed completely (false).
			*/
			bool remove_parent(Entity entity);

			// Returns a label for the entity specified.
			// If no `NameComponent` is associated with the entity, the entity number will be used.
			std::string label(Entity entity) const;

			// Returns the name associated with the `entity` specified.
			// If no `NameComponent` is associated with the entity, an empty string will be returned.
			std::string_view get_name(Entity entity) const; // const;

			// Returns true if the result of `get_name` is a non-empty string.
			bool has_name(Entity entity) const;

			// Sets the name of `entity`, emplacing a `NameComponent` if necessary.
			// The return value of this function indicates success/failure.
			bool set_name(Entity entity, std::string_view name);

			// Retrieves the first entity found with the `name` specified.
			// NOTE: Multiple entities may share the same name.
			Entity get_by_name(std::string_view name) const;

			/*
				Retrieves the first child-entity found with the name specified,
				regardless of other attributes/components.
				(e.g. includes both bone & non-bone children)
			*/
			Entity get_child_by_name(Entity entity, std::string_view child_name, bool recursive=true) const;

			// Attempts to retrieve an entity with the `player` index specified.
			Entity get_player(PlayerIndex player) const;

			// Attempts to retrieve the primary local player.
			Entity get_player() const;

			// NOTE: Registering to this event handler is considered unsafe due to there
			// being 'standard' and 'forwarding' event handlers internally. Use this method with caution.
			EventHandler& get_active_event_handler();

			// See `get_active_event_handler` for notes on using service-owned event handlers directly.
			EventHandler& get_standard_event_handler();

			// See `get_active_event_handler` for notes on using service-owned event handlers directly.
			EventHandler& get_forwarding_event_handler();

			// Retrieves a non-owning pointer to an internal `UniversalVariables` object.
			// If a `UniversalVariables` object does not already exist for this service, this will return `nullptr`.
			UniversalVariables* peek_universal_variables() const;

			// Attempts to allocate a `UniversalVariables` object, managed internally.
			// If a `UniversalVariables` object has already been allocated, the existing instance will be used.
			// 
			// The return value of this member-function is a const-reference to the internal member referencing the remote object.
			// 
			// TODO: Look into pros/cons of changing this interface to const + mutable member.
			const std::shared_ptr<UniversalVariables>& get_universal_variables();
		private:
			// Generates a generic `Command` base-type from a derived type.
			template <typename CommandType, typename ...Args>
			Command generalize_command(Entity source, Entity target, const Args&... other_args)
			{
				return Command(source, target);
			}

			// Generates a generic `Command` base-type from a derived type.
			template <typename CommandType, typename ...Args>
			Command generalize_command(const Command& command_instance, const Args&... other_args) // const CommandType&
			{
				return Command(command_instance);
			}

			template <typename EventType, typename Callback, typename ...Args>
			void on_event(Callback&& callback, const Args&... args)
			{
				if constexpr (std::is_base_of_v<Command, EventType>)
				{
					std::optional<MetaTypeID> command_id = std::nullopt;

					if (auto type = entt::resolve<EventType>())
					{
						command_id = type.id();
					}

					callback
					(
						OnCommandExecution
						{
							generalize_command<EventType>(args...),
							command_id
						}
					);
				}
			}

			template <typename EventType, typename ...Args>
			void on_queue(const Args&... args)
			{
				on_event<EventType>
				(
					[this](auto&& instance)
					{
						queue_event(std::move(instance));
					},

					args...
				);
			}

			template <typename EventType, typename ...Args>
			void on_trigger(const Args&... args)
			{
				on_event<EventType>
				(
					[this](auto&& instance)
					{
						event(std::move(instance));
					},

					args...
				);
			}

			void on_indirect_component_patch(const IndirectComponentPatchCommand& component_patch);

			// NOTE: This does not take the event as const-reference since we would normally
			// move from the enclosed `ComponentPatchCommand::component` object.
			// 
			// This, in turn, means that said object is in a moved-from state after this method executes.
			void on_direct_component_patch(const ComponentPatchCommand& component_patch);

			// NOTE: This does not take the event as const-reference since we would normally
			// move from the enclosed `ComponentReplaceCommand::component` object.
			// 
			// This, in turn, means that said object is in a moved-from state after this method executes.
			void on_component_replace(ComponentReplaceCommand& component_replace);

			void on_set_parent(const SetParentCommand& parent_command);
		protected:
			void opaque_function_handler(const FunctionCommand& function_command);
			void opaque_expression_handler(const ExprCommand& expr_command);

			void relationship_destroyed_handler(Registry& registry, Entity entity);

			virtual void on_function_command(const FunctionCommand& function_command);
			virtual void on_expression_command(const ExprCommand& expr_command);

			void handle_deferred_operations();

			Registry& registry; // std::reference_wrapper<Registry>
			SystemManagerInterface& systems;

			Entity root = null;

			std::shared_ptr<UniversalVariables> universal_variables;

			ServicePolicy policy;

			EventHandler* swap_event_handlers();
			EventHandler* use_standard_events();
			EventHandler* use_forwarding_events();

			// Input re-submission callbacks. -- These routines produce additional
			// events indicating that this service accepted an input.
			// For more details, see `input_events`.
			void on_mouse_input(const app::input::MouseState& mouse);
			void on_keyboard_input(const app::input::KeyboardState& keyboard);

			// Interop functionality with standard event-handler systems.
			void enqueue_timed_event_wrapper(const TimedEvent& timed_event);

			// Regular rvalue input from directly enqueued timed-event objects.
			void enqueue_timed_event(TimedEvent&& timed_event);

			// Enumerates timed events, triggering underlying event types when timers complete.
			void update_timed_events();

			// Triggers the the event method for the underlying object in `event_instance`.
			// The return-value of this method indicates success.
			bool trigger_opaque_event(MetaAny&& event_instance);

			// A pointer to the active event handler object.
			// (One of the two found below)
			EventHandler* active_event_handler;

			// Standard event handler; used under most conditions.
			EventHandler standard_event_handler;

			// Forwarding event handler; used while processing events.
			EventHandler forwarding_event_handler;

			// Used for service-originated events and deferred operations.
			// (Prevents conflicts during service operations)
			EventHandler service_event_handler;

			util::small_vector<TimedEvent, 8> pending_timed_events;
			util::small_vector<OpaqueFunction, 8> deferred_operations;
	};
}