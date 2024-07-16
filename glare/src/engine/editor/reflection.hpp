#pragma once

#include <engine/reflection.hpp>

namespace engine
{
	class Editor;

	template <> void reflect<Editor>();
}