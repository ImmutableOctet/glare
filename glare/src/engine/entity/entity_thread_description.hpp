#pragma once

#include "types.hpp"
#include "entity_instruction.hpp"

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

		// A series of instructions to be executed in-order.
		util::small_vector<EntityInstruction, 32> instructions; // 64 // 48

		// NOTE: May move this to another location later. (e.g. handle via map)
		std::optional<EntityThreadID> thread_id;

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