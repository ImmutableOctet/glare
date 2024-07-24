#pragma once

#include "control_flow_token.hpp"
#include "event_yield_request.hpp"

#include <engine/timer.hpp>

#include <engine/entity/types.hpp>

//#include <engine/entity/entity_instruction.hpp>
//#include <engine/entity/entity_state_action.hpp>
//#include <engine/entity/actions/entity_thread_spawn_action.hpp>

#include <entt/core/hashed_string.hpp>

#include <variant>

namespace engine
{
	using ScriptFiberResponse = std::variant
	<
		// Yield control / until next update.
		std::monostate,

		ScriptControlFlowToken,
		//EntityStateAction,
		//EntityThreadSpawnAction,

		ScriptEventYieldRequest,

		// State hashes:
		EntityStateHash,
		entt::hashed_string,

		// Instructions:
		//EntityInstruction,

		// Timer/duration:
		Timer,
		Timer::Duration,
		Timer::Days,
		Timer::Hours,
		Timer::Minutes,
		Timer::Seconds,
		Timer::Milliseconds,
		Timer::Microseconds,
		Timer::FloatSeconds,
		Timer::DoubleSeconds
	>;
}