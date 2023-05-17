#pragma once

#include "types.hpp"
#include "entity_instruction.hpp"
#include "entity_thread_cadence.hpp"

#include <util/small_vector.hpp>

#include <optional>
#include <memory>
//#include <limits>

namespace engine
{
	struct EntityThreadDescription
	{
		using Instruction = EntityInstruction;
		using InstructionIndex = EntityInstructionIndex;

		// The default cadence that this thread uses.
		// (Not guaranteed to be what it's executed with)
		EntityThreadCadence cadence = EntityThreadCadence::Default;

		// NOTE: May move this to another location later. (e.g. handle via map)
		std::optional<EntityThreadID> thread_id;

		// A series of instructions to be executed in-order.
		util::small_vector<EntityInstruction, 64> instructions; // 32 // 48

		inline const EntityInstruction& get_instruction(InstructionIndex index) const
		{
			return instructions[index]; // .at(index);
		}

		inline std::optional<EntityThreadID> name() const
		{
			return thread_id;
		}

		inline bool empty() const
		{
			return instructions.empty();
		}

		inline EntityInstructionCount size() const
		{
			//assert(instructions.size() < std::numeric_limits<EntityInstructionCount>::max());

			return static_cast<EntityInstructionCount>(instructions.size());
		}
	};
}