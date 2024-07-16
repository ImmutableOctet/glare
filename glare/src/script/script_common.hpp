#pragma once

// Default script includes:
#include <math/math.hpp>

#include <engine/types.hpp>
#include <engine/service.hpp>
#include <engine/transform.hpp>

#include <engine/script/script.hpp>
#include <engine/script/script_state_interface.hpp>
#include <engine/script/script_fiber_response.hpp>

#include <engine/system_manager_interface.hpp>

#include <engine/meta/meta_evaluation_context.hpp>
#include <engine/meta/hash.hpp>

#include <engine/components/name_component.hpp>
#include <engine/components/relationship_component.hpp>
#include <engine/components/player_component.hpp>
#include <engine/components/player_target_component.hpp>
#include <engine/components/transform_component.hpp>

#include <engine/entity/entity_system.hpp>
#include <engine/entity/entity_state.hpp>
#include <engine/entity/entity_instruction.hpp>

#include <engine/world/world.hpp>
#include <engine/world/delta/delta_system.hpp>
#include <engine/world/motion/components/velocity_component.hpp>

#include <engine/input/events.hpp>

#include <util/log.hpp>

#include <string>

namespace glare_script_common
{
	//using namespace glare;
	
	using namespace engine;
	using namespace engine::literals;
	using namespace engine::instructions;

	using namespace math;

	//using Vector = math::Vector;

	//using namespace std::literals::chrono_literals;
	using namespace std::literals;

	using enum engine::ControlFlowToken;
}