#pragma once

#include <engine/types.hpp>

#include <memory>

namespace graphics
{
	class Model;
}

namespace engine
{
	using ModelRef = std::shared_ptr<graphics::Model>;
	using WeakModelRef = std::weak_ptr<graphics::Model>; // const graphics::Model*
}