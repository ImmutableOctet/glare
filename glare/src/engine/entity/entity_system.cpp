#include "entity_system.hpp"

#include "events.hpp"
#include "entity_instruction.hpp"
#include "entity_thread_description.hpp"
#include "entity_thread.hpp"
#include "entity_descriptor.hpp"
#include "entity_state.hpp"
#include "entity_state_action.hpp"

#include "components/state_component.hpp"
#include "components/entity_thread_component.hpp"
#include "components/instance_component.hpp"

#include "commands/commands.hpp"

#include <engine/service.hpp>
#include <engine/service_events.hpp>
#include <engine/meta/meta.hpp>
#include <engine/meta/events.hpp>
#include <engine/meta/traits.hpp>

#include <engine/resource_manager/resource_manager.hpp>

#include <util/variant.hpp>

#include <optional>
#include <iterator>
#include <type_traits>
#include <tuple>
#include <utility>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	EntitySystem::EntitySystem(Service& service, const ResourceManager& resource_manager, bool subscribe_immediately)
		: BasicSystem(service), resource_manager(resource_manager)
	{
		if (subscribe_immediately)
		{
			subscribe();
		}
	}

	bool EntitySystem::on_subscribe(Service& service)
	{
		auto& registry = service.get_registry();

		// Registry events:
		registry.on_construct<StateComponent>().connect<&EntitySystem::on_state_init>(*this);
		registry.on_update<StateComponent>().connect<&EntitySystem::on_state_update>(*this);
		registry.on_destroy<StateComponent>().connect<&EntitySystem::on_state_destroyed>(*this);

		// Standard events:
		service.register_event<OnServiceUpdate,           &EntitySystem::on_update>(*this);
		service.register_event<OnServiceFixedUpdate,      &EntitySystem::on_fixed_update>(*this);
		service.register_event<OnStateChange,             &EntitySystem::on_state_change>(*this);
		service.register_event<OnStateActivate,           &EntitySystem::on_state_activate>(*this);
		service.register_event<OnComponentCreate,         &EntitySystem::on_component_create>(*this);
		service.register_event<OnComponentUpdate,         &EntitySystem::on_component_update>(*this);
		service.register_event<OnComponentDestroy,        &EntitySystem::on_component_destroy>(*this);

		// Commands:
		service.register_event<StateChangeCommand,        &EntitySystem::on_state_change_command>(*this);
		service.register_event<StateActivationCommand,    &EntitySystem::on_state_activation_command>(*this);

		service.register_event<EntityThreadSpawnCommand,  &EntitySystem::on_thread_spawn_command>(*this);
		service.register_event<EntityThreadStopCommand,   &EntitySystem::on_thread_stop_command>(*this);
		service.register_event<EntityThreadPauseCommand,  &EntitySystem::on_thread_pause_command>(*this);
		service.register_event<EntityThreadResumeCommand, &EntitySystem::on_thread_resume_command>(*this);
		service.register_event<EntityThreadAttachCommand, &EntitySystem::on_thread_attach_command>(*this);
		service.register_event<EntityThreadDetachCommand, &EntitySystem::on_thread_detach_command>(*this);
		service.register_event<EntityThreadUnlinkCommand, &EntitySystem::on_thread_unlink_command>(*this);
		service.register_event<EntityThreadSkipCommand,   &EntitySystem::on_thread_skip_command>(*this);
		service.register_event<EntityThreadRewindCommand, &EntitySystem::on_thread_rewind_command>(*this);

		return true;
	}

	bool EntitySystem::on_unsubscribe(Service& service)
	{
		auto& registry = service.get_registry();

		registry.on_construct<StateComponent>().disconnect(*this);
		registry.on_update<StateComponent>().disconnect(*this);
		registry.on_destroy<StateComponent>().disconnect(*this);

		service.unregister(*this);

		return true;
	}

	Registry& EntitySystem::get_registry() const
	{
		return get_service().get_registry();
	}

	const EntityDescriptor* EntitySystem::get_descriptor(Entity entity) const
	{
		if (entity == null)
		{
			return nullptr;
		}

		auto& registry = get_registry();

		const auto* instance_details = registry.try_get<InstanceComponent>(entity);

		if (!instance_details)
		{
			return nullptr;
		}

		return resource_manager.get_existing_descriptor(instance_details->instance);
	}

	std::optional<EntityStateIndex> EntitySystem::get_state(Entity entity) const
	{
		auto& registry = get_registry();

		if (const auto* current_state = registry.try_get<StateComponent>(entity))
		{
			return current_state->state_index;
		}

		return std::nullopt;
	}

	bool EntitySystem::set_state(Entity entity, std::string_view state_name) const
	{
		return set_state(entity, hash(state_name));
	}

	bool EntitySystem::set_state(Entity entity, StringHash state_id) const
	{
		return set_state_by_id(entity, state_id);
	}

	bool EntitySystem::set_state_by_id(Entity entity, StringHash state_id) const
	{
		const auto* descriptor = get_descriptor(entity);

		if (!descriptor)
		{
			return false;
		}

		const auto current_state = get_state(entity);

		const auto& from_state_index = current_state;
		
		const EntityState* from_state = (from_state_index)
			? descriptor->states.at(*from_state_index).get()
			: nullptr
		;

		const std::optional<StringHash> from_state_id = (from_state)
			? from_state->name
			: std::nullopt
		;

		const auto& to_state_id = state_id;

		const auto to_state = descriptor->get_state(to_state_id); // auto*

		if (!to_state)
		{
			return false;
		}

		const auto to_state_index = descriptor->get_state_index(to_state_id);

		//assert(to_state_index.has_value());

		if (!to_state_index)
		{
			return false;
		}

		auto new_state_info = EntityStateInfo { *to_state_index, to_state_id };

		auto old_state_info = (from_state)
			? EntityStateInfo { *from_state_index, from_state_id }
			: new_state_info
		;

		auto& registry = get_registry();

		if (to_state->has_activation_delay())
		{
			assert(from_state_index);

			// Decay the previous state, if present.
			if (from_state)
			{
				from_state->decay(*descriptor, registry, entity, *from_state_index, &to_state->components.persist);
			}

			// Force-update the state-component to reflect the new (unactivated) state.
			to_state->force_update_component(registry, entity, *to_state_index, from_state_index);
				
			// Notify listeners that we've changed state, but haven't activated yet.
			service->queue_event<OnStateChange>
			(
				entity,

				std::move(old_state_info),
				std::move(new_state_info),

				false // Delayed activation.
			);

			// Generate a timed event for activation, based on the delay indicated by the state:
			service->timed_event<StateActivationCommand>
			(
				*(to_state->activation_delay),

				entity,
				entity,

				to_state_id
			);

			return true;
		}

		if (descriptor->set_state_by_index(registry, entity, from_state_index, *to_state_index))
		{
			// Notify listeners that the state of `entity` has changed.
			service->queue_event<OnStateChange>
			(
				entity,

				std::move(old_state_info),
				new_state_info,

				true // Immediate activation.
			);

			// Notify listeners that activation has occurred.
			service->queue_event<OnStateActivate>
			(
				entity,

				std::move(new_state_info)
			);

			return true;
		}

		return false;
	}

	bool EntitySystem::activate_state(Entity entity, StringHash state_id) const
	{
		const auto* descriptor = get_descriptor(entity);

		if (!descriptor)
		{
			return false;
		}

		const auto target_state_index = get_state(entity);

		if (!target_state_index)
		{
			return false;
		}

		const auto target_state = descriptor->states.at(*target_state_index).get();

		if (!target_state)
		{
			return false;
		}

		auto& registry = get_registry();

		target_state->activate(*descriptor, registry, entity, *target_state_index, std::nullopt, false);

		return true;
	}

	void EntitySystem::on_update(const OnServiceUpdate& data)
	{
		// ...
	}

	void EntitySystem::on_fixed_update(const OnServiceFixedUpdate& data)
	{
		auto& registry = get_registry();

		registry.view<InstanceComponent, EntityThreadComponent>().each
		(
			[this, &registry](auto entity, const InstanceComponent& instance_comp, EntityThreadComponent& thread_comp)
			{
				const auto descriptor = resource_manager.get_existing_descriptor(instance_comp.instance);
				
				if (!descriptor)
				{
					return;
				}

				bool has_updated_thread = false;

				for (auto& thread_entry : thread_comp.threads)
				{
					if (thread_entry.is_suspended()) // || thread_entry.is_complete (implied)
					{
						continue;
					}

					const auto& thread_source = descriptor->get_thread(thread_entry.thread_index);

					thread_entry.next_instruction = step_thread
					(
						registry,
						entity,
						
						*descriptor,
						thread_source,

						thread_comp,
						thread_entry
					);

					if (thread_entry.is_complete)
					{
						service->queue_event<OnThreadComplete>
						(
							entity,

							thread_entry.thread_index,
							thread_source.thread_id,

							static_cast<EntityThreadIndex>
							(
								std::distance(thread_comp.threads.begin(), &thread_entry)
							),

							thread_entry.next_instruction
						);
					}

					has_updated_thread = true;
				}

				if (has_updated_thread)
				{
					registry.patch<EntityThreadComponent>(entity); // [](auto&){}
				}
			}
		);
	}

	// TODO: Review event-handling issue.
	void EntitySystem::on_state_init(Registry& registry, Entity entity)
	{
		on_state_update_impl(registry, entity, false);
	}

	// TODO: Review event-handling issue.
	void EntitySystem::on_state_update(Registry& registry, Entity entity)
	{
		on_state_update_impl(registry, entity);
	}

	void EntitySystem::on_state_update_impl(Registry& registry, Entity entity, bool handle_existing)
	{
		const auto* descriptor = get_descriptor(entity);

		assert(descriptor);

		const auto& state_component = registry.get<StateComponent>(entity);

		if (handle_existing)
		{
			// No state-change occurred, no need to update anything.
			if (state_component.state_index == state_component.prev_state_index)
			{
				return;
			}

			// NOTE: Does not currelty destruct/disconnect existing listeners.
			// TODO: Determine if reference counting makes sense for this use-case.
			
			/*
			// Explcit removal implementation (faster, but potentially less thorough):
			const auto prev_state_index = state_component.prev_state_index;
			const auto& prev_state = *descriptor->states[prev_state_index];

			for (const auto& rule_entry : prev_state.rules)
			{
				const auto& event_type_id = rule_entry.first;
				auto listener_it = listeners.find(event_type_id);

				if (listener_it == listeners.end())
				{
					continue;
				}

				auto& listener_entry = *listener_it;
				auto& listener = listener_entry.second;

				listener.remove_entity(entity);
			}
			*/

			// Sweep-based removal implementation (slower, but ensures removal):
			for (auto& listener_entry : listeners)
			{
				auto& listener = listener_entry.second;

				listener.remove_entity(entity);
			}
		}

		const auto state_index = state_component.state_index;

		const auto& state = *(descriptor->states[state_index]);

		// NOTE: For component-member based triggers, this will look
		// for the component type in the form of an event.
		// 
		// These triggers are satisfied via the `OnComponentCreate`/`OnComponentUpdate`
		// handlers in this system, which forward a reference to the underlying component.
		for (const auto& rule_entry : state.rules)
		{
			const auto& event_type_id = rule_entry.first;
			//const auto& state_rules   = rule_entry.second;

			auto* listener = listen(event_type_id);

			if (!listener)
			{
				continue;
			}

			listener->add_entity(entity);
		}

		/*
		for (const auto& thread : state.threads)
		{
			// TODO: Handle listening for entity thread triggers.
			listen(...)
		}
		*/
	}

	void EntitySystem::on_state_destroyed(Registry& registry, Entity entity)
	{
		// TODO: Implement state cleanup.
	}
	
	// Handles logging/printing for debugging purposes.
	void EntitySystem::on_state_change(const OnStateChange& state_change)
	{
		assert(state_change.to.id.has_value());

		if (state_change.from.id)
		{
			print("Entity {}: state changed from #{} to #{}", state_change.entity, *state_change.from.id, *state_change.to.id);
		}
		else
		{
			print("Entity {}: state changed to #{}", state_change.entity, *state_change.to.id);
		}
	}

	void EntitySystem::on_state_activate(const OnStateActivate& state_activate)
	{
		if (!state_activate.state.id)
		{
			return;
		}

		print("Entity {}: state #{} activated", state_activate.entity, *state_activate.state.id);
	}

	void EntitySystem::on_state_change_command(const StateChangeCommand& state_change)
	{
		set_state(state_change.target, state_change.state_name);
	}

	void EntitySystem::on_state_activation_command(const StateActivationCommand& state_activation)
	{
		auto result = activate_state(state_activation.target, state_activation.state_name);

		//assert(result);

		if (result)
		{
			if (state_activation.source == state_activation.target)
			{
				print("Entity {}: state #{} activated", state_activation.target, state_activation.state_name);
			}
			else
			{
				print("Entity {}: state #{} activated by Entity {}", state_activation.target, state_activation.state_name, state_activation.source);
			}
		}
		else
		{
			print_error("Failed to activate state #{} for Entity {} (activation by {})", state_activation.state_name, state_activation.target, state_activation.source);
		}
	}

	template <bool allow_emplace, typename EventType, typename ThreadCommandType, typename RangeCallback, typename IDCallback, typename ...EventArgs>
	void EntitySystem::thread_command_impl(const ThreadCommandType& thread_command, RangeCallback&& range_callback, IDCallback&& id_callback, std::string_view dbg_name, std::string_view dbg_name_past_tense, EventArgs&&... event_args)
	{
		static_assert(!std::is_same_v<ThreadCommandType, void>);
		static_assert(!std::is_same_v<RangeCallback, void>);
		static_assert(!std::is_same_v<IDCallback, void>);

		auto& registry = get_registry();

		const auto entity = thread_command.entity();
		const auto& thread_target = thread_command.threads;

		if (thread_target.empty())
		{
			print_warn("Entity #{}: Empty thread command detected.", entity);

			return;
		}

		auto& thread_component = (allow_emplace)
			? registry.get_or_emplace<EntityThreadComponent>(entity)
			: registry.get<EntityThreadComponent>(entity)
		;

		util::visit
		(
			thread_target.value,

			//  Should never be reached anyway. (see above use of `empty`)
			[](const EntityThreadTarget::Empty&) {},

			[this, &range_callback, &thread_command, entity, &thread_component, &dbg_name, &dbg_name_past_tense](const EntityThreadRange& threads)
			{
				const auto begin = threads.begin_point();
				
				if (threads.size() == 1)
				{
					print("Entity #{}: Attempting to {} thread #{}...", entity, dbg_name, begin);
				}
				else
				{
					const auto end = threads.end_point();

					print("Entity #{}: Attempting to {} threads #{} through #{}...", entity, dbg_name, begin, (end - 1));
				}

				const auto result = range_callback(thread_command, thread_component, threads);

				if (static_cast<bool>(result))
				{
					if (threads.size() == 1)
					{
						print("Entity #{}: Thread #{} {} successfully.", entity, begin, dbg_name_past_tense);
					}
					else
					{
						print("Entity #{}: {} {} of {} threads.", entity, dbg_name_past_tense, result, threads.size());
					}
				}
				else
				{
					print_warn("Entity #{}: Failed to {} any threads.", entity, dbg_name);
				}

				if constexpr (!std::is_same_v<EventType, void>)
				{
					bool check_linked = true; // false;

					if constexpr (has_field_check_linked_v<ThreadCommandType>)
					{
						check_linked = thread_command.check_linked;
					}

					thread_component.enumerate_threads
					(
						threads,

						[this, entity, &thread_component, ...event_args = std::forward<EventArgs>(event_args)](const EntityThread& local_thread)
						{
							const auto local_index = thread_component.get_local_index(local_thread);

							assert(local_index);

							if (!local_index)
							{
								return;
							}

							service->queue_event<EventType>
							(
								entity,
								
								local_thread.thread_index,
								std::optional<EntityThreadID>(std::nullopt), // 0
								static_cast<ThreadEvent::LocalThreadIndex>(*local_index),

								local_thread.next_instruction,

								std::forward<EventArgs>(event_args)...
							);
						}
					);
				}
			},

			[this, &id_callback, &thread_command, entity, &thread_component, &dbg_name, &dbg_name_past_tense](EntityThreadID thread_id)
			{
				print("Entity #{}: Attempting to {} thread by ID (#{})...", entity, dbg_name, thread_id);
				
				const auto* descriptor = get_descriptor(entity);
				
				assert(descriptor);

				if (!descriptor)
				{
					print_warn("Unable to resolve descriptor for Entity #{}.", entity);

					return;
				}

				const auto result = id_callback(thread_command, thread_component, *descriptor, thread_id);

				if (static_cast<bool>(result))
				{
					print("Entity #{}: Thread (#{}) {} successfully.", entity, thread_id, dbg_name_past_tense);
				}
				else
				{
					print_warn("Entity #{}: Failed to {} any threads.", entity, dbg_name);
				}

				if constexpr (!std::is_same_v<EventType, void>)
				{
					bool check_linked = true; // false;

					if constexpr (has_field_check_linked_v<ThreadCommandType>)
					{
						check_linked = thread_command.check_linked;	
					}

					const auto local_thread = thread_component.get_thread(*descriptor, thread_id, check_linked);

					//assert(local_thread);

					if (!local_thread)
					{
						return;
					}

					const auto local_index = thread_component.get_local_index(*local_thread);

					assert(local_index);

					if (!local_index)
					{
						return;
					}

					service->queue_event<EventType>
					(
						entity,
						local_thread->thread_index,
						thread_id,
						static_cast<ThreadEvent::LocalThreadIndex>(*local_index),
						local_thread->next_instruction,

						std::forward<EventArgs>(event_args)...
					);
				}
			}
		);
	}

	void EntitySystem::on_thread_spawn_command(const EntityThreadSpawnCommand& thread_command)
	{
		thread_command_impl<true, OnThreadSpawn>
		(
			thread_command,
			
			[](const EntityThreadSpawnCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads)
			{
				return thread_component.start_threads(threads, thread_command.state_index, thread_command.restart_existing);
			},

			[](const EntityThreadSpawnCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return thread_component.start_thread
				(
					descriptor, thread_id,
					thread_command.state_index,
					true, true,
					thread_command.restart_existing
				);
			},

			"start", "started"
		);
	}

	void EntitySystem::on_thread_stop_command(const EntityThreadStopCommand& thread_command)
	{
		thread_command_impl<false, OnThreadTerminated>
		(
			thread_command,
			
			[](const EntityThreadStopCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads)
			{
				return thread_component.stop_threads(threads, thread_command.check_linked);
			},

			[](const EntityThreadStopCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return thread_component.stop_thread(descriptor, thread_id, thread_command.check_linked);
			},

			"stop", "stopped"
		);
	}

	void EntitySystem::on_thread_pause_command(const EntityThreadPauseCommand& thread_command)
	{
		thread_command_impl<false, OnThreadPaused>
		(
			thread_command,
			
			[](const EntityThreadPauseCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads)
			{
				return thread_component.pause_threads(threads, thread_command.check_linked);
			},

			[](const EntityThreadPauseCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return thread_component.pause_thread(descriptor, thread_id, thread_command.check_linked);
			},

			"pause", "paused"
		);
	}

	void EntitySystem::on_thread_resume_command(const EntityThreadResumeCommand& thread_command)
	{
		thread_command_impl<false, OnThreadResumed>
		(
			thread_command,
			
			[](const EntityThreadResumeCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads)
			{
				return thread_component.resume_threads(threads, thread_command.check_linked);
			},

			[](const EntityThreadResumeCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return thread_component.resume_thread(descriptor, thread_id, thread_command.check_linked);
			},

			"resume", "resumed"
		);
	}

	void EntitySystem::on_thread_attach_command(const EntityThreadAttachCommand& thread_command)
	{
		thread_command_impl<false, OnThreadAttach>
		(
			thread_command,
			
			[this](const EntityThreadAttachCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads) -> std::size_t // EntityThreadCount
			{
				std::optional<EntityStateIndex> state_index = std::nullopt;
				
				if (thread_command.state_id)
				{
					const auto entity = thread_command.entity();
					const auto* descriptor = get_descriptor(entity);

					assert(descriptor);

					if (!descriptor)
					{
						print_warn("Unable to resolve descriptor for Entity #{}.", entity);

						return 0;
					}

					state_index = descriptor->get_state_index(thread_command.state_id);
				}

				return thread_component.attach_threads(threads, state_index, thread_command.check_linked);
			},

			[](const EntityThreadAttachCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				const auto state_index = descriptor.get_state_index(thread_command.state_id);

				return thread_component.attach_thread(descriptor, thread_id, state_index, thread_command.check_linked);
			},

			"attach", "attached"
		);
	}

	void EntitySystem::on_thread_detach_command(const EntityThreadDetachCommand& thread_command)
	{
		thread_command_impl<false, OnThreadDetach>
		(
			thread_command,
			
			[](const EntityThreadDetachCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads) -> std::size_t // EntityThreadCount
			{
				return thread_component.detach_threads(threads, thread_command.check_linked);
			},

			[](const EntityThreadDetachCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return thread_component.detach_thread(descriptor, thread_id, thread_command.check_linked);
			},

			"detach", "detached"
		);
	}

	void EntitySystem::on_thread_unlink_command(const EntityThreadUnlinkCommand& thread_command)
	{
		thread_command_impl<false, OnThreadUnlink>
		(
			thread_command,
			
			[](const EntityThreadUnlinkCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads) -> std::size_t // EntityThreadCount
			{
				return thread_component.unlink_threads(threads);
			},

			[](const EntityThreadUnlinkCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return thread_component.unlink_thread(descriptor, thread_id);
			},

			"unlink", "unlinked"
		);
	}

	void EntitySystem::on_thread_skip_command(const EntityThreadSkipCommand& thread_command)
	{
		thread_command_impl<false>
		(
			thread_command,
			
			[](const EntityThreadSkipCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads) -> std::size_t // EntityThreadCount
			{
				return thread_component.skip_threads(threads, thread_command.instructions_skipped, thread_command.check_linked);
			},

			[](const EntityThreadSkipCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return thread_component.skip_thread(descriptor, thread_id, thread_command.instructions_skipped, thread_command.check_linked);
			},

			"skip", "skipped"
		);
	}

	void EntitySystem::on_thread_rewind_command(const EntityThreadRewindCommand& thread_command)
	{
		thread_command_impl<false>
		(
			thread_command,
			
			[](const EntityThreadRewindCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads) -> std::size_t // EntityThreadCount
			{
				return thread_component.rewind_threads(threads, thread_command.instructions_rewound, thread_command.check_linked);
			},

			[](const EntityThreadRewindCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return thread_component.rewind_thread(descriptor, thread_id, thread_command.instructions_rewound, thread_command.check_linked);
			},

			"rewind", "rewound"
		);
	}

	void EntitySystem::on_component_create(const OnComponentCreate& component_details)
	{
		on_component_update({ component_details.entity, component_details.component.as_ref() });
	}

	void EntitySystem::on_component_update(const OnComponentUpdate& component_details)
	{
		assert(component_details.entity != null);
		assert(component_details.component);

		const auto event_type = component_details.component.type();
		const auto event_type_id = event_type.id();

		auto& listener = listeners[event_type_id];

		assert(listener.is_active());

		listener.on_event(event_type, component_details.component.as_ref());
	}

	void EntitySystem::on_component_destroy(const OnComponentDestroy& component_details)
	{
		// Nothing so far.
	}

	EntityInstructionCount EntitySystem::step_thread
	(
		Registry& registry,
		Entity entity,

		const EntityDescriptor& descriptor,
		const EntityThreadDescription& source,

		EntityThreadComponent& thread_comp,
		EntityThread& thread
	)
	{
		using namespace instructions;

		if (thread.next_instruction >= source.instructions.size())
		{
			// We've stepped beyond the scope of this thread; mark this thread as complete.
			thread.is_complete = true;

			// Always point to the last valid instruction when complete.
			thread.next_instruction = static_cast<EntityInstructionIndex>(source.instructions.size()-1);

			// Return the last valid instruction.
			return thread.next_instruction;
		}

		auto& service = get_service();

		const auto& instruction = source.instructions[thread.next_instruction];
		const auto thread_index = thread.thread_index;

		// Forwards `action` to `execute_action` with the appropriate parameters.
		auto on_action = [&registry, &service, entity, &descriptor](const auto& action)
		{
			Entity target = null;

			if constexpr (!std::is_same_v<EntityStateUpdateAction, std::decay_t<decltype(action)>>)
			{
				target = entity;
			}

			execute_action
			(
				registry,
				service,
				descriptor,
				action,
				entity,
				target
			);
		};

		auto resolve_thread_instruction = [this, &registry, &descriptor, entity, thread_index](const auto& thread_instruction) -> std::tuple<Entity, std::optional<EntityThreadIndex>>
		{
			const auto target_entity = thread_instruction.target_entity.get(registry, entity);
			
			std::optional<EntityThreadIndex> target_thread_index = std::nullopt;

			if (target_entity == entity)
			{
				if (thread_instruction.thread_id)
				{
					target_thread_index = descriptor.get_thread_index(*thread_instruction.thread_id);
				}
				else
				{
					target_thread_index = thread_index;
				}
			}
			else if (const auto* target_descriptor = get_descriptor(target_entity))
			{
				target_thread_index = target_descriptor->get_thread_index(thread_instruction.thread_id);

				//assert(target_thread_index);
			}

			return { target_entity, target_thread_index };
		};

		auto control_flow_command = [&service, entity, &resolve_thread_instruction] <typename CommandType>
		(const auto& thread_instruction, bool defer_event, auto&&... additional_args) -> bool
		{
			const auto [target_entity, target_thread_index] = resolve_thread_instruction(thread_instruction);

			assert(target_thread_index);

			if (!target_thread_index)
			{
				return false;
			}

			auto thread_range = EntityThreadRange { *target_thread_index, 1 };

			auto submit_command = [&service, defer_event](CommandType&& command)
			{
				if (defer_event)
				{
					service.queue_event(std::move(command));
				}
				else
				{
					service.event(std::move(command));
				}

				return true;
			};

			if constexpr (has_field_check_linked_v<CommandType>)
			{
				return submit_command
				(
					CommandType
					{
						entity, target_entity,

						std::move(thread_range),

						thread_instruction.check_linked,

						std::forward<decltype(additional_args)>(additional_args)...
					}
				);
			}
			else
			{
				return submit_command
				(
					CommandType
					{
						entity, target_entity,

						std::move(thread_range),

						std::forward<decltype(additional_args)>(additional_args)...
					}
				);
			}
		};

		// Launches a single thread.
		auto launch_thread = [&control_flow_command](const auto& launch, bool restart_existing=true)
		{
			control_flow_command.template operator()<EntityThreadSpawnCommand>(launch, true, restart_existing);
		};

		auto stop_thread = [&registry, &service, entity, &descriptor, &thread_comp, &thread, thread_index, &resolve_thread_instruction](const auto& stop)
		{
			const auto [target_entity, target_thread_index] = resolve_thread_instruction(stop);

			assert(target_thread_index);

			if (!target_thread_index)
			{
				return;
			}

			// NOTE: Additional force-pause logic added for safety.
			// 
			// This is mainly to cover scenarios where the event
			// produced does not execute immediately after this instruction.
			if ((target_entity == entity) && (target_thread_index == thread.thread_index))
			{
				if (thread_comp.get_thread(*target_thread_index, stop.check_linked) == &thread)
				{
					// Force-pause while awaiting 'stop' command's execution.
					thread.pause();
				}
			}

			// Enqueue deferred 'stop' command for this thread.
			service.queue_event<EntityThreadStopCommand>
			(
				entity, target_entity,

				EntityThreadRange
				{
					*target_thread_index,
					1
				},

				stop.check_linked
			);
		};

		EntityInstructionCount step_stride = 1;

		///*
		util::visit
		(
			instruction.value,

			[](const NoOp&) {},

			[&on_action](const EntityStateAction& action)           { on_action(action); },
			[&on_action](const EntityStateTransitionAction& action) { on_action(action); },
			[&on_action](const EntityStateCommandAction& action)    { on_action(action); },
			[&on_action](const EntityStateUpdateAction& action)     { on_action(action); },
			[&on_action](const EntityThreadSpawnAction& action)     { on_action(action); },
			[&on_action](const EntityThreadStopAction& action)      { on_action(action); },
			[&on_action](const EntityThreadPauseAction& action)     { on_action(action); },
			[&on_action](const EntityThreadResumeAction& action)    { on_action(action); },
			[&on_action](const EntityThreadAttachAction& action)    { on_action(action); },
			[&on_action](const EntityThreadDetachAction& action)    { on_action(action); },
			[&on_action](const EntityThreadUnlinkAction& action)    { on_action(action); },
			[&on_action](const EntityThreadSkipAction& action)      { on_action(action); },
			[&on_action](const EntityThreadRewindAction& action)    { on_action(action); },

			[&launch_thread](const Start& start) { launch_thread(start, start.restart_existing); },
			[&launch_thread](const Restart& restart) { launch_thread(restart, true); },
			[&stop_thread](const Stop& stop) { stop_thread(stop); },

			// NOTE: We currently need to trigger the generated command
			// immediately to remain compatible with multi-instructions.
			[&control_flow_command](const Pause& pause)
			{
				control_flow_command.template operator()<EntityThreadPauseCommand>(pause, false);
			},

			// NOTE: We currently need to trigger the generated command
			// immediately to remain compatible with multi-instructions.
			[&control_flow_command](const Resume& resume)
			{
				control_flow_command.template operator()<EntityThreadResumeCommand>(resume, false);
			},

			[&descriptor, &thread_comp, &thread](const Link& link)
			{
				auto local_index = thread_comp.get_local_index(thread);

				assert(local_index);

				if (!local_index)
				{
					return;
				}

				auto result = thread_comp.link_thread(*local_index);

				assert(result);
			},

			[&control_flow_command](const Unlink& unlink)
			{
				control_flow_command.template operator()<EntityThreadUnlinkCommand>(unlink, true);
			},

			[&control_flow_command](const Attach& attach)
			{
				control_flow_command.template operator()<EntityThreadAttachCommand>(attach, true, attach.state_id);
			},

			[&control_flow_command](const Detach& detach)
			{
				control_flow_command.template operator()<EntityThreadDetachCommand>(detach, true);
			},

			[&registry, &service, entity, &descriptor, thread_index, &resolve_thread_instruction](const Sleep& sleep) // [..., &thread]
			{
				const auto source_entity = entity;
				const auto [target_entity, target_thread_index] = resolve_thread_instruction(sleep);
				
				assert(target_thread_index);

				if (!target_thread_index)
				{
					return;
				}

				const auto thread_range = EntityThreadRange { *target_thread_index, 1 };

				// Trigger an event to immediately pause this thread.
				// 
				// NOTE: This event must happen immediately to ensure compatibility with multi-instructions.
				service.event<EntityThreadPauseCommand> // queue_event
				(
					source_entity, target_entity,

					thread_range,

					sleep.check_linked
				);

				// Alternative:
				//thread.pause();

				// Enqueue a timed 'resume' event to trigger after the sleep duration.
				service.timed_event<EntityThreadResumeCommand>
				(
					sleep.duration,

					source_entity, target_entity,

					thread_range,

					sleep.check_linked
				);
			},

			[this, entity, &descriptor, &thread, &step_stride](const Yield& yield)
			{
				const auto& condition_raw = yield.condition.get(descriptor);
				
				bool yielded_successfully = false;

				EventTriggerConditionType::visit_type_enabled
				(
					condition_raw,

					[this, entity, &yielded_successfully](const auto& condition)
					{
						condition.enumerate_types
						(
							[this, entity, &yielded_successfully](MetaTypeID type_id)
							{
								auto* listener = listen(type_id);
								
								if (!listener)
								{
									print_warn("Failed locate listener while processing yield condition.");

									return;
								}

								// See `EntityListener::update_entity` for corresponding `remove_entity` usage.
								listener->add_entity(entity);

								yielded_successfully = true;
							}
						);
					}
				);

				assert(yielded_successfully);

				if (!yielded_successfully)
				{
					print_warn("Failed to execute yield instruction. Pausing thread for safety.");

					thread.pause();

					return;
				}

				thread.is_yielding = true;
				step_stride = 0;
			},

			// NOTE: This instruction type is inherently incompatible with multi-instructions.
			[&control_flow_command](const Skip& skip)
			{
				control_flow_command.template operator()<EntityThreadSkipCommand>(skip, false, skip.instructions_skipped.size); // true
			},

			// NOTE: This instruction type is inherently incompatible with multi-instructions.
			[entity, &service, &resolve_thread_instruction, &step_stride](const Rewind& rewind)
			{
				const auto source_entity = entity;
				const auto [target_entity, target_thread_index] = resolve_thread_instruction(rewind);

				assert(target_thread_index);

				if (!target_thread_index)
				{
					return;
				}

				const auto thread_range = EntityThreadRange{ *target_thread_index, 1 };

				if ((target_entity == entity) && !rewind.thread_id)
				{
					step_stride = 0;
				}

				service.queue_event<EntityThreadRewindCommand>
				(
					source_entity, target_entity,
					
					thread_range,
					
					rewind.check_linked,
					rewind.instructions_rewound
				);
			},

			[entity, &registry, &descriptor, &thread, &step_stride](const IfControlBlock& control_block)
			{
				const auto& condition = control_block.condition.get(descriptor);
				
				if (!EventTriggerConditionType::get_condition_status(condition, MetaAny {}, registry, entity))
				{
					// Step forward the length of the if-block.
					// (+ the current value of 1 for this instruction)
					step_stride += control_block.execution_range.size;
				}

				// NOTE: When the condition is met, we continue to the next instruction regularly.
			},

			[&](const MultiControlBlock& control_block)
			{
				// Manually progress by one instruction so that we
				// don't loop indefinitely on this multi-instruction.
				thread.next_instruction++;
				
				// NOTE: This target index accounts for this instruction as well.
				auto target_instruction_index = (thread.next_instruction + control_block.included_instructions.size);

				// Continue executing instructions until we reach the end of the multi-instruction block.
				while ((thread.next_instruction < target_instruction_index) && !thread.is_suspended())
				{
					// NOTE: Recursion.
					step_thread(registry, entity, descriptor, source, thread_comp, thread);
				}

				// Stride already handled via multi-instruction loop.
				step_stride = 0;
			}
		);
		//*/

		auto updated_instruction_index = (thread.next_instruction + step_stride);

		// NOTE: In the event the thread is suspended, completion is deferred
		// until we attempt to process the next instruction.
		// (e.g. we unpause the thread, but there's nothing else left to do)
		//
		// NOTE: A similar check to this is handled at the beginning of this function as well.
		// (i.e. checking that we don't step over the range of available instructions)
		if ((!thread.is_suspended()) && (updated_instruction_index >= source.instructions.size()))
		{
			// NOTE: When execution is finished, we always point to
			// the last valid instruction prior to completion.
			// (Hence why we don't update `next_instruction` here).

			thread.is_complete = true;
		}
		else
		{
			thread.next_instruction = updated_instruction_index;
		}

		// Return the next instruction index to the caller.
		return updated_instruction_index;
	}

	EntityListener* EntitySystem::listen(MetaTypeID event_type_id)
	{
		auto& listener = listeners[event_type_id];

		if (listener.is_active())
		{
			return &listener;
		}

		auto event_type = resolve(event_type_id);

		if (!event_type)
		{
			print_warn("Unable to resolve meta-type for event: #{}", event_type_id);

			return nullptr;
		}

		auto connect_fn = event_type.func(hash("connect_meta_event"));

		if (!connect_fn)
		{
			print_warn("Unable to resolve connection function for meta event-listener -- type: #{}", event_type_id);

			return nullptr;
		}

		auto& listener_ref = reinterpret_cast<MetaEventListener&>(listener);

		auto result = connect_fn.invoke
		(
			{},

			entt::forward_as_meta(listener_ref),
			entt::forward_as_meta(this->service),

			// NOTE: We need to manually specify a type-qualified `std::nullopt`
			// to handle the (normally defaulted) `flags` parameter.
			entt::forward_as_meta(std::optional<MetaEventListenerFlags>(std::nullopt))
		);

		if (!result)
		{
			print_warn("Failed to invoke meta event-listener connection routine. -- type: #{}", event_type_id);

			//return nullptr;
		}

		return &listener;
	}
}