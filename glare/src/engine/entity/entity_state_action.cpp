#include "entity_state_action.hpp"

#include "entity_descriptor.hpp"
#include "components/instance_component.hpp"
#include "commands/commands.hpp"

#include <engine/service.hpp>
//#include <engine/resource_manager/resource_manager.hpp>

#include <engine/commands/component_patch_command.hpp>
#include <engine/commands/indirect_component_patch_command.hpp>
#include <engine/commands/component_replace_command.hpp>

//#include <engine/meta/hash.hpp>
//#include <engine/meta/function.hpp>
#include <engine/meta/data_member.hpp>

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateTransitionAction& transition,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<StateChangeCommand>
		(
			delay,
			source, target,
			transition.state_name
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateCommandAction& command,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		//const auto& command_descriptor = command.command;
		const auto& command_descriptor = command.command.get(descriptor);

		auto command_instance = (event_instance)
			? command_descriptor.instance(true, event_instance, registry, source, context)
			: command_descriptor.instance(true, registry, source, context)
		;

		assert(command_instance);
		assert(command_instance.type());

		set_data_member(command_instance, "source", source);
		set_data_member(command_instance, "target", target);

		// Debugging related:
		//const auto* cmd = reinterpret_cast<const engine::Command*>(command_instance.data());

		service.timed_event(delay, std::move(command_instance));
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateUpdateAction& update,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance,

		bool defer_evaluation,
		bool use_member_assignment_on_patch
	)
	{
		if (auto redirected_target = update.target_entity.get(registry, ((target == null) ? source : target)); redirected_target != null)
		{
			target = redirected_target;
		}

		for (const auto& component_entry : update.updated_components.type_definitions)
		{
			const auto& component = component_entry.get(descriptor);

			if (defer_evaluation)
			{
				// Attempt to patch the component in-place, rather than reconstructing it.
				// NOTE: Raw pointer type. (May replace with `std::shared_ptr` later)
				service.timed_event<IndirectComponentPatchCommand>
				(
					delay,
					source, target,
					&component,

					// NOTE: Technically unsafe due to usage of raw pointers with assumption of extended lifetime.
					// (We aren't currently storing shared pointers internally)
					MetaEvaluationContextStore { context },

					// NOTE: Potentially unsafe due to implied copy of event-instance.
					// (May reference stale data at time of execution)
					event_instance // event_instance.as_ref()
				);
			}
			else
			{
				auto instance = (event_instance)
					? component.instance(true, event_instance, registry, source, context)
					: component.instance(true, registry, source, context)
				;

				if (!instance)
				{
					continue;
				}

				if (component.forces_field_assignment())
				{
					// Patch the component according to the desired specifications.
					service.timed_event<ComponentPatchCommand>
					(
						delay,
						source, target,
						std::move(instance),
						use_member_assignment_on_patch
					);
				}
				else
				{
					// Replace/reconstruct the component according to the desired specifications.
					service.timed_event<ComponentReplaceCommand>
					(
						delay,
						source, target,
						std::move(instance)
					);
				}
			}
		}
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadSpawnAction& thread_spawn,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance,

		std::optional<EntityStateIndex> state_index
	)
	{
		service.timed_event<EntityThreadSpawnCommand>
		(
			delay,
			
			source, target,
			
			thread_spawn.threads,
			thread_spawn.restart_existing,

			state_index
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadStopAction& thread_stop,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<EntityThreadStopCommand>
		(
			delay,
			
			source, target,
			
			thread_stop.threads,
			thread_stop.check_linked
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadPauseAction& thread_pause,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<EntityThreadPauseCommand>
		(
			delay,
			
			source, target,
			
			thread_pause.threads,
			thread_pause.check_linked
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadResumeAction& thread_resume,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<EntityThreadResumeCommand>
		(
			delay,
			
			source, target,
			
			thread_resume.threads,
			thread_resume.check_linked
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadAttachAction& thread_attach,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<EntityThreadAttachCommand>
		(
			delay,
			
			source, target,
			
			thread_attach.threads,
			thread_attach.check_linked,
			thread_attach.state_id
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadDetachAction& thread_detach,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<EntityThreadDetachCommand>
		(
			delay,
			
			source, target,

			thread_detach.threads,
			thread_detach.check_linked
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadUnlinkAction& thread_detach,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<EntityThreadUnlinkCommand>
		(
			delay,
			
			source, target,

			thread_detach.threads
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadSkipAction& thread_skip,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<EntityThreadSkipCommand>
		(
			delay,
			
			source, target,
			
			thread_skip.threads,
			thread_skip.check_linked,
			thread_skip.instructions_skipped
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityThreadRewindAction& thread_rewind,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		service.timed_event<EntityThreadRewindCommand>
		(
			delay,
			
			source, target,

			thread_rewind.threads,
			thread_rewind.check_linked,
			thread_rewind.instructions_rewound
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateAction& action,
		Entity source, Entity target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		std::visit
		(
			[&](const auto& action)
			{
				execute_action
				(
					registry,
					service,
					descriptor,
					action,
					source,
					target,
					delay,
					context,
					event_instance
				);
			},

			action
		);
	}

	void execute_action
	(
		Registry& registry,
		Service& service,
		const EntityDescriptor& descriptor,
		const EntityStateAction& action,
		Entity source_entity, const EntityTarget& target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		auto target_entity = target.get(registry, source_entity);

		if (target_entity == null)
		{
			print_warn("Failed to resolve target entity.");

			return;
		}

		execute_action
		(
			registry,
			service,
			descriptor,
			action,
			source_entity,
			target_entity,
			delay,
			context,
			event_instance
		);
	}

	void execute_action
	(
		Service& service,
		const EntityStateAction& action,
		Entity source_entity, const EntityTarget& target,
		std::optional<engine::Timer::Duration> delay,
		const MetaEvaluationContext& context,
		const MetaAny& event_instance
	)
	{
		auto& registry = service.get_registry();

		const auto& instance_comp = registry.get<InstanceComponent>(source_entity);

		//auto& resource_manager = service.get_resource_manager();

		const auto& descriptor = instance_comp.get_descriptor();

		execute_action
		(
			registry,
			service,
			descriptor,
			action,
			source_entity,
			target,
			delay,
			context,
			event_instance
		);
	}
}