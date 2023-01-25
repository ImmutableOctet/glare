#pragma once

#include "reflection.hpp"

#include "types.hpp"
#include "events.hpp"

#include "entity_target.hpp"
#include "entity_descriptor.hpp"
#include "entity_thread_target.hpp"
#include "event_trigger_condition.hpp"
#include "state_storage_manager.hpp"
#include "entity_thread_range.hpp"

#include "components/instance_component.hpp"
#include "components/state_component.hpp"
#include "components/state_storage_component.hpp"
#include "components/frozen_state_component.hpp"
#include "components/entity_thread_component.hpp"

#include "commands/commands.hpp"

#include <string>
#include <string_view>

namespace engine
{
	template <>
    void reflect<EntityTarget>()
    {
        engine_meta_type<EntityTarget>()
            .data<nullptr, &EntityTarget::target_index>("target_index"_hs)

            .data<nullptr, &EntityTarget::is_self_targeted>("is_self_targeted"_hs)
            .data<nullptr, &EntityTarget::is_parent_target>("is_parent_target"_hs)
            .data<nullptr, &EntityTarget::is_exact_entity_target>("is_exact_entity_target"_hs)
            .data<nullptr, &EntityTarget::is_entity_name_target>("is_entity_name_target"_hs)
            .data<nullptr, &EntityTarget::is_child_target>("is_child_target"_hs)
            .data<nullptr, &EntityTarget::is_player_target>("is_player_target"_hs)
            .data<nullptr, &EntityTarget::is_null_target>("is_null_target"_hs)
            
            .func<&EntityTarget::get>("resolve"_hs)

            // NOTE: Alias to `resolve`.
            .func<&EntityTarget::get>("get"_hs)

            //.func<&EntityTarget::parse_type>("parse_type"_hs)
        ;
    }

	template <>
	void reflect<EntityDescriptor>()
	{
		//engine_empty_meta_type<EntityDescriptor>() // engine_meta_type
			//.data<&EntityDescriptor::components>("components"_hs)
			//.data<&EntityDescriptor::states>("states"_hs)
			//.data<&EntityDescriptor::immediate_threads>("immediate_threads"_hs)
			//.data<&EntityDescriptor::model_details>("model_details"_hs)
			//.data<&EntityDescriptor::shared_storage>("shared_storage"_hs)
		//;

		engine_meta_type<EntityDescriptor::ModelDetails>()
			.data<&EntityDescriptor::ModelDetails::path>("path"_hs)
			.data<&EntityDescriptor::ModelDetails::offset>("offset"_hs)
			.data<&EntityDescriptor::ModelDetails::collision_cfg>("collision_cfg"_hs)
			.data<&EntityDescriptor::ModelDetails::allow_multiple>("allow_multiple"_hs)
		;
	}

	template <>
	void reflect<EntityThreadRange>()
	{
		engine_meta_type<EntityThreadRange>()
			.data<&EntityThreadRange::start_index>("start_index"_hs)
			.data<&EntityThreadRange::thread_count>("thread_count"_hs)
			.data<nullptr, &EntityThreadRange::begin_point>("begin_point"_hs)
			.data<nullptr, &EntityThreadRange::end_point>("end_point"_hs)
			.data<nullptr, &EntityThreadRange::size>("size"_hs)
			.data<nullptr, &EntityThreadRange::empty>("empty"_hs)
			.ctor
			<
				decltype(EntityThreadRange::start_index),
				decltype(EntityThreadRange::thread_count)
			>()
		;
	}

	template <>
	void reflect<EntityThreadTarget>()
	{
		engine_meta_type<EntityThreadTarget>()
			.ctor<EntityThreadRange>()
			.ctor<EntityThreadID>()
			.ctor<std::string_view>()
			.ctor<std::string>()

			.data<nullptr, &EntityThreadTarget::empty>("empty"_hs)
			//.data<&EntityThreadTarget::value>("value"_hs)
		;
	}

