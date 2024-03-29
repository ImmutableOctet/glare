#pragma once

#include "reflect.hpp"

namespace engine
{
	class Script;

	class ScriptNamespace {};

	extern template void reflect<ScriptNamespace>();
	extern template void reflect<Script>();
}