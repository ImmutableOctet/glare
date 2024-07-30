#include "entity_system.hpp"

#include "events.hpp"
#include "entity_instruction.hpp"
#include "entity_thread_description.hpp"
#include "entity_thread.hpp"
#include "entity_descriptor.hpp"
#include "entity_state.hpp"
#include "entity_state_action.hpp"
#include "entity_context.hpp"

#include "components/state_component.hpp"
#include "components/entity_thread_component.hpp"
#include "components/instance_component.hpp"
#include "components/entity_context_component.hpp"

#include "commands/commands.hpp"

#include <engine/service.hpp>
#include <engine/service_events.hpp>
#include <engine/events.hpp>
#include <engine/concurrency.hpp>
#include <engine/system_manager_interface.hpp>

#include <engine/components/relationship_component.hpp>

#include <engine/script/script_handle.hpp>
#include <engine/script/script.hpp>
#include <engine/script/script_fiber.hpp>
#include <engine/script/script_fiber_response.hpp>
#include <engine/script/script_runtime_impl.hpp>

//#include <engine/meta/meta.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/listen.hpp>
#include <engine/meta/events.hpp>
#include <engine/meta/traits.hpp>
#include <engine/meta/indirection.hpp>

#include <engine/meta/meta_evaluation_context.hpp>
#include <engine/meta/meta_evaluation_context_store.hpp>
#include <engine/meta/meta_variable_target.hpp>
#include <engine/meta/meta_variable_evaluation_context.hpp>
#include <engine/meta/meta_variable_storage_interface.hpp>

#include <engine/lod/update_level.hpp>
#include <engine/lod/events/on_batch_update.hpp>
#include <engine/lod/components/level_of_detail_component.hpp>

#include <engine/commands/function_command.hpp>
#include <engine/commands/expr_command.hpp>

#include <util/variant.hpp>

#include <optional>
#include <iterator>
#include <type_traits>
#include <tuple>
#include <utility>

