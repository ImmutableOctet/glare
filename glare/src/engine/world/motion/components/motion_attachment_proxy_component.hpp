#pragma once

#include <engine/types.hpp>

namespace engine
{
	// Handles swapping of parent entities while attached to another object.
	// (e.g. when landing on a platform or other moving object)
	// This component is automatically attached to an entity by `MotionSystem` when 'attachment' conditions are met.
	struct MotionAttachmentProxy
	{
		// The parent this entity is intended to have.
		// We store this entity in order to restore our parent back to it when attachment is done.
		Entity intended_parent = null;

		// Whether this proxy state is active.
		inline bool is_active() const
		{
			return (intended_parent != null);
		}
	};

	// TODO: Rename `MotionAttachmentProxy` to `MotionAttachmentProxyComponent` or similar.
	using MotionAttachmentProxyComponent = MotionAttachmentProxy;
}