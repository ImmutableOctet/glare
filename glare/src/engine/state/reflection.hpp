#pragma once

#include <engine/reflection.hpp>

#include "state_storage_manager.hpp"
#include "types.hpp"
#include "events.hpp"

#include "components/state_component.hpp"
#include "components/state_storage_component.hpp"
#include "components/frozen_state_component.hpp"

#include "commands/state_change_command.hpp"
#include "commands/state_activation_command.hpp"

namespace engine
{
	class StateSystem;

	GENERATE_EMPTY_TYPE_REFLECTION(StateStorageComponent);
	GENERATE_EMPTY_TYPE_REFLECTION(FrozenStateComponent);

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
	void reflect<StateSystem>()
	{
		// General:
		reflect<EntityStateInfo>();
		reflect<StateStorageManager>();

		// Components:
		reflect<StateComponent>();
		reflect<StateStorageComponent>();
		reflect<FrozenStateComponent>();

		// Events:
		reflect<OnStateChange>();
		reflect<OnStateActivate>();

		// Commands:
		reflect<StateChangeCommand>();
		reflect<StateActivationCommand>();
	}
}