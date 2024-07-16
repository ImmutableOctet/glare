#pragma once

#include "reflection.hpp"

#include "types.hpp"
#include "events.hpp"

#include "entity_system.hpp"
#include "entity_target.hpp"
#include "entity_descriptor.hpp"
#include "entity_instruction.hpp"
#include "entity_thread_target.hpp"
#include "event_trigger_condition.hpp"
#include "state_storage_manager.hpp"
#include "entity_thread_range.hpp"
#include "entity_thread_cadence.hpp"

#include "components/instance_component.hpp"
#include "components/state_component.hpp"
#include "components/state_storage_component.hpp"
#include "components/frozen_state_component.hpp"
#include "components/entity_thread_component.hpp"
#include "components/static_mutation_component.hpp"

#include "commands/commands.hpp"

//#include <engine/meta/meta_variable_evaluation_context.hpp>

#include <engine/resource_manager/entity_factory_data.hpp>

#include <util/variant.hpp>

#include <string>
#include <string_view>

namespace engine
{
	template <>
    void reflect<EntityTarget>()
    {
		engine_meta_type<EntityTarget>()
			// Disabled for now.
			//.data<&EntityTarget::type>("type"_hs)

			.data<nullptr, &EntityTarget::target_index>("target_index"_hs)

			.data<nullptr, &EntityTarget::is_self_targeted>("is_self_targeted"_hs)
			.data<nullptr, &EntityTarget::is_parent_target>("is_parent_target"_hs)
			.data<nullptr, &EntityTarget::is_exact_entity_target>("is_exact_entity_target"_hs)
			.data<nullptr, &EntityTarget::is_entity_name_target>("is_entity_name_target"_hs)
			.data<nullptr, &EntityTarget::is_child_target>("is_child_target"_hs)
			.data<nullptr, &EntityTarget::is_player_target>("is_player_target"_hs)
			.data<nullptr, &EntityTarget::is_null_target>("is_null_target"_hs)

			// Disabled for now:
			//.ctor<EntityTarget::TargetType>()
			//.ctor<&EntityTarget::from_target_type<EntityTarget::IndirectTarget>>()

			.ctor<&EntityTarget::from_target_type<EntityTarget::SelfTarget>>()
			.ctor<&EntityTarget::from_target_type<EntityTarget::ParentTarget>>()
			.ctor<&EntityTarget::from_target_type<EntityTarget::ExactEntityTarget>>()
			.ctor<&EntityTarget::from_target_type<EntityTarget::EntityNameTarget>>()
			.ctor<&EntityTarget::from_target_type<EntityTarget::ChildTarget>>()
			.ctor<&EntityTarget::from_target_type<EntityTarget::PlayerTarget>>()
			.ctor<&EntityTarget::from_target_type<EntityTarget::NullTarget>>()

			.ctor<&EntityTarget::from_entity>()

			.ctor<static_cast<EntityTarget(*)(const std::string&)>(&EntityTarget::from_string)>()
			.ctor<static_cast<EntityTarget(*)(std::string_view)>(&EntityTarget::from_string)>()
        ;

		// Disabled for now:
		//engine_meta_type<EntityTarget::TargetType>();
		//engine_meta_type<EntityTarget::IndirectTarget>();

		engine_meta_type<EntityTarget::ParentTarget>();
		
		// NOTE: `entity` field implicitly reflected.
		engine_meta_type<EntityTarget::ExactEntityTarget>();

		engine_meta_type<EntityTarget::EntityNameTarget>()
			.data<&EntityTarget::EntityNameTarget::entity_name>("entity_name"_hs)
			.data<&EntityTarget::EntityNameTarget::search_children_first>("search_children_first"_hs)
			.ctor<decltype(EntityTarget::EntityNameTarget::entity_name)>()
			.ctor<decltype(EntityTarget::EntityNameTarget::entity_name), decltype(EntityTarget::EntityNameTarget::search_children_first)>()
			.ctor<static_cast<EntityTarget::EntityNameTarget(*)(const std::string&)>(&EntityTarget::EntityNameTarget::from_string)>()
			.ctor<static_cast<EntityTarget::EntityNameTarget(*)(std::string_view)>(&EntityTarget::EntityNameTarget::from_string)>()
		;

		engine_meta_type<EntityTarget::ChildTarget>()
			.data<&EntityTarget::ChildTarget::child_name>("child_name"_hs)
			
			.ctor<decltype(EntityTarget::ChildTarget::child_name)>()
			
			.ctor<static_cast<EntityTarget::ChildTarget(*)(std::string_view, bool)>(&EntityTarget::ChildTarget::from_string)>()
			.ctor<static_cast<EntityTarget::ChildTarget(*)(std::string_view)>(&EntityTarget::ChildTarget::from_string)>()

			.ctor<static_cast<EntityTarget::ChildTarget(*)(const std::string&, bool)>(&EntityTarget::ChildTarget::from_string)>()
			.ctor<static_cast<EntityTarget::ChildTarget(*)(const std::string&)>(&EntityTarget::ChildTarget::from_string)>()
		;

		// NOTE: `player_index` field implicitly reflected.
		// 
		// TODO: Look into adding name-based constructor.
		engine_meta_type<EntityTarget::PlayerTarget>();

		engine_meta_type<EntityTarget::NullTarget>();
    }

