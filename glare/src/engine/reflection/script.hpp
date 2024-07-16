#pragma once

#include "reflect.hpp"

namespace engine
{
	class Script;

	class ScriptNamespace {};

	template <> void reflect<ScriptNamespace>();
	template <> void reflect<Script>();
}