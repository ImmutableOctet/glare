#pragma once

#include <types.hpp>

namespace graphics
{
	class Model;
}

namespace engine
{
	struct ModelComponent
	{
		ref<graphics::Model> model;
	};
}