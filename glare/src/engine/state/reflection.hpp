#pragma once

#include <engine/reflection.hpp>

#include "state_storage_manager.hpp"

#include "components/state_component.hpp"
#include "components/state_storage_component.hpp"
#include "components/frozen_state_component.hpp"

#include "commands/state_change_command.hpp"

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
	void reflect<StateSystem>()
	{
		reflect<StateStorageManager>();

		// Components:
		reflect<StateComponent>();
		reflect<StateStorageComponent>();
		reflect<FrozenStateComponent>();

		// Commands:
		reflect<StateChangeCommand>();
	}
}