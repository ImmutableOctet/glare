#pragma once

#include <engine/meta/types.hpp>
#include <engine/command.hpp>

namespace engine
{
	// Performs copy, move, or member-wise assignment of `component` to `target`'s attached instance of the same type.
	// If `target` does not currently have an attached instance sharing the type of `component`, this will emplace a new instance.
	struct ComponentPatchCommand : public Command
	{
		using Component = MetaAny;

		Component component;

		// If enabled, meta data-member assignment is used over traditional assignment operators.
		bool use_member_assignment = false; // true; // : 1 = ...
	};
}