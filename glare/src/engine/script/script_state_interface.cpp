#include "script_state_interface.hpp"

#include "script.hpp"

namespace engine
{
	// ScriptCurrentStateInterface:
	EntityStateID ScriptCurrentStateInterface::get() const
	{
		return script.get_state_id();
	}

	ScriptCurrentStateInterface::EntityStatePtr ScriptCurrentStateInterface::get_description() const
	{
		return script.get_state_description();
	}

	bool ScriptCurrentStateInterface::set(EntityStateID state_id)
	{
		script.set_state_id(state_id);

		// NOTE: Validation disabled for now.
		//return (get() == state_id);

		return true;
	}

	bool ScriptCurrentStateInterface::set(EntityStateRef state)
	{
		script.set_state(state);

		// NOTE: Validation disabled for now.
		//return (get() == state_id);

		return true;
	}

	// ScriptPreviousStateInterface:
	EntityStateID ScriptPreviousStateInterface::get() const
	{
		return script.get_prev_state_id();
	}

	ScriptPreviousStateInterface::EntityStatePtr ScriptPreviousStateInterface::get_description() const
	{
		return script.get_prev_state_description();
	}

	bool ScriptPreviousStateInterface::set(EntityStateID state_id)
	{
		return false;
	}

	bool ScriptPreviousStateInterface::set(EntityStateRef state)
	{
		return false;
	}
}