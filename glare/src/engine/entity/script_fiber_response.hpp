#pragma once

#include "entity_instruction.hpp"

#include <engine/control_flow_token.hpp>
#include <engine/timer.hpp>

#include <variant>

namespace engine
{
	using ScriptControlFlowToken = ControlFlowToken;

	using ScriptFiberResponse = std::variant
	<
		std::monostate,
		ScriptControlFlowToken,
		EntityInstruction,

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