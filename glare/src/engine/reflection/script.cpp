#include "script.hpp"
#include "reflection.hpp"

//#include <engine/script.hpp>

namespace engine
{
	template <>
	void reflect<ScriptNamespace>()
	{
		auto script_type = engine_global_static_type<ScriptNamespace>("Script"_hs);
	}

	template <>
	void reflect<Script>()
	{
		// Currently reflects the shared namespace only.
		reflect<ScriptNamespace>();
	}
}