#pragma once

#include "types.hpp"
#include "entity_instruction.hpp"

#include <util/small_vector.hpp>

#include <optional>
#include <memory>

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

		inline const auto& get_instruction(InstructionIndex index) const
		{
			return instructions[index]; // .at(index);
		}

		inline auto name() const
		{
			return thread_id;
		}

		inline bool empty() const
		{
			return instructions.empty();
		}

		inline std::size_t size() const
		{
			return instructions.size();
		}
	};
}