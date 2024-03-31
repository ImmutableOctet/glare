#pragma once

#include "control_flow_token.hpp"
#include "event_yield_request.hpp"

#include <engine/timer.hpp>

#include <engine/entity/entity_instruction.hpp>

#include <variant>

namespace engine
{
	using ScriptFiberResponse = std::variant
	<
		std::monostate,

		ScriptControlFlowToken,
		ScriptEventYieldRequest,

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