#include <cassert>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	MetaVariableEvaluationContext EntitySystem::resolve_variable_context
	(
		Service* opt_service,
		Registry* opt_registry,
		Entity opt_entity,

		EntityThreadComponent* opt_thread_comp,
		EntityThread* opt_thread,

		EntityContextComponent* opt_context_comp,

		std::optional<MetaVariableScope> referenced_scope
	)
	{
		MetaVariableStorageInterface* local_variables = (opt_thread)
			? opt_thread->variables.get()
			: nullptr
		;

		MetaVariableStorageInterface* global_variables = (opt_thread_comp)
			? opt_thread_comp->global_variables.get()
			: nullptr
		;

		MetaVariableStorageInterface* context_variables = (opt_service)
			? opt_service->peek_universal_variables()
			: nullptr
		;

		// TODO: Implement universal variables.
		MetaVariableStorageInterface* universal_variables = {};

		if (referenced_scope)
		{
			switch (*referenced_scope)
			{
				case MetaVariableScope::Local:
					if (opt_thread)
					{
						local_variables = opt_thread->get_variables();
					}

					break;

				case MetaVariableScope::Global:
					if (opt_thread_comp)
					{
						global_variables = opt_thread_comp->get_global_variables();
					}

					break;

				case MetaVariableScope::Context:
					if (!opt_context_comp && opt_registry && (opt_entity != null))
					{
						opt_context_comp = &(opt_registry->get_or_emplace<EntityContextComponent>(opt_entity));
					}

					break;
				case MetaVariableScope::Universal:
					if (opt_service)
					{
						context_variables = opt_service->get_universal_variables().get();
					}

					break;
			}
		}

		if (!opt_context_comp && opt_registry && (opt_entity != null))
		{
			opt_context_comp = opt_registry->try_get<EntityContextComponent>(opt_entity);
		}

		if (opt_context_comp)
		{
			if (auto context = opt_context_comp->shared_context.get())
			{
				context_variables = &context->variables;
			}
		}

		return MetaVariableEvaluationContext
		{
			local_variables,
			global_variables,
			context_variables,
			universal_variables
		};
	}

	EntitySystem::EntitySystem
	(
		Service& service,
		SystemManagerInterface& system_manager,
		bool subscribe_immediately
	) :
		BasicSystem(service),
		system_manager(&system_manager)
	{
		if (subscribe_immediately)
		{
			subscribe();
		}
	}

	EntitySystem::EntitySystem
	(
		Service& service,
		bool subscribe_immediately
	) :
		BasicSystem(service),
		system_manager(nullptr)
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

		registry.on_construct<EntityContextComponent>().connect<&EntitySystem::on_context_init>(*this);

		registry.on_update<EntityThreadComponent>().connect<&EntitySystem::on_entity_threads_updated_generate_delayed_event>(*this);

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

		service.register_event<EntityThreadFiberSpawnCommand, &EntitySystem::on_fiber_thread_spawn_command>(*this);

		service.register_event<OnThreadEventCaptured, &EntitySystem::on_thread_event_captured>(*this);

		// Standard events:
		service.register_event<OnServiceUpdate,           &EntitySystem::on_update>(*this);
		service.register_event<OnServiceFixedUpdate,      &EntitySystem::on_fixed_update>(*this);
		service.register_event<OnBatchUpdate,             &EntitySystem::on_batch_update>(*this);
		service.register_event<OnStateChange,             &EntitySystem::on_state_change>(*this);
		service.register_event<OnStateActivate,           &EntitySystem::on_state_activate>(*this);
		service.register_event<OnComponentCreate,         &EntitySystem::on_component_create>(*this);
		service.register_event<OnComponentUpdate,         &EntitySystem::on_component_update>(*this);
		service.register_event<OnComponentDestroy,        &EntitySystem::on_component_destroy>(*this);
		service.register_event<OnParentChanged,           &EntitySystem::on_parent_changed>(*this);
		service.register_event<OnEntityThreadsUpdated,    &EntitySystem::on_entity_threads_updated>(*this);

		return true;
	}

	bool EntitySystem::on_unsubscribe(Service& service)
	{
		auto& registry = service.get_registry();

		registry.on_construct<StateComponent>().disconnect(this);
		registry.on_update<StateComponent>().disconnect(this);
		registry.on_destroy<StateComponent>().disconnect(this);

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

		return &(instance_details->get_descriptor());
	}

	std::optional<EntityStateIndex> EntitySystem::get_state_index(Entity entity) const
	{
		auto& registry = get_registry();

		if (const auto state_component = registry.try_get<StateComponent>(entity))
		{
			return state_component->state_index;
		}

		return std::nullopt;
	}

	std::optional<EntityStateIndex> EntitySystem::get_prev_state_index(Entity entity) const
	{
		auto& registry = get_registry();

		if (const auto state_component = registry.try_get<StateComponent>(entity))
		{
			return state_component->prev_state_index;
		}

		return std::nullopt;
	}

	const EntityState* EntitySystem::get_state_by_index(Entity entity, EntityStateIndex entity_state_index) const
	{
		if (const auto descriptor = get_descriptor(entity))
		{
			// TODO: Implement multi-entity state descriptions. (use of `get(*descriptor)`)
			return &(descriptor->states[entity_state_index].get(*descriptor));
		}

		return {};
	}

	const EntityState* EntitySystem::get_state(Entity entity) const
	{
		if (const auto current_state_index = get_state_index(entity))
		{
			return get_state_by_index(entity, *current_state_index);
		}

		return {};
	}

	const EntityState* EntitySystem::get_state(Entity entity, std::string_view state_name) const
	{
		return get_state(entity, hash(state_name));
	}

	const EntityState* EntitySystem::get_state(Entity entity, StringHash state_name) const
	{
		return get_state_by_id(entity, state_name);
	}

	const EntityState* EntitySystem::get_state_by_id(Entity entity, StringHash state_id) const
	{
		if (const auto descriptor = get_descriptor(entity))
		{
			if (const auto state_index = descriptor->get_state_index(state_id))
			{
				// TODO: Implement multi-entity state descriptions. (use of `get(*descriptor)`)
				return &(descriptor->states[*state_index].get(*descriptor));
			}
		}

		return {};
	}

	const EntityState* EntitySystem::get_prev_state(Entity entity) const
	{
		if (const auto prev_state_index = get_prev_state_index(entity))
		{
			return get_state_by_index(entity, *prev_state_index);
		}

		return {};
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

		const auto current_state = get_state_index(entity);

		const auto& from_state_index = current_state;
		
		// TODO: Implement multi-entity state descriptions. (use of `get(*descriptor)`)
		const EntityState* from_state = (from_state_index)
			? &(descriptor->states[*from_state_index].get(*descriptor))
			: nullptr
		;

		const std::optional<StringHash> from_state_id = (from_state)
			? from_state->name
			: std::nullopt
		;

		const auto& to_state_id = state_id;

		if (!to_state_id)
		{
			return false;
		}

		const auto to_state = descriptor->get_state(to_state_id); // auto*

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

	EntityListener* EntitySystem::listen(MetaTypeID event_type_id)
	{
		if (!event_type_id)
		{
			return {};
		}

		const auto event_type = resolve(event_type_id);

		return listen(event_type);
	}

	EntityListener* EntitySystem::listen(const MetaType& event_type)
	{
		if (!event_type)
		{
			return {};
		}

		const auto event_type_id = event_type.id();

		auto& listener = listeners[event_type_id];

		if (listener.is_active())
		{
			return &listener;
		}

		const auto connection_result = connect_listener
		(
			listener,
			event_type,

			*this->service,
			
			this->system_manager
		);

		if (!connection_result)
		{
			print_error("Failed to connect listener for type: {}", get_known_string_from_hash(event_type_id));
		}

		return &listener;
	}

	bool EntitySystem::activate_state(Entity entity, StringHash state_id) const
	{
		const auto* descriptor = get_descriptor(entity);

		if (!descriptor)
		{
			return false;
		}

		const auto target_state_index = get_state_index(entity);

		if (!target_state_index)
		{
			return false;
		}

		// TODO: Implement multi-entity state descriptions. (use of `get(*descriptor)`)
		const auto& target_state = descriptor->states[*target_state_index].get(*descriptor);

		auto& registry = get_registry();

		target_state.activate(*descriptor, registry, entity, *target_state_index, std::nullopt, false);

		return true;
	}

	void EntitySystem::on_update(const OnServiceUpdate& data)
	{
		progress_threads<EntityThreadCadence::Update, EntityThreadCadence::Multi>();
	}

	void EntitySystem::on_fixed_update(const OnServiceFixedUpdate& data)
	{
		progress_threads<EntityThreadCadence::FixedUpdate>();
	}

	void EntitySystem::on_batch_update(const OnBatchUpdate& data)
	{
		progress_threads<EntityThreadCadence::Update, EntityThreadCadence::Multi>(data.batch_update_level);
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

		// TODO: Implement multi-entity state descriptions. (use of `get(*descriptor)`)
		const auto& state = descriptor->states[state_index].get(*descriptor);

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

	void EntitySystem::on_entity_threads_updated_generate_delayed_event(Registry& registry, Entity entity)
	{
		// Enqueue the `OnEntityThreadsUpdated` event so that we don't process thread removals before other commands/events have been processed.
		service->queue_event<OnEntityThreadsUpdated>(entity);
	}

	void EntitySystem::on_entity_threads_updated(const OnEntityThreadsUpdated& update_event)
	{
		auto& registry = get_registry();

		const auto& entity = update_event.entity;

		auto& thread_component = registry.get<EntityThreadComponent>(entity);

		thread_component.erase_completed_threads();
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
	bool EntitySystem::thread_command_impl(const ThreadCommandType& thread_command, RangeCallback&& range_callback, IDCallback&& id_callback, std::string_view dbg_name, std::string_view dbg_name_past_tense, EventArgs&&... event_args)
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

			return false;
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
				
				/*
				if (!dbg_name.empty())
				{
					if (threads.size() == 1)
					{
						print("Entity #{}: Attempting to {} thread #{}...", entity, dbg_name, begin);
					}
					else
					{
						const auto end = threads.end_point();

						print("Entity #{}: Attempting to {} threads #{} through #{}...", entity, dbg_name, begin, (end - 1));
					}
				}
				*/

				const EntityDescriptor* descriptor = nullptr;

				bool result = false;

				if constexpr (std::is_invocable_v<RangeCallback, const ThreadCommandType&, EntityThreadComponent&, const EntityDescriptor&, const EntityThreadRange&>)
				{
					descriptor = get_descriptor(entity);

					assert(descriptor);

					if (!descriptor)
					{
						print_warn("Unable to resolve descriptor for Entity #{}.", entity);

						return;
					}

					result = static_cast<bool>(range_callback(thread_command, thread_component, *descriptor, threads));
				}
				else
				{
					result = static_cast<bool>(range_callback(thread_command, thread_component, threads));
				}

				if (result)
				{
					/*
					if (!dbg_name_past_tense.empty())
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
					*/
				}
				else
				{
					if (!dbg_name.empty())
					{
						print_warn("Entity #{}: Failed to {} any threads.", entity, dbg_name);
					}
				}

				if constexpr (!std::is_same_v<EventType, void>)
				{
					if (!descriptor)
					{
						descriptor = get_descriptor(entity);

						//assert(descriptor);

						if (!descriptor)
						{
							print_warn("Unable to resolve descriptor for Entity #{}.", entity);
						}
					}

					bool check_linked = true; // false;

					if constexpr (has_field_check_linked_v<ThreadCommandType>)
					{
						check_linked = thread_command.check_linked;
					}

					thread_component.enumerate_threads
					(
						threads,

						[this, entity, &thread_component, &descriptor, ...event_args = std::forward<EventArgs>(event_args)](const EntityThread& local_thread)
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
								
								(descriptor)
									? descriptor->get_thread_id(local_thread.thread_index)
									: EntityThreadID {}
								,

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
				/*
				if (!dbg_name.empty())
				{
					print("Entity #{}: Attempting to {} thread by ID (#{})...", entity, dbg_name, thread_id);
				}
				*/
				
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
					/*
					if (!dbg_name_past_tense.empty())
					{
						print("Entity #{}: Thread (#{}) {} successfully.", entity, thread_id, dbg_name_past_tense);
					}
					*/
				}
				else
				{
					/*
					if (!dbg_name.empty())
					{
						print_warn("Entity #{}: Failed to {} any threads.", entity, dbg_name);
					}
					*/
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

		return true;
	}

	void EntitySystem::on_thread_spawn_command(const EntityThreadSpawnCommand& thread_command)
	{
		thread_command_impl<true, OnThreadSpawn>
		(
			thread_command,
			
			[](const EntityThreadSpawnCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, const EntityThreadRange& threads) -> bool
			{
				return thread_component.start_threads(descriptor, threads, thread_command.state_index, thread_command.parent_thread_name, thread_command.restart_existing);
			},

			[](const EntityThreadSpawnCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id) -> bool
			{
				return thread_component.start_thread
				(
					descriptor, thread_id,
					thread_command.state_index,
					
					true, true,
					thread_command.restart_existing,

					thread_command.parent_thread_name
				);

				return false;
			},

			"start", "started"
		);
	}

	void EntitySystem::on_thread_stop_command(const EntityThreadStopCommand& thread_command)
	{
		// Execute an initial 'dry run' to generate `OnThreadTerminated` events.
		auto event_result = thread_command_impl<false, OnThreadTerminated>
		(
			thread_command,
			
			[](const EntityThreadStopCommand& thread_command, EntityThreadComponent& thread_component, const EntityThreadRange& threads)
			{
				return true;
			},

			[](const EntityThreadStopCommand& thread_command, EntityThreadComponent& thread_component, const EntityDescriptor& descriptor, EntityThreadID thread_id)
			{
				return true;
			},

			{}, {}
		);

		if (event_result)
		{
			thread_command_impl<false, void>
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

	void EntitySystem::on_fiber_thread_spawn_command(EntityThreadFiberSpawnCommand& thread_command)
	{
		auto& fiber = thread_command.fiber;

		if (!fiber)
		{
			return;
		}

		auto& registry = get_registry();

		const auto entity = thread_command.entity();

		registry.patch<EntityThreadComponent>
		(
			entity,
			
			[&](auto& thread_component)
			{
				thread_component.start_thread
				(
					std::move(fiber),

					thread_command.state_index,
					thread_command.thread_name,
					thread_command.parent_thread_name,
					thread_command.thread_flags,
					thread_command.script_handle
				);
			}
		);
	}

	void EntitySystem::on_thread_event_captured(const OnThreadEventCaptured& thread_event)
	{
		auto& registry = get_registry();

		const auto& entity = thread_event.entity;

		auto& thread_component = registry.get<EntityThreadComponent>(entity);

		const auto instance_component = registry.try_get<InstanceComponent>(entity);

		const EntityDescriptor* descriptor = (instance_component)
			? (&(instance_component->get_descriptor()))
			: nullptr
		;

		const auto& local_thread_index = thread_event.local_instance;

		auto& threads = thread_component.get_threads();
		auto& thread_entry = threads[local_thread_index];

		if (thread_entry.is_suspended()) // || thread_entry.is_complete (implied)
		{
			return;
		}

		const auto& active_fiber = thread_entry.active_fiber;

		if (!active_fiber.has_script_handle())
		{
			return;
		}

		const bool process_event_immediately = (active_fiber.process_events_exhaustively());

		if (!process_event_immediately)
		{
			return;
		}

		const auto thread_source_index = thread_entry.thread_index;
		const bool has_thread_source_index = (thread_source_index != ENTITY_THREAD_INDEX_INVALID);

		// TODO: Look into making this more generic with an additional ID-based lookup as a fallback.
		const EntityThreadDescription* thread_source = ((has_thread_source_index) && (descriptor))
			? (&(descriptor->get_thread(thread_entry.thread_index)))
			: nullptr
		;

		// TODO: Look into removing requirement for `descriptor`.
		assert(descriptor);

		step_thread(registry, entity, *descriptor, thread_source, thread_component, thread_entry);
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

	void EntitySystem::on_parent_changed(const OnParentChanged& parent_changed)
	{
		auto& registry = get_registry();

		on_parent_context_changed(registry, parent_changed);
	}

	void EntitySystem::on_context_init(Registry& registry, Entity entity)
	{
		auto& context_comp = registry.get<EntityContextComponent>(entity);

		auto* relationship_comp = registry.try_get<RelationshipComponent>(entity);

		if (!relationship_comp)
		{
			if (!context_comp.shared_context)
			{
				context_comp.shared_context = std::make_shared<EntityContext>();
			}

			return;
		}

		if (context_comp.shared_context)
		{
			realign_child_contexts(registry, entity, context_comp, *relationship_comp);
		}
		else
		{
			use_parent_context_or_generate(registry, entity, context_comp, *relationship_comp, true);
		}
	}

	void EntitySystem::realign_child_contexts(Registry& registry, Entity entity, EntityContextComponent& context_comp, const RelationshipComponent& relationship_comp)
	{
		relationship_comp.enumerate_children
		(
			registry,

			[&registry, &context_comp](Entity child, const RelationshipComponent& child_relationship, Entity next_child) -> bool
			{
				if (auto* child_context_comp = registry.try_get<EntityContextComponent>(child))
				{
					if (child_context_comp->shared_context != context_comp.shared_context)
					{
						context_comp.adopt(child_context_comp->shared_context);
						child_context_comp->shared_context = context_comp.shared_context;

						// Mark the child context-component as patched.
						registry.patch<EntityContextComponent>(child);

						return true;
					}
				}
				else
				{
					registry.emplace<EntityContextComponent>(child, context_comp.shared_context);
				}

				return true;
			},

			true
		);
	}

	void EntitySystem::on_parent_context_changed(Registry& registry, const OnParentChanged& parent_changed)
	{
		const auto& entity = parent_changed.entity;
		const auto& old_parent = parent_changed.from_parent;
		const auto& new_parent = parent_changed.to_parent;

		auto* context_comp = registry.try_get<EntityContextComponent>(entity);

		if (!context_comp)
		{
			return;
		}

		const auto& relationship_comp = registry.get<RelationshipComponent>(entity);

		auto* old_parent_context_comp = registry.try_get<EntityContextComponent>(old_parent);

		if (!old_parent_context_comp)
		{
			return;
		}

		auto& old_context = old_parent_context_comp->shared_context;

		if (!old_context)
		{
			return;
		}

		const bool new_parent_context_merged = use_parent_context_or_generate(registry, entity, *context_comp, relationship_comp, true, false);

		if (!new_parent_context_merged)
		{
			// Since we couldn't use/merge the new parent's context,
			// a new (empty) context has been generated for us.
			// 
			// Copy the old context into this new object.
			*context_comp->shared_context = *old_context;

			// NOTE: There's no need to realign children here, since
			// `use_parent_context_or_generate` has already done that for us.
		}

		// Mark `EntityContextComponent` as patched.
		registry.patch<EntityContextComponent>(entity);
	}

	bool EntitySystem::use_parent_context_or_generate(Registry& registry, Entity entity, EntityContextComponent& context_comp, const RelationshipComponent& relationship_comp, bool realign_children, bool keep_existing)
	{
		bool parent_context_found = false;

		relationship_comp.enumerate_parents
		(
			registry,
			
			[this, &registry, entity, &context_comp, &relationship_comp, realign_children, &parent_context_found](Entity parent_entity) -> bool
			{
				auto* parent_context = registry.try_get<EntityContextComponent>(parent_entity);

				if (parent_context)
				{
					if (parent_context->shared_context)
					{
						parent_context->adopt(context_comp.shared_context);
						context_comp.shared_context = parent_context->shared_context;

						// Mark our context-component as patched.
						registry.patch<EntityContextComponent>(entity);

						if (realign_children)
						{
							realign_child_contexts(registry, entity, context_comp, relationship_comp);
						}

						parent_context_found = true;

						return false;
					}
				}

				return true;
			}
		);

		if (!parent_context_found)
		{
			if ((!keep_existing) || (!context_comp.shared_context))
			{
				context_comp.shared_context = std::make_shared<EntityContext>();

				if (realign_children)
				{
					realign_child_contexts(registry, entity, context_comp, relationship_comp);
				}
			}
		}

		return false;
	}

	bool EntitySystem::try_resume_thread(EntityThread& thread)
	{
		if (thread.is_sleeping())
		{
			if (thread.has_fiber())
			{
				if (thread.active_fiber.has_script_handle())
				{
					auto& script = *thread.active_fiber.script;

					if (script.has_yield_continuation_predicate())
					{
						const auto& predicate = script.get_yield_continuation_predicate();

						if (predicate(script, {}))
						{
							if (thread.resume())
							{
								script.set_yield_continuation_predicate(nullptr);

								return true;
							}
						}
					}
				}
			}
		}


		return false;
	}

	template <EntityThreadCadence... target_cadence>
	std::size_t EntitySystem::progress_threads(std::optional<UpdateLevel> target_update_level) // EntityThreadCount
	{
		using namespace engine::instructions;

		auto& registry = get_registry();

		std::size_t threads_updated = 0; // EntityThreadCount

		registry.view<EntityThreadComponent>().each
		(
			[&](Entity entity, EntityThreadComponent& thread_component)
			{
				// Ensure that this entity has the correct level of detail:
				const auto level_of_detail_component = registry.try_get<LevelOfDetailComponent>(entity);

				if (target_update_level)
				{
					if (!level_of_detail_component)
					{
						return;
					}

					if (level_of_detail_component->update_level != (*target_update_level))
					{
						return;
					}
				}
				else
				{
					if (level_of_detail_component)
					{
						return;
					}
				}

				const auto instance_component = registry.try_get<InstanceComponent>(entity);

				// TODO: Allow `EntityThreadComponent` to work without `InstanceComponent`, then remove this assert.
				assert(instance_component);

				const EntityDescriptor* descriptor = (instance_component)
					? (&(instance_component->get_descriptor()))
					: nullptr
				;

				bool has_updated_thread = false;

				for (auto& thread_entry : thread_component.threads)
				{
					if (((thread_entry.cadence == target_cadence) || ...))
					{
						if (thread_entry.is_suspended()) // || thread_entry.is_complete (implied)
						{
							if (!try_resume_thread(thread_entry))
							{
								continue;
							}
						}

						const auto thread_source_index = thread_entry.thread_index;
						const bool has_thread_source_index = (thread_source_index != ENTITY_THREAD_INDEX_INVALID);

						// TODO: Look into making this more generic with an additional ID-based lookup as a fallback.
						const EntityThreadDescription* thread_source = ((has_thread_source_index) && (descriptor))
							? (&(descriptor->get_thread(thread_entry.thread_index)))
							: nullptr
						;

						auto step = [&]()
						{
							const auto updated_instruction_index = step_thread
							(
								registry,
								entity,
								
								// TODO: Make `descriptor` an optional pointer argument.
								*descriptor,
								thread_source,

								thread_component,
								thread_entry
							);

							thread_entry.next_instruction = updated_instruction_index;

							return updated_instruction_index;
						};

						const auto initial_thread_cadence = thread_entry.cadence;

						switch (initial_thread_cadence)
						{
							case EntityThreadCadence::Multi:
								while (true)
								{
									const auto initial_instruction_index = thread_entry.next_instruction;
									const auto updated_instruction_index = step();

									if (updated_instruction_index == initial_instruction_index)
									{
										break;
									}

									if (thread_entry.is_suspended()) // is_complete
									{
										break;
									}

									bool is_implicit_yield_instruction = false;

									if (thread_source)
									{
										const auto& current_instruction = thread_source->get_instruction(initial_instruction_index);

										// After handling a rewind instruction, pause execution until the next `progress_threads` call.
										// (Ensures continuous loops don't stall the application)
										switch (current_instruction.type_index())
										{
											case util::variant_index<EntityInstruction::InstructionType, Rewind>():
											{
												is_implicit_yield_instruction = true;

												break;
											}

											case util::variant_index<EntityInstruction::InstructionType, CadenceControlBlock>():
											{
												const auto& cadence_control_block = std::get<CadenceControlBlock>(current_instruction.value);

												if (cadence_control_block.cadence != initial_thread_cadence)
												{
													is_implicit_yield_instruction = true;
												}

												break;
											}
										}
									}

									if (is_implicit_yield_instruction)
									{
										break;
									}
								}

								break;

							default:
								step();

								break;
						}

						threads_updated++;

						if (thread_entry.is_complete)
						{
							if (auto local_thread_index = thread_component.get_local_index(thread_entry))
							{
								service->event<OnThreadComplete> // queue_event
								(
									entity,

									thread_entry.thread_index,
									thread_entry.thread_id,

									static_cast<OnThreadComplete::LocalThreadIndex>(*local_thread_index),

									thread_entry.next_instruction
								);
							}
						}

						has_updated_thread = true;
					}
				}

				if (has_updated_thread)
				{
					registry.patch<EntityThreadComponent>(entity);
				}
			}
		);

		return threads_updated;
	}

	EntityInstructionIndex EntitySystem::step_thread
	(
		Registry& registry,
		Entity entity,

		const EntityDescriptor& descriptor,
		const EntityThreadDescription* source,

		EntityThreadComponent& thread_comp,
		EntityThread& thread
	)
	{
		using namespace instructions;

		auto& system_manager = this->system_manager;

		if (source)
		{
			if (thread.next_instruction >= source->instructions.size())
			{
				// We've stepped beyond the scope of this thread; mark this thread as complete.
				thread.is_complete = true;

				if (!source->instructions.empty())
				{
					// Always point to the last valid instruction when complete.
					thread.next_instruction = static_cast<EntityInstructionIndex>(source->instructions.size()-1);
				}

				// Return the last valid instruction.
				return thread.next_instruction;
			}
		}

		auto& service = get_service();

		const EntityInstruction* instruction = (source) ?
			(&(source->instructions[thread.next_instruction]))
			: nullptr
		;

		const auto& thread_index = thread.thread_index;
		const auto& thread_id    = thread.thread_id;

		std::optional<IndirectMetaAny> coroutine_function = std::nullopt;

		auto get_variable_context = [&service, &registry, entity, &thread, &thread_comp] // this
		(std::optional<MetaVariableScope> referenced_scope=std::nullopt)
		{
			return resolve_variable_context(&service, &registry, entity, &thread_comp, &thread, {}, referenced_scope);
		};

		auto is_coroutine_expr = [](const auto& expr_content)
		{
			if (const auto as_direct_function_call = expr_content.try_cast<MetaFunctionCall>())
			{
				if (const auto target_function = as_direct_function_call->get_function())
				{
					const auto has_overloads = static_cast<bool>(target_function.next());

					// Only allow coroutine forwarding on unambiguous calls.
					if (!has_overloads)
					{
						return type_is_coroutine(target_function.ret());
					}
				}
			}

			return false;
		};

		auto handle_coroutine_result = [&thread](auto&& result, auto&& on_coroutine)
		{
			if (auto fiber = result.try_cast<ScriptFiber>())
			{
				if (*fiber)
				{
					thread.active_fiber = { std::move(*fiber), {} };

					on_coroutine(thread.active_fiber);

					return true;
				}
			}

			return false;
		};

		auto execute_function_impl = [&registry, entity, &service, &system_manager, &get_variable_context, &handle_coroutine_result](auto&& expr_content, auto&& on_coroutine, auto&&... additional_args)
		{
			auto variable_context = get_variable_context();

			auto result = execute_opaque_function
			(
				expr_content,

				entt::forward_as_meta(std::forward<decltype(additional_args)>(additional_args))...,

				registry,
				entity,

				MetaEvaluationContext
				{
					&variable_context,
					&service,
					system_manager
				}
			);

			// Handle any resulting coroutines, if any.
			handle_coroutine_result(result, std::forward<decltype(on_coroutine)>(on_coroutine));

			return result;
		};

		auto execute_function = [&descriptor, &execute_function_impl](auto&& expr, auto&& on_coroutine, auto&&... additional_args)
		{
			if constexpr (std::is_same_v<std::decay_t<decltype(expr)>, IndirectMetaAny>)
			{
				return execute_function_impl(expr.get(descriptor.get_shared_storage()), std::forward<decltype(on_coroutine)>(on_coroutine), std::forward<decltype(additional_args)>(additional_args)...);
			}
			else
			{
				return execute_function_impl(std::forward<decltype(expr)>(expr), std::forward<decltype(on_coroutine)>(on_coroutine), std::forward<decltype(additional_args)>(additional_args)...);
			}
		};

		auto start_fiber = [&thread, &execute_function](auto&& expr) -> EntityThreadFiber&
		{
			if (!thread.active_fiber.exists())
			{
				execute_function
				(
					std::forward<decltype(expr)>(expr),

					[](auto& fiber)
					{
						// Briefly resume the fiber to handle the initialization step,
						// updating `fiber.script` in the process.
						fiber();
					},

					thread.active_fiber.script
				);

				if (thread.active_fiber.script)
				{
					thread.active_fiber.script->set_executing_thread(thread);
				}
			}

			return thread.active_fiber;
		};

		auto stop_fiber = [&thread]() -> EntityThreadFiber&
		{
			thread.active_fiber = {};

			return thread.active_fiber;
		};

		auto restart_fiber = [&stop_fiber, &start_fiber](auto&& expr) -> EntityThreadFiber&
		{
			stop_fiber();

			return start_fiber(std::forward<decltype(expr)>(expr));
		};

		// Forwards `action` to `execute_action` with the appropriate parameters.
		auto on_action = [&registry, &service, &system_manager, entity, &descriptor, &get_variable_context](const auto& action, auto&&... args)
		{
			Entity target = null;

			if constexpr (!std::is_same_v<EntityStateUpdateAction, std::decay_t<decltype(action)>>)
			{
				target = entity;
			}

			auto variable_context = get_variable_context();

			execute_action
			(
				registry,
				service,
				descriptor,
				action,
				entity,
				target,

				std::nullopt,

				MetaEvaluationContext
				{
					.variable_context = &variable_context,
					.service = &service,
					.system_manager = system_manager
				},

				std::forward<decltype(args)>(args)...
			);
		};

		auto on_spawn_action = [&thread, &on_action](const EntityThreadSpawnAction& action)
		{
			on_action
			(
				// Thread spawn action.
				action,
							
				// Unused event object.
				MetaAny {},

				// The state the active `thread` is connected to. (If any)
				thread.state_index
			);
		};

		auto resolve_thread_instruction = [this, &registry, &descriptor, entity, thread_index, thread_id](const auto& thread_instruction) -> std::tuple<Entity, EntityThreadTarget>
		{
			const auto target_entity = thread_instruction.target_entity.get(registry, entity);
			
			EntityThreadTarget target_thread = thread_instruction.thread_id;

			if (target_entity == entity)
			{
				if (!target_thread)
				{
					if (thread_id)
					{
						target_thread = thread_id;
					}

					if (!target_thread)
					{
						if (thread_index != ENTITY_THREAD_INDEX_INVALID)
						{
							target_thread = EntityThreadRange { thread_index, 1 };
						}
					}
				}
			}

			return { target_entity, target_thread };
		};

		auto control_flow_command = [&service, entity, &resolve_thread_instruction] <typename CommandType>
		(const auto& thread_instruction, bool defer_event, auto&&... additional_args) -> bool
		{
			const auto [target_entity, target_thread] = resolve_thread_instruction(thread_instruction);

			assert(target_thread);

			if (!target_thread)
			{
				return false;
			}

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

						std::move(target_thread),

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

						std::move(target_thread),

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

		auto stop_thread = [&registry, &service, entity, &descriptor, &thread_comp, &thread, thread_index, &resolve_thread_instruction](const instructions::Stop& stop={})
		{
			const auto [target_entity, target_thread] = resolve_thread_instruction(stop);

			assert(target_thread);

			if (!target_thread)
			{
				return;
			}

			// NOTE: Additional force-pause logic added for safety.
			// 
			// This is mainly to cover scenarios where the event
			// produced does not execute immediately after this instruction.
			if (target_entity == entity)
			{
				if ((target_thread == thread.thread_id) || (target_thread == EntityThreadRange { thread.thread_index, 1 }))
				{
					// Force-pause while awaiting 'stop' command's execution.
					thread.pause();
				}
			}

			// Enqueue deferred 'stop' command for this thread.
			service.queue_event<EntityThreadStopCommand>
			(
				entity, target_entity,

				target_thread,

				stop.check_linked
			);
		};

		auto condition_met = [&registry, &service, &system_manager, entity, &get_variable_context](auto&& condition)
		{
			auto variable_context = get_variable_context();

			return EventTriggerConditionType::get_condition_status
			(
				std::forward<decltype(condition)>(condition),

				MetaAny {},

				registry, entity,

				MetaEvaluationContext
				{
					.variable_context = &variable_context,
					.service = &service,
					.system_manager = system_manager
				}
			);
		};

		auto is_coroutine_instruction = [](const auto& instruction) -> bool
		{
			switch (instruction.type_index())
			{
				case util::variant_index<EntityInstruction::InstructionType, CoroutineCall>():
					return true;
			}

			return false;
		};

		auto start_listening = [this, entity](auto type_id) -> bool
		{
			auto listener = listen(type_id);
								
			if (!listener)
			{
				if (type_id)
				{
					print_warn("Failed locate listener while processing yield request.");
				}

				return false;
			}

			// See `EntityListener::update_entity` for corresponding `remove_entity` usage.
			listener->add_entity(entity);

			return true;
		};

		EntityInstructionCount step_stride = 1;

		auto exec = [&](auto&& instruction_value) -> void
		{
			auto exec_impl = [&](auto&& instruction_value, auto&& exec_impl_recursive) -> void
			{
				// Convenience wrapper for instruction delegation via recursion.
				// Handles both wrapped (variant / visit-able) and unwrapped (direct usage) instruction values.
				auto instruct = [&exec_impl_recursive](auto&& instruction)
				{
					using underlying_instruction_t = std::remove_reference_t<decltype(instruction)>;

					if constexpr (std::is_same_v<underlying_instruction_t, EntityInstruction>)
					{
						exec_impl_recursive(instruction.value, exec_impl_recursive);
					}
					else if constexpr (std::is_same_v<underlying_instruction_t, EntityInstruction::InstructionType>)
					{
						exec_impl_recursive(std::forward<decltype(instruction)>(instruction), exec_impl_recursive);
					}
					else
					{
						exec_impl_recursive(EntityInstruction::InstructionType { std::forward<decltype(instruction)>(instruction) }, exec_impl_recursive);
					}
				};

				util::visit
				(
					instruction_value,

					[](const NoOp&) {},

					[&on_action](const EntityStateAction& action)           { on_action(action); },
					[&on_action](const EntityStateTransitionAction& action) { on_action(action); },
					[&on_action](const EntityStateCommandAction& action)    { on_action(action); },
					[&on_action](const EntityStateUpdateAction& action)     { on_action(action); },
					[&on_action](const EntityThreadStopAction& action)      { on_action(action); },
					[&on_action](const EntityThreadPauseAction& action)     { on_action(action); },
					[&on_action](const EntityThreadResumeAction& action)    { on_action(action); },
					[&on_action](const EntityThreadAttachAction& action)    { on_action(action); },
					[&on_action](const EntityThreadDetachAction& action)    { on_action(action); },
					[&on_action](const EntityThreadUnlinkAction& action)    { on_action(action); },
					[&on_action](const EntityThreadSkipAction& action)      { on_action(action); },
					[&on_action](const EntityThreadRewindAction& action)    { on_action(action); },

					[&on_spawn_action](const EntityThreadSpawnAction& action) { on_spawn_action(action); },

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
						const auto local_index = thread_comp.get_local_index(thread);

						assert(local_index);

						if (!local_index)
						{
							return;
						}

						auto result = thread_comp.link_local_thread(*local_index);

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

					[&registry, &service, entity, &descriptor, &resolve_thread_instruction](const Sleep& sleep) // [..., &thread]
					{
						const auto source_entity = entity;
						const auto [target_entity, target_thread] = resolve_thread_instruction(sleep);
				
						assert(target_thread);

						if (!target_thread)
						{
							return;
						}

						// Trigger an event to immediately pause this thread.
						// 
						// NOTE: This event must happen immediately to ensure compatibility with multi-instructions.
						service.event<EntityThreadPauseCommand> // queue_event
						(
							source_entity, target_entity,

							target_thread,

							sleep.check_linked
						);

						// Alternative:
						//thread.pause();

						// Enqueue a timed 'resume' event to trigger after the sleep duration.
						service.timed_event<EntityThreadResumeCommand>
						(
							sleep.duration,

							source_entity, target_entity,

							target_thread,

							sleep.check_linked
						);
					},

					[this, entity, &descriptor, &thread, &condition_met, &start_listening, &step_stride](const Yield& yield)
					{
						const auto& condition_raw = yield.condition.get(descriptor);
						
						// TODO: Review whether 'yield as no-op' makes sense;
						// I'm not sure if this behavior is the most intuitive.
						if (condition_met(condition_raw))
						{
							return;
						}

						bool yielded_successfully = false;

						EventTriggerConditionType::visit_type_enabled
						(
							condition_raw,

							[this, entity, &descriptor, &start_listening, &yielded_successfully](const auto& condition)
							{
								condition.enumerate_types
								(
									descriptor,

									[this, entity, &start_listening, &yielded_successfully](MetaTypeID type_id)
									{
										if (start_listening(type_id))
										{
											yielded_successfully = true;
										}
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

						thread.yield();

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
						const auto [target_entity, target_thread] = resolve_thread_instruction(rewind);

						assert(target_thread);

						if (!target_thread)
						{
							return;
						}

						if ((target_entity == entity) && !rewind.thread_id)
						{
							step_stride = 0;
						}

						service.event<EntityThreadRewindCommand> // queue_event
						(
							source_entity, target_entity,
					
							target_thread,
					
							rewind.check_linked,
							rewind.instructions_rewound
						);
					},

					[&descriptor, &condition_met, &step_stride](const IfControlBlock& control_block)
					{
						const auto& condition = control_block.condition.get(descriptor);

						const auto result = condition_met(condition);

						if (!result)
						{
							// Step forward the length of the if-block.
							// (+ the current value of 1 for this instruction)
							step_stride += control_block.execution_range.size;
						}

						// NOTE: When the condition is met, we continue to the next instruction regularly.
					},

					[&](const MultiControlBlock& control_block)
					{
						if (control_block.included_instructions.size > 0)
						{
							// Manually progress by one instruction so that we
							// don't loop indefinitely on this multi-instruction.
							thread.next_instruction++;
				
							// NOTE: This target index accounts for this instruction as well.
							auto target_instruction_index = (thread.next_instruction + control_block.included_instructions.size);

							// Continue executing instructions until we reach the end of the multi-instruction block.
							while ((thread.next_instruction < target_instruction_index) && !thread.is_suspended())
							{
								const auto active_instruction = thread.next_instruction;

								// NOTE: Recursion.
								step_thread(registry, entity, descriptor, source, thread_comp, thread);

								const auto updated_instruction = thread.next_instruction;

								// If the updated instruction index is less than the active instruction, we're processing a loop,
								// and should therefore continue execution on the next update tick (`step_thread` call).
								// 
								// i.e. from here, it is up to the runtime environment to determine if we should continue looping.
								if (updated_instruction < active_instruction)
								{
									break;
								}
							}

							// Stride already handled via multi-instruction loop.
							step_stride = 0;
						}
					},

					[&instruct, &thread](const CadenceControlBlock& control_block)
					{
						switch (control_block.cadence)
						{
							case EntityThreadCadence::Multi:
								if (control_block.included_instructions.size > 0)
								{
									instruct
									(
										MultiControlBlock
										{
											control_block.included_instructions
										}
									);
								}

								thread.cadence = EntityThreadCadence::Multi; // control_block.cadence;

								break;

							default:
								thread.cadence = control_block.cadence;

								break;
						}
					},

					[&instruct](const FunctionCall& function_call)
					{
						// NOTE: Currently forwards to `AdvancedMetaExpression`. (This may change later)
						instruct(AdvancedMetaExpression { function_call.function });
					},

					[&instruct, &registry, &service, &system_manager, entity, &descriptor, &thread, &start_fiber, &stop_fiber, &restart_fiber, &coroutine_function](const CoroutineCall& coroutine_call)
					{
						if (auto& fiber = start_fiber(coroutine_call.coroutine_function); fiber.exists())
						{
							coroutine_function = coroutine_call.coroutine_function;
						}
					},

					[&descriptor, &is_coroutine_expr, &execute_function, &instruct](const AdvancedMetaExpression& advanced_expr)
					{
						auto expr_content = advanced_expr.expr.get(descriptor.get_shared_storage());

						if (is_coroutine_expr(expr_content))
						{
							// If this is a direct call to a coroutine, handle this as a `CoroutineCall` instruction instead.
							instruct(CoroutineCall { advanced_expr.expr });
						}
						else
						{
							execute_function(expr_content, [](auto&&...) {});
						}
					},

					[&registry, &service, entity, &descriptor, &thread, &get_variable_context](const VariableDeclaration& variable_declaration)
					{
						const auto& variable_scope = variable_declaration.variable_details.scope;

						if (auto variable_context = get_variable_context(variable_scope))
						{
							const auto& resolved_variable_name = variable_declaration.variable_details.name;

							const auto result = variable_context.declare(variable_scope, resolved_variable_name);

							if (!result)
							{
								if (variable_context.exists(variable_scope, resolved_variable_name))
								{
									//print_warn("Variable declaration for \"#{}\" (Scope: {}) skipped due to existing value.", resolved_variable_name, variable_scope);
								}
								else
								{
									print_warn("Failed to declare variable: \"#{}\" (Scope: {})", resolved_variable_name, variable_scope);
								}
							}
						}
						else
						{
							print_warn("Unable to process variable instruction: Empty variable evaluation context.");
						}
					},

					[&registry, &service, &system_manager, entity, &descriptor, &thread, &thread_comp, thread_index, &get_variable_context](const VariableAssignment& variable_assignment)
					{
						auto& source_descriptor = descriptor;

						auto as_remote_variable = [source=entity, &service, &system_manager, &registry, &variable_assignment, &source_descriptor, &get_variable_context]()
						{
							const auto& target = variable_assignment.target_entity.resolve(registry, source);

							if (target == null)
							{
								print_warn("Failed to assign variable for remote entity: `null` entity detected.", target);

								return;
							}

							auto* target_thread_comp = registry.try_get<EntityThreadComponent>(target);

							if (!target_thread_comp)
							{
								print_warn("Failed to assign variable for: Entity #{} -- Unable to find `EntityThreadComponent`.", target);

								return;
							}

							auto* target_instance_comp = registry.try_get<InstanceComponent>(target);

							if (!target_instance_comp)
							{
								print_warn("Failed to assign variable for: Entity #{} -- Unable to find `InstanceComponent`.", target);

								return;
							}

							auto& target_descriptor = target_instance_comp->get_descriptor();

							EntityThread* target_thread = (variable_assignment.thread_id)
								? target_thread_comp->get_thread(target_descriptor, variable_assignment.thread_id)
								: nullptr
							;

							const auto target_variable_scope = (variable_assignment.variable_details)
								? variable_assignment.variable_details->scope
								: MetaVariableScope::Local
							;

							auto target_variable_context = resolve_variable_context
							(
								&service,
								&registry,
								target,
								target_thread_comp,
								target_thread,
								{},
								target_variable_scope
							);

							if (!target_variable_context)
							{
								return;
							}

							if (variable_assignment.variable_details)
							{
								const auto& target_variable_name = variable_assignment.variable_details->name;

								auto existing = target_variable_context.get_ptr(target_variable_scope, target_variable_name);

								if (existing)
								{
									if (variable_assignment.ignore_if_already_assigned)
									{
										if (*existing)
										{
											return;
										}
									}
								}
								else if (variable_assignment.ignore_if_not_declared)
								{
									return;
								}
							}

							auto source_variable_context = get_variable_context(target_variable_scope); // MetaVariableScope::Local

							auto source_evaluation_context = MetaEvaluationContext
							{
								.variable_context = &source_variable_context,
								.service = &service,
								.system_manager = system_manager
							};

							auto assignment_expr = variable_assignment.assignment.get(source_descriptor.get_shared_storage());

							auto result = execute_opaque_expression
							(
								assignment_expr,
								registry, source,
								source_evaluation_context
							);

							if (result && variable_assignment.variable_details)
							{
								if (!target_thread)
								{
									return;
								}

								const auto target_local_index = target_thread_comp->get_local_index(*target_thread);

								if (!target_local_index)
								{
									return;
								}

								auto target_evaluation_context = MetaEvaluationContext
								{
									.variable_context = &target_variable_context,
									.service = &service,
									.system_manager = system_manager
								};

								// TODO: Look into making some of these fields optional.
								service.queue_event<OnThreadVariableUpdate> // event
								(
									target,

									target_thread->thread_index,
									variable_assignment.thread_id,

									static_cast<ThreadEvent::LocalThreadIndex>(*target_local_index),
									target_thread->next_instruction,

									variable_assignment.variable_details->name,
									variable_assignment.variable_details->scope,

									get_indirect_value_or_ref
									(
										result,
										registry, target,
										target_evaluation_context
									)
								);
							}
						};

						auto as_thread_local_variable = [&]()
						{
							if (auto variable_context = get_variable_context(((variable_assignment.variable_details) ? variable_assignment.variable_details->scope : std::optional<MetaVariableScope>(std::nullopt))))
							{
								if (variable_assignment.variable_details)
								{
									const auto& variable_scope = variable_assignment.variable_details->scope;
									const auto& variable_name = variable_assignment.variable_details->name;

									auto existing = variable_context.get_ptr(variable_scope, variable_name);

									if (existing)
									{
										if (variable_assignment.ignore_if_already_assigned)
										{
											if (*existing)
											{
												return;
											}
										}
									}
									else if (variable_assignment.ignore_if_not_declared)
									{
										return;
									}
								}

								auto assignment_expr = variable_assignment.assignment.get(descriptor.get_shared_storage());

								auto evaluation_context = MetaEvaluationContext
								{
									.variable_context = &variable_context,
									.service = &service,
									.system_manager = system_manager
								};

								auto result = execute_opaque_expression
								(
									assignment_expr,
									registry, entity,
									evaluation_context
								);

								if (result && variable_assignment.variable_details)
								{
									const auto local_index = thread_comp.get_local_index(thread);

									assert(local_index);

									if (!local_index)
									{
										return;
									}

									service.queue_event<OnThreadVariableUpdate> // event
									(
										entity,

										thread_index,
										EntityThreadID {},
										static_cast<ThreadEvent::LocalThreadIndex>(*local_index),
										thread.next_instruction,

										variable_assignment.variable_details->name,
										variable_assignment.variable_details->scope,

										get_indirect_value_or_ref
										(
											result,
											registry, entity,
											evaluation_context
										)
									);
								}
							}
						};

						if (variable_assignment.target_entity.is_self_targeted()) // (variable_assignment.target_entity.resolve(registry, entity) != entity)
						{
							if (variable_assignment.thread_id)
							{
								if (const auto self_thread_id = descriptor.get_thread_id(thread.thread_index); (variable_assignment.thread_id == self_thread_id))
								{
									as_thread_local_variable();
								}
								else
								{
									as_remote_variable();
								}
							}
							else
							{
								as_thread_local_variable();
							}
						}
						else
						{
							as_remote_variable();
						}
					},

					// NOTE: This instruction normally follows a yield instruction, and is skipped if an event has been provided.
					// This implementation acts as a fallback in the case of forced continuation, or an incompatible event type.
					[&get_variable_context](const EventCapture& event_capture)
					{
						// Implemented by external source. (e.g. `EntityListener`)
						if (auto variable_context = get_variable_context(event_capture.variable_details.scope))
						{
							const auto& variable_scope = event_capture.variable_details.scope;
							const auto& variable_name  = event_capture.variable_details.name;

							MetaType intended_type = (event_capture.intended_type)
								? resolve(event_capture.intended_type)
								: MetaType {}
							;

							if (!intended_type)
							{
								if (auto existing = variable_context.get_ptr(variable_scope, variable_name))
								{
									intended_type = existing->type();
								}
							}

							if (intended_type)
							{
								// Attempt to default-construct the intended type:
								if (auto default_constructed = intended_type.construct())
								{
									variable_context.set(variable_scope, variable_name, std::move(default_constructed));

									return;
								}
							}

							// Fallback to assigning an empty value.
							variable_context.set(variable_scope, variable_name, MetaAny {});
						}
					},

					[&descriptor, &condition_met, &stop_thread](const instructions::Assert& assert_instruction)
					{
						const auto& condition = assert_instruction.condition.get(descriptor);

						const auto result = condition_met(condition);

						if (!result)
						{
							//assert(false);

							if (assert_instruction.debug_message)
							{
								const auto& debug_message = assert_instruction.debug_message->get(descriptor);

								if (assert_instruction.condition_representation)
								{
									const auto& condition_representation = assert_instruction.condition_representation->get(descriptor);

									print_error("Assert failed: \"{}\" ({})", debug_message, condition_representation);
								}
								else
								{
									print_error("Assert failed: \"{}\"", debug_message);
								}
							}
							else
							{
								if (assert_instruction.condition_representation)
								{
									const auto& condition_representation = assert_instruction.condition_representation->get(descriptor);

									print_error("Assert failed: ({})", condition_representation);
								}
								else
								{
									print_error("Assert failed");
								}
							}

							print_warn("Unable to continue execution of thread; terminating...");

							// TODO: Look into alternative methods for sending 'stop' signal.
							stop_thread();
						}

						// NOTE: If the assert condition was met, we continue to the next instruction regularly.
					},

					[&instruct, &get_variable_context, &registry, entity, &service, &system_manager](const InstructionDescriptor& instruction_desc)
					{
						auto result = MetaAny {};

						if (auto variable_context = get_variable_context())
						{
							auto evaluation_context = MetaEvaluationContext
							{
								.variable_context = &variable_context,
								.service = &service,
								.system_manager = system_manager
							};

							result = try_get_underlying_value(instruction_desc.instruction, registry, entity, evaluation_context);
						}
						else
						{
							result = try_get_underlying_value(instruction_desc.instruction, registry, entity);
						}

						if (result)
						{
							if (auto new_instruction = EntityInstruction(std::move(result)))
							{
								instruct(new_instruction.value);
							}
						}
						else
						{
							print_warn("Failed to resolve instruction; continuing anyway.");
						}
					}
				);
			};

			exec_impl(std::forward<decltype(instruction_value)>(instruction_value), exec_impl);
		};

		auto sleep_for = [&exec](const Timer::Duration& duration)
		{
			exec(EntityInstruction::InstructionType { Sleep { EntityTarget {}, EntityThreadID {}, duration}});
		};

		auto& fiber = thread.active_fiber;

		if (instruction)
		{
			if ((!fiber.exists()) || (is_coroutine_instruction(*instruction)))
			{
				exec(instruction->value);
			}
		}

		if (fiber.exists())
		{
			using ResponseToken = ScriptControlFlowToken;

			auto control_flow_response = ResponseToken::Default;

			if (fiber.script)
			{
				impl::set_running_script(fiber.script);
			}

			const auto result = fiber();

			if (fiber.script)
			{
				assert(impl::get_running_script() == fiber.script);

				impl::clear_running_script();
			}

			if (fiber.done())
			{
				control_flow_response = ResponseToken::Complete;
			}
			else if (result)
			{
				util::visit
				(
					*result,

					[](const std::monostate&) {},

					[&control_flow_response](const ResponseToken& token) { control_flow_response = token; },

					[&on_action](const EntityStateAction& action) { on_action(action); },

					[this, entity, &fiber, &start_listening, &thread, &step_stride](const EventYieldRequest& yield_request)
					{
						const auto type_id = yield_request.event_type_id;

						const bool listen_result = start_listening(type_id);

						assert(listen_result);

						if (listen_result)
						{
							assert(fiber.script);

							if (fiber.script)
							{
								fiber.script->set_pending_event_type(type_id);
							}

							thread.yield();

							step_stride = 0;
						}
					},

					[&on_action](const EntityStateHash& next_state)     { on_action(EntityStateTransitionAction { next_state }); },
					[&on_action](const entt::hashed_string& next_state) { on_action(EntityStateTransitionAction { static_cast<EntityStateHash>(next_state) }); },

					[&on_spawn_action](const EntityThreadSpawnAction& action) { on_spawn_action(action); },
					
					[&exec](const EntityInstruction& instruction)       { exec(instruction.value); },

					[&sleep_for](const Timer& timer)                    { sleep_for(timer.remaining());            },
					[&sleep_for](const Timer::Duration& duration)       { sleep_for(duration);                     },
					[&sleep_for](const Timer::Days& duration)           { sleep_for(Timer::to_duration(duration)); },
					[&sleep_for](const Timer::Hours& duration)          { sleep_for(Timer::to_duration(duration)); },
					[&sleep_for](const Timer::Minutes& duration)        { sleep_for(Timer::to_duration(duration)); },
					[&sleep_for](const Timer::Seconds& duration)        { sleep_for(Timer::to_duration(duration)); },
					[&sleep_for](const Timer::Milliseconds& duration)   { sleep_for(Timer::to_duration(duration)); },
					[&sleep_for](const Timer::Microseconds& duration)   { sleep_for(Timer::to_duration(duration)); },
					[&sleep_for](const Timer::FloatSeconds& duration)   { sleep_for(Timer::to_duration(duration)); },
					[&sleep_for](const Timer::DoubleSeconds& duration)  { sleep_for(Timer::to_duration(duration)); }
				);
			}
							
			switch (control_flow_response)
			{
				case ResponseToken::NextUpdate:
					step_stride = 0;

					break;

				case ResponseToken::Restart:
					step_stride = 0;

					if (coroutine_function)
					{
						restart_fiber(*coroutine_function);
					}

					break;

				case ResponseToken::UntilWake:
					exec((EntityInstruction { Pause {} }).value);

					step_stride = 0;

					break;

				case ResponseToken::Complete:
					stop_fiber();

					break;
			}
		}

		auto updated_instruction_index = thread.next_instruction;

		if (source)
		{
			updated_instruction_index += step_stride;

			// NOTE: In the event the thread is suspended, completion is deferred
			// until we attempt to process the next instruction.
			// (e.g. we unpause the thread, but there's nothing else left to do)
			//
			// NOTE: A similar check to this is handled at the beginning of this function as well.
			// (i.e. checking that we don't step over the range of available instructions)
			if ((!thread.is_suspended()) && (updated_instruction_index >= source->instructions.size()))
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
		}
		else
		{
			if (!thread.is_suspended())
			{
				if (!fiber)
				{
					thread.is_complete = true;
				}
			}
		}

		// Return the next instruction index to the caller.
		return updated_instruction_index;
	}
}