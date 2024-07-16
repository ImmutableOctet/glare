#include "script_interface.hpp"

#include "script.hpp"

namespace engine
{
	ScriptInterface& ScriptInterface::get(Script& script)
	{
		return static_cast<ScriptInterface&>(script);
	}

	const ScriptInterface& ScriptInterface::get(const Script& script)
	{
		return static_cast<const ScriptInterface&>(script);
	}

	ScriptInterface::~ScriptInterface() {}
}