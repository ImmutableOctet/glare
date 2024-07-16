#pragma once

// Internal runtime functionality for scripts.

#include "script_handle.hpp"

namespace engine
{
	namespace impl
	{
		// Sets the current `Script` instance running for a given thread based on the handle specified.
		ScriptHandle set_running_script(ScriptHandle script);

		// Returns a handle to the current `Script` instance running on the calling thread.
		ScriptHandle get_running_script();

		// Removes association to a `Script` instance from this thread.
		void clear_running_script();
	}
}