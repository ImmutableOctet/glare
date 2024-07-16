#pragma once

// Internal runtime functionality for scripts.

#include "script_handle.hpp"

namespace engine
{
	namespace impl
	{
		// Sets the current `Script` instance running for a given thread.
		ScriptHandle set_running_script(ScriptHandle script);

		// Returns the current `Script` instance running on the calling thread.
		ScriptHandle get_running_script();

		// Removes association to a `Script` instance from this thread.
		void clear_running_script();
	}
}