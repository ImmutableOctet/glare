#include "script_runtime_impl.hpp"

#include <cassert>

namespace engine
{
	namespace impl
	{
		static thread_local ScriptHandle _running_script = {};
		
		ScriptHandle set_running_script(ScriptHandle script)
		{
			assert(!_running_script);
			assert(script);

			_running_script = script;

			return _running_script;
		}

		ScriptHandle get_running_script()
		{
			return _running_script;
		}

		void clear_running_script()
		{
			//assert(_running_script);

			_running_script = {};
		}
	}
}