    template <>
	void reflect<StateStorageManager>()
	{
		engine_meta_type<StateStorageManager>()
			.data<&StateStorageManager::states>("states"_hs)
			.ctor<decltype(StateStorageManager::states)>()
			.func<&StateStorageManager::get_storage>("get_storage"_hs)
		;
	}

	template <>
	void reflect<EntityStateInfo>()
	{
		engine_meta_type<EntityStateInfo>()
			.data<&EntityStateInfo::index>("index"_hs)
			.data<&EntityStateInfo::id>("id"_hs)
			.ctor
			<
				decltype(EntityStateInfo::index),
				decltype(EntityStateInfo::id)
			>()
		;
	}

    /*
    template <>
    void reflect<EventTriggerConditionType>()
    {
        engine_meta_type<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerConditionType::compound_method>("compound_method"_hs)
        ;
    }
    */

    template <>
    void reflect<EventTriggerSingleCondition>()
    {
        engine_meta_type<EventTriggerSingleCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerSingleCondition::compound_method>("compound_method"_hs)
            .data<nullptr, &EventTriggerSingleCondition::get_event_type_member>("event_type_member"_hs)
            .data<nullptr, &EventTriggerSingleCondition::get_comparison_value>("comparison_value"_hs)
            .data<nullptr, &EventTriggerSingleCondition::get_comparison_method>("comparison_method"_hs)
            .ctor<MetaSymbolID, MetaAny&&, EventTriggerComparisonMethod>()
        ;
    }

    template <>
    void reflect<EventTriggerMemberCondition>()
    {
        engine_meta_type<EventTriggerMemberCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerMemberCondition::compound_method>("compound_method"_hs)
            .data<nullptr, &EventTriggerMemberCondition::get_event_type_member>("event_type_member"_hs)
            .data<nullptr, &EventTriggerMemberCondition::get_comparison_value>("comparison_value"_hs)
            .data<nullptr, &EventTriggerMemberCondition::get_comparison_method>("comparison_method"_hs)
            .ctor<IndirectMetaDataMember&&, MetaAny&&, EventTriggerComparisonMethod>()
        ;
    }

    template <>
    void reflect<EventTriggerAndCondition>()
    {
        engine_meta_type<EventTriggerAndCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerAndCondition::compound_method>("compound_method"_hs)
        ;
    }

    template <>
    void reflect<EventTriggerOrCondition>()
    {
        engine_meta_type<EventTriggerOrCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerOrCondition::compound_method>("compound_method"_hs)
        ;
    }

    template <>
    void reflect<EventTriggerTrueCondition>()
    {
        engine_meta_type<EventTriggerTrueCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerTrueCondition::compound_method>("compound_method"_hs)
        ;
    }

    template <>
    void reflect<EventTriggerFalseCondition>()
    {
        engine_meta_type<EventTriggerFalseCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerFalseCondition::compound_method>("compound_method"_hs)
        ;
    }

	template <>
	void reflect<EventTriggerInverseCondition>()
	{
		engine_meta_type<EventTriggerInverseCondition>()
            //.base<EventTriggerConditionType>()
            .data<nullptr, &EventTriggerInverseCondition::compound_method>("compound_method"_hs)
        ;
	}

    template <>
    void reflect<EventTriggerCondition>()
    {
        //reflect<EventTriggerConditionType>();
        reflect<EventTriggerSingleCondition>();
        reflect<EventTriggerMemberCondition>();
        reflect<EventTriggerAndCondition>();
        reflect<EventTriggerOrCondition>();
        reflect<EventTriggerTrueCondition>();
        reflect<EventTriggerFalseCondition>();
		reflect<EventTriggerInverseCondition>();

        //engine_meta_type<EventTriggerCondition>();
    }

    GENERATE_EMPTY_TYPE_REFLECTION(StateStorageComponent);
	GENERATE_EMPTY_TYPE_REFLECTION(FrozenStateComponent);

	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(InstanceComponent, instance);

	template <>
	void reflect<StateComponent>()
	{
		engine_meta_type<StateComponent>()
			.data<&StateComponent::state_index>("state_index"_hs)
			.data<&StateComponent::prev_state_index>("prev_state_index"_hs)
			.ctor
			<
				decltype(StateComponent::state_index),
				decltype(StateComponent::prev_state_index)
			>()
		;
	}

	template <>
	void reflect<EntityThreadComponent>()
	{
		engine_meta_type<EntityThreadComponent>()
			.data<&EntityThreadComponent::threads>("threads"_hs)
		;
	}

	template <>
	void reflect<OnStateChange>()
	{
		engine_meta_type<OnStateChange>()
			.data<&OnStateChange::from>("from"_hs)
			.data<&OnStateChange::to>("to"_hs)
			.data<&OnStateChange::state_activated>("state_activated"_hs)
			.ctor
			<
				decltype(OnStateChange::entity),
				decltype(OnStateChange::from),
				decltype(OnStateChange::to),
				decltype(OnStateChange::state_activated)
			>()
		;
	}

	template <>
	void reflect<OnStateActivate>()
	{
		engine_meta_type<OnStateActivate>()
			.data<&OnStateActivate::state>("state"_hs)
			.ctor
			<
				decltype(OnStateActivate::entity),
				decltype(OnStateActivate::state)
			>()
		;
	}

	template <>
	void reflect<ThreadEvent>()
	{
		engine_meta_type<ThreadEvent>()
			//.data<&ThreadEvent::entity>("entity"_hs)
			.data<&ThreadEvent::thread_index>("thread_index"_hs)
			.data<&ThreadEvent::thread_id>("thread_id"_hs)
			.data<&ThreadEvent::local_instance>("local_instance"_hs)
			.data<&ThreadEvent::last_instruction_index>("last_instruction_index"_hs)
		;
	}

	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnThreadSpawn, ThreadEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnThreadComplete, ThreadEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnThreadTerminated, ThreadEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnThreadPaused, ThreadEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnThreadResumed, ThreadEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnThreadAttach, ThreadEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnThreadDetach, ThreadEvent);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(OnThreadUnlink, ThreadEvent);

	template <>
	void reflect<StateChangeCommand>()
	{
		engine_command_type<StateChangeCommand>()
			.data<&StateChangeCommand::state_name>("state_name"_hs)
			.ctor
			<
				decltype(Command::source),
				decltype(Command::target),
				
				decltype(StateChangeCommand::state_name)
			>()
		;
	}

	template <>
	void reflect<StateActivationCommand>()
	{
		engine_command_type<StateActivationCommand>()
			.data<&StateActivationCommand::state_name>("state_name"_hs)
			.ctor
			<
				decltype(Command::source),
				decltype(Command::target),
				
				decltype(StateActivationCommand::state_name)
			>()
		;
	}

	template <>
	void reflect<EntityThreadSpawnCommand>()
	{
		engine_command_type<EntityThreadSpawnCommand>()
			.data<&EntityThreadSpawnCommand::threads>("threads"_hs)
			.data<&EntityThreadSpawnCommand::restart_existing>("restart_existing"_hs)
			.ctor
			<
				decltype(EntityThreadSpawnCommand::source),
				decltype(EntityThreadSpawnCommand::target),
				
				decltype(EntityThreadSpawnCommand::threads),
				decltype(EntityThreadSpawnCommand::restart_existing)
			>()
		;
	}

	template <typename ThreadCommandType>
	static auto reflect_thread_control_flow_command()
	{
		return engine_command_type<ThreadCommandType>()
			.data<&ThreadCommandType::threads>("threads"_hs)
			.data<&ThreadCommandType::check_linked>("check_linked"_hs)
			.ctor
			<
				decltype(ThreadCommandType::source),
				decltype(ThreadCommandType::target),
				
				decltype(ThreadCommandType::threads),
				decltype(ThreadCommandType::check_linked)
			>()
		;
	}

	template <>
	void reflect<EntityThreadStopCommand>()
	{
		reflect_thread_control_flow_command<EntityThreadStopCommand>();
	}

	template <>
	void reflect<EntityThreadPauseCommand>()
	{
		reflect_thread_control_flow_command<EntityThreadPauseCommand>();
	}

	template <>
	void reflect<EntityThreadResumeCommand>()
	{
		reflect_thread_control_flow_command<EntityThreadResumeCommand>();
	}

	template <>
	void reflect<EntityThreadAttachCommand>()
	{
		reflect_thread_control_flow_command<EntityThreadAttachCommand>()
			.data<&EntityThreadAttachCommand::state_id>("state_id"_hs)
			.ctor
			<
				decltype(EntityThreadAttachCommand::source),
				decltype(EntityThreadAttachCommand::target),
				
				decltype(EntityThreadAttachCommand::threads),
				decltype(EntityThreadAttachCommand::check_linked),
				
				decltype(EntityThreadAttachCommand::state_id)
			>()
		;
	}

	template <>
	void reflect<EntityThreadDetachCommand>()
	{
		reflect_thread_control_flow_command<EntityThreadDetachCommand>();
	}

	template <>
	void reflect<EntityThreadUnlinkCommand>()
	{
		engine_command_type<EntityThreadUnlinkCommand>()
			.data<&EntityThreadUnlinkCommand::threads>("threads"_hs)
			.ctor
			<
				decltype(EntityThreadUnlinkCommand::source),
				decltype(EntityThreadUnlinkCommand::target),
				
				decltype(EntityThreadUnlinkCommand::threads)
			>()
		;
	}

	template <>
	void reflect<EntityThreadSkipCommand>()
	{
		reflect_thread_control_flow_command<EntityThreadSkipCommand>()
			.data<&EntityThreadSkipCommand::instructions_skipped>("instructions_skipped"_hs)
			.ctor
			<
				decltype(EntityThreadSkipCommand::source),
				decltype(EntityThreadSkipCommand::target),

				decltype(EntityThreadSkipCommand::threads),
				decltype(EntityThreadSkipCommand::check_linked),
				decltype(EntityThreadSkipCommand::instructions_skipped)
			>()
		;
	}

	template <>
	void reflect<EntityThreadRewindCommand>()
	{
		reflect_thread_control_flow_command<EntityThreadRewindCommand>()
			.data<&EntityThreadRewindCommand::instructions_rewound>("instructions_rewound"_hs)
			.ctor
			<
				decltype(EntityThreadRewindCommand::source),
				decltype(EntityThreadRewindCommand::target),

				decltype(EntityThreadRewindCommand::threads),
				decltype(EntityThreadRewindCommand::check_linked),
				decltype(EntityThreadRewindCommand::instructions_rewound)
			>()
		;
	}

    template <>
	void reflect<EntitySystem>()
	{
		// General:
		reflect<EntityThreadRange>();
		reflect<EntityTarget>();
		reflect<EntityDescriptor>();
		reflect<EntityThreadTarget>();
		reflect<EventTriggerCondition>();

		reflect<StateStorageManager>();
		reflect<EntityStateInfo>();

		// Components:
		reflect<InstanceComponent>();
		reflect<StateComponent>();
		reflect<StateStorageComponent>();
		reflect<FrozenStateComponent>();
		reflect<EntityThreadComponent>();

		// Events:
		reflect<OnStateChange>();
		reflect<OnStateActivate>();
		
		reflect<ThreadEvent>();
		reflect<OnThreadSpawn>();
		reflect<OnThreadComplete>();
		reflect<OnThreadTerminated>();
		reflect<OnThreadPaused>();
		reflect<OnThreadResumed>();
		reflect<OnThreadAttach>();
		reflect<OnThreadDetach>();
		reflect<OnThreadUnlink>();

		// Commands:
		reflect<StateChangeCommand>();
		reflect<StateActivationCommand>();

		reflect<EntityThreadSpawnCommand>();
		reflect<EntityThreadStopCommand>();
		reflect<EntityThreadPauseCommand>();
		reflect<EntityThreadResumeCommand>();
		reflect<EntityThreadAttachCommand>();
		reflect<EntityThreadDetachCommand>();
		reflect<EntityThreadUnlinkCommand>();
		reflect<EntityThreadSkipCommand>();
		reflect<EntityThreadRewindCommand>();
	}
}