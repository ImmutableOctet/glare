#pragma once

#include <engine/reflection.hpp>

#include "state_storage_manager.hpp"

#include "components/state_component.hpp"
#include "components/state_storage_component.hpp"
#include "components/frozen_state_component.hpp"

namespace engine
{
	class StateSystem;

	GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION(StateComponent, state_index);
	GENERATE_EMPTY_TYPE_REFLECTION(StateStorageComponent);
	GENERATE_EMPTY_TYPE_REFLECTION(FrozenStateComponent);

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
	void reflect<StateSystem>()
	{
		reflect<StateStorageManager>();

		reflect<StateComponent>();
		reflect<StateStorageComponent>();
		reflect<FrozenStateComponent>();
	}
}