	template <>
	void reflect<EntityDescriptor>()
	{
		//engine_empty_type<EntityDescriptor>() // engine_meta_type
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
			// Disabled for now; EnTT's having trouble detecting move-only types.
			//.data<nullptr, static_cast<const EntityThreadComponent::EntityThreadContainer& (EntityThreadComponent::*)() const>(&EntityThreadComponent::get_threads), entt::as_cref_t>("threads"_hs)
		;
	}

	template <>
	void reflect<StaticMutationComponent>()
	{
		engine_meta_type<StaticVariableMutation>()
			.data<&StaticVariableMutation::variable_name>("variable_name"_hs) // get_name
			.data<nullptr, &StaticVariableMutation::get_scope>("variable_scope"_hs)
		;

		engine_meta_type<StaticMutationComponent>()
			.data<&StaticMutationComponent::mutated_components>("mutated_components"_hs)
			.data<&StaticMutationComponent::mutated_variables>("mutated_variables"_hs)

			.func<static_cast<bool(StaticMutationComponent::*)(MetaTypeID)>(&StaticMutationComponent::add_component)>("add_component"_hs)
			.func<static_cast<bool(StaticMutationComponent::*)(const MetaType&)>(&StaticMutationComponent::add_component)>("add_component"_hs)

			.func<static_cast<bool(StaticMutationComponent::*)(MetaTypeID)>(&StaticMutationComponent::remove_component)>("remove_component"_hs)
			.func<static_cast<bool(StaticMutationComponent::*)(const MetaType&)>(&StaticMutationComponent::remove_component)>("remove_component"_hs)

			.func<static_cast<bool(StaticMutationComponent::*)(MetaTypeID) const>(&StaticMutationComponent::contains_component)>("contains_component"_hs)
			.func<static_cast<bool(StaticMutationComponent::*)(const MetaType&) const>(&StaticMutationComponent::contains_component)>("contains_component"_hs)

			.func<static_cast<bool(StaticMutationComponent::*)(const StaticVariableMutation&)>(&StaticMutationComponent::add_variable)>("add_variable"_hs)

			.func<static_cast<bool(StaticMutationComponent::*)(const StaticVariableMutation&)>(&StaticMutationComponent::remove_variable)>("remove_variable"_hs)
			.func<static_cast<bool(StaticMutationComponent::*)(std::string_view, MetaVariableScope)>(&StaticMutationComponent::remove_variable)>("remove_variable"_hs)
			.func<static_cast<bool(StaticMutationComponent::*)(std::string_view)>(&StaticMutationComponent::remove_variable)>("remove_variable"_hs)

			.func<static_cast<bool(StaticMutationComponent::*)(std::string_view, MetaVariableScope) const>(&StaticMutationComponent::contains_variable)>("contains_variable"_hs)
			.func<static_cast<bool(StaticMutationComponent::*)(std::string_view) const>(&StaticMutationComponent::contains_variable)>("contains_variable"_hs)
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
	void reflect<OnThreadVariableUpdate>()
	{
		engine_meta_type<OnThreadVariableUpdate>()
			.base<ThreadEvent>()
			.data<&OnThreadVariableUpdate::resolved_variable_name>("resolved_variable_name"_hs)
			.data<&OnThreadVariableUpdate::variable_scope>("variable_scope"_hs)
			.data<&OnThreadVariableUpdate::variable_update_result>("variable_update_result"_hs)
		;
	}

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
			.data<&EntityThreadSpawnCommand::parent_thread_name>("parent_thread_name"_hs)
			.data<&EntityThreadSpawnCommand::restart_existing>("restart_existing"_hs)
			.ctor
			<
				decltype(EntityThreadSpawnCommand::source),
				decltype(EntityThreadSpawnCommand::target),
				
				decltype(EntityThreadSpawnCommand::threads),
				decltype(EntityThreadSpawnCommand::parent_thread_name),
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
	void reflect<EntityThreadFiberSpawnCommand>()
	{
		// Constructor and `fiber` member disabled for now.
		// (EnTT's trying to generate bindings for a non-existent copy constructor)
		engine_command_type<EntityThreadFiberSpawnCommand>()
			//.data<&EntityThreadFiberSpawnCommand::fiber, entt::as_ref_t>("fiber"_hs)
			.data<&EntityThreadFiberSpawnCommand::state_index>("state_index"_hs)
			.data<&EntityThreadFiberSpawnCommand::thread_name>("thread_name"_hs)
			.data<&EntityThreadFiberSpawnCommand::parent_thread_name>("parent_thread_name"_hs)
			.data<&EntityThreadFiberSpawnCommand::thread_flags>("thread_flags"_hs)

			// Disabled for now; EnTT wants the full type definition for `Script`.
			//.data<&EntityThreadFiberSpawnCommand::script_handle>("script_handle"_hs)

			/*
			.ctor
			<
				decltype(EntityThreadFiberSpawnCommand::source),
				decltype(EntityThreadFiberSpawnCommand::target),

				decltype(EntityThreadFiberSpawnCommand::fiber)&,
				decltype(EntityThreadFiberSpawnCommand::state_index),
				decltype(EntityThreadFiberSpawnCommand::thread_name),
				decltype(EntityThreadFiberSpawnCommand::parent_thread_name),
				decltype(EntityThreadFiberSpawnCommand::thread_flags)

				//decltype(EntityThreadFiberSpawnCommand::script_handle)
			>()
			*/
		;
	}

	template <>
	void reflect<EntityInstruction>()
	{
		auto type = engine_meta_type<EntityInstruction>()
			//.data<&EntityInstruction::value>("value"_hs)
			//.data<nullptr, &EntityInstruction::type_index>("type_index"_hs)

			//.ctor<MetaAny>()
			.ctor<&EntityInstruction::from_meta_any>()
		;

		/*
		util::for_each_variant_type<decltype(EntityInstruction::value)>
		(
			[&type]<typename T>()
			{
				type = type.ctor<&EntityInstruction::from_type<T>>();
				//type = type.ctor<T>();
			}
		);
		*/
	}

	template <>
	void reflect<EntityThreadInstruction>()
	{
		engine_meta_type<EntityThreadInstruction>()
			.data<&EntityThreadInstruction::target_entity>("target_entity"_hs)
			.data<&EntityThreadInstruction::thread_id>("thread_id"_hs)
			
			.ctor
			<
				decltype(EntityThreadInstruction::target_entity),
				decltype(EntityThreadInstruction::thread_id)
			>()

			.ctor
			<
				decltype(EntityThreadInstruction::target_entity),
				decltype(EntityThreadInstruction::thread_id)
			>()
		;
	}

	template <>
	void reflect<EntityInstructionDescriptor>()
	{
		engine_meta_type<EntityInstructionDescriptor>()
			.data<&EntityInstructionDescriptor::instruction>("instruction"_hs)
			.ctor<decltype(EntityInstructionDescriptor::instruction)>()
		;
	}

	template <>
	void reflect<instructions::LocalConditionControlBlock>()
	{
		engine_meta_type<instructions::LocalConditionControlBlock>()
			.data<&instructions::LocalConditionControlBlock::condition>("condition"_hs)
			.data<&instructions::LocalConditionControlBlock::execution_range>("execution_range"_hs)
			.ctor
			<
				decltype(instructions::LocalConditionControlBlock::condition),
				decltype(instructions::LocalConditionControlBlock::execution_range)
			>()
		;
	}

	template <>
	void reflect<instructions::Attach>()
	{
		engine_meta_type<instructions::Attach>()
			.base<instructions::Thread>()
			.data<&instructions::Attach::state_id>("state_id"_hs)
			.data<&instructions::Attach::check_linked>("check_linked"_hs)
		;
	}

	template <>
	void reflect<instructions::Sleep>()
	{
		engine_meta_type<instructions::Sleep>()
			.base<instructions::Thread>()
			.data<&instructions::Sleep::duration>("duration"_hs)
			.data<&instructions::Sleep::check_linked>("check_linked"_hs)
		;
	}

	template <>
	void reflect<instructions::Skip>()
	{
		engine_meta_type<instructions::Skip>()
			.base<instructions::Thread>()
			.data<&instructions::Skip::instructions_skipped>("instructions_skipped"_hs)
			.data<&instructions::Skip::check_linked>("check_linked"_hs)
		;
	}

	template <>
	void reflect<instructions::Rewind>()
	{
		engine_meta_type<instructions::Rewind>()
			.base<instructions::Thread>()
			.data<&instructions::Rewind::instructions_rewound>("instructions_rewound"_hs)
			.data<&instructions::Rewind::check_linked>("check_linked"_hs)
		;
	}

	GENERATE_SINGLE_FIELD_TYPE_REFLECTION(instructions::ControlBlock, size);
	GENERATE_SINGLE_FIELD_TYPE_REFLECTION(instructions::MultiControlBlock, included_instructions);
	GENERATE_SINGLE_FIELD_DERIVED_TYPE_REFLECTION(instructions::Start, instructions::Thread, restart_existing);
	GENERATE_SINGLE_FIELD_DERIVED_TYPE_REFLECTION(instructions::Stop, instructions::Thread, check_linked);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(instructions::Restart, instructions::Thread);
	GENERATE_SINGLE_FIELD_DERIVED_TYPE_REFLECTION(instructions::Pause, instructions::Thread, check_linked);
	GENERATE_SINGLE_FIELD_DERIVED_TYPE_REFLECTION(instructions::Resume, instructions::Thread, check_linked);
	//GENERATE_EMPTY_TYPE_REFLECTION(instructions::Link);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(instructions::Unlink, instructions::Thread);
	GENERATE_SINGLE_FIELD_DERIVED_TYPE_REFLECTION(instructions::Detach, instructions::Thread, check_linked);
	GENERATE_SINGLE_FIELD_DERIVED_TYPE_REFLECTION(instructions::Yield, instructions::Thread, condition);
	GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(instructions::IfControlBlock, instructions::LocalConditionControlBlock);
	GENERATE_SINGLE_FIELD_TYPE_REFLECTION(instructions::FunctionCall, function);
	GENERATE_SINGLE_FIELD_TYPE_REFLECTION(instructions::CoroutineCall, coroutine_function);
	GENERATE_SINGLE_FIELD_TYPE_REFLECTION(instructions::AdvancedMetaExpression, expr);
	GENERATE_SINGLE_FIELD_TYPE_REFLECTION(instructions::VariableDeclaration, variable_details);

	template <>
	void reflect<instructions::CadenceControlBlock>()
	{
		engine_meta_type<instructions::CadenceControlBlock>()
			.data<&instructions::CadenceControlBlock::cadence>("cadence"_hs)
			.data<&instructions::CadenceControlBlock::included_instructions>("included_instructions"_hs)
			//.data<&instructions::CadenceControlBlock::prev_cadence>("prev_cadence"_hs)
		;
	}

	template <>
	void reflect<instructions::VariableAssignment>()
	{
		engine_meta_type<instructions::VariableAssignment>()
			.base<instructions::Thread>()
			.data<&instructions::VariableAssignment::assignment>("assignment"_hs)
			.data<&instructions::VariableAssignment::variable_details>("variable_details"_hs)
			.data<&instructions::VariableAssignment::ignore_if_already_assigned>("ignore_if_already_assigned"_hs)
			.data<&instructions::VariableAssignment::ignore_if_not_declared>("ignore_if_not_declared"_hs)
		;
	}

	template <>
	void reflect<instructions::EventCapture>()
	{
		engine_meta_type<instructions::EventCapture>()
			.data<&instructions::EventCapture::variable_details>("variable_details"_hs)
			.data<&instructions::EventCapture::intended_type>("intended_type"_hs)
		;
	}

	template <>
	void reflect<instructions::Assert>()
	{
		engine_meta_type<instructions::Assert>()
			.data<&instructions::Assert::condition>("condition"_hs)
			.data<&instructions::Assert::debug_message>("debug_message"_hs)
			.data<&instructions::Assert::condition_representation>("condition_representation"_hs)

			.ctor
			<
				decltype(instructions::Assert::condition)
			>()

			.ctor
			<
				decltype(instructions::Assert::condition),
				decltype(instructions::Assert::debug_message)
			>()

			.ctor
			<
				decltype(instructions::Assert::condition),
				decltype(instructions::Assert::debug_message),
				decltype(instructions::Assert::condition_representation)
			>()
		;
	}

    template <>
	void reflect<EntitySystem>()
	{
		engine_system_type<EntitySystem>()
			//.func<&EntitySystem::resolve_variable_context>("resolve_variable_context"_hs)
		;

		// General:
		reflect<EntityThreadRange>();
		reflect<EntityTarget>();
		reflect<EntityDescriptor>();
		reflect<EntityThreadTarget>();
		reflect<EventTriggerCondition>();
		reflect<StateStorageManager>();
		reflect<EntityStateInfo>();
		reflect<EntityThreadCadence>();

		// Components:
		reflect<InstanceComponent>();
		reflect<StateComponent>();
		reflect<StateStorageComponent>();
		reflect<FrozenStateComponent>();
		reflect<EntityThreadComponent>();
		reflect<StaticMutationComponent>();

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
		reflect<OnThreadVariableUpdate>();

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
		reflect<EntityThreadFiberSpawnCommand>();

		// Instructions:
		reflect<EntityInstruction>();
		reflect<EntityThreadInstruction>();
		reflect<EntityInstructionDescriptor>();

		reflect<instructions::ControlBlock>();
		reflect<instructions::LocalConditionControlBlock>();

		reflect<instructions::Start>();
		reflect<instructions::Restart>();
		reflect<instructions::Stop>();
		reflect<instructions::Pause>();
		reflect<instructions::Resume>();
		//reflect<instructions::Link>();
		reflect<instructions::Unlink>();
		reflect<instructions::Attach>();
		reflect<instructions::Detach>();
		reflect<instructions::Sleep>();
		reflect<instructions::Yield>();
		reflect<instructions::Skip>();
		reflect<instructions::Rewind>();
		reflect<instructions::MultiControlBlock>();
		reflect<instructions::CadenceControlBlock>();
		reflect<instructions::IfControlBlock>();
		reflect<instructions::FunctionCall>();
		reflect<instructions::CoroutineCall>();
		reflect<instructions::AdvancedMetaExpression>();
		reflect<instructions::VariableDeclaration>();
		reflect<instructions::VariableAssignment>();
		reflect<instructions::EventCapture>();
		reflect<instructions::Assert>();
	